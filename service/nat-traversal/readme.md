# nat-traversal

基于已有的tcp-like模块，设计实现nat穿透模块

## 技术栈要求
使用cpp，环境是linux，基于已有的tcp-like模块。
主要是起到以下作用：
检测当前网络环境属于以下哪一种内网穿透类型｛公网IP无NAT；完全锥形NAT；限制锥形NAT；端口限制锥形NAT；对称型NAT；｝ 使用枚举类型表示。
维护
辅助连接功能，即“申请一个已经连接的主机协助连接” “接收/拒绝来自其他主机的协助申请”之类的


## 接口要求
按照主流的tcp-like的可靠udp去做就行。要求轻量级。


## 项目录结构要求


目录结构清晰，模块划分清晰，接口设计与实现清晰，内外部接口权限划分清晰；


---

## NAT 探测说明与示例

本模块提供最小可用的 STUN Binding 探测以获取外网映射地址（server-reflexive candidate），并在单/多 IP 的 STUN 服务器下给出可用的 NAT 类型近似判断：

1) 若本地地址与 STUN 映射一致，判定为 `OpenInternet`；
2) 若对同一域名解析得到的两个 STUN IPv4 地址返回的映射不同，判定为 `Symmetric`；
3) 其他情况保守判为 `PortRestrictedCone`；异常时为 `Unknown`。

示例调用：

```cpp
#include "nat_traversal/nat_detector.hpp"

int main() {
    nat::NatDetector det;
    tcp_like::Endpoint local{"0.0.0.0", 52000};
    tcp_like::Endpoint stun{"stun.chat.bilibili.com", 3478};
    if (det.detect(local, stun)) {
        auto t = det.natType();
        auto cand = det.mappedCandidate();
        // 根据 t 与 cand 进行后续候选交换与打洞
    }
}
```

与 NatAgent 对接：

```cpp
#include "nat_traversal/nat_agent.hpp"

nat::NatConfig cfg; cfg.stunServer = {"stun.chat.bilibili.com", 3478};
nat::NatAgent agent(cfg);
agent.start({"0.0.0.0", 52000});
auto localCands = agent.gatherCandidates();
// 与对端交换后：
auto path = agent.negotiate(/*remote candidates*/);
tcp_like::ReliableSession sess;
agent.connectReliable(path, sess);
```

注意：当前实现仅覆盖 IPv4、单次 STUN Binding 与最小化 NAT 类型近似判断，未实现 TURN 中继与完整的锥型细分；后续可在不破坏接口的前提下逐步增强。

### detect 函数逻辑讲解

`NatDetector::detect(local, stunServer)` 的实现严格遵循 RFC 5389 的 STUN Binding 流程，并以最小代码量完成对外网映射的获取与可测的基础 NAT 类型判定。函数首先对传入的 STUN 域名进行解析，获得一到两个可用的 IPv4 地址。之所以保留多个地址，是为了在不引入复杂行为探测的情形下，通过跨 IP 的映射一致性检验获取对称性线索。解析结果经过去重并按解析顺序排列，保证探测的确定性与可复现性。

在探测阶段，函数构造仅含 20 字节报文头的 STUN Binding Request，设置类型字段为 0x0001，报文长度为 0，并写入标准的 magic cookie（0x2112A442）与随机生成的 12 字节事务 ID。请求通过 `tcp_like::UdpSocket` 以 UDP 发送至解析得到的 STUN 服务器地址与端口（默认 3478），同时为接收路径配置毫秒级的超时，以避免在网络不可达场景下长时间阻塞。响应抵达后，函数首先校验消息类型为 Binding Success（0x0101），随后按照 TLV 结构遍历属性集合，优先解析 XOR-MAPPED-ADDRESS；在服务器不返回该属性时，退化解析 MAPPED-ADDRESS。由于实现当前仅覆盖 IPv4，XOR-MAPPED-ADDRESS 的端口与地址字段需分别与 magic cookie 进行异或以还原真实的外网 `ip:port`。

完成映射提取后，函数将其作为 server‑reflexive 候选回传给上层，以支撑候选交换与直连尝试。在类型判定方面，函数采取刻意保守而又可验证的策略：当本地监听地址与端口与 STUN 返回的映射完全一致时，可认定处于开放互联网环境（OpenInternet）；当对同一 STUN 域名解析得到的两个不同 IPv4 地址分别返回的映射不一致时，可据此判定为对称型 NAT（Symmetric），其典型特征为针对不同外部目的地址生成不同的映射；其余仍可达但缺乏对称性证据的情形则归入端口限制锥型（PortRestrictedCone）的近似集合；若解析失败或超时，类型标记为 Unknown 并清空映射，以避免误判。

需要强调的是，该实现没有引入 CHANGE‑REQUEST 属性驱动的行为测试，也未联动多台异网段 STUN 服务器进行多维度交叉验证，因此无法在 Full Cone、Restricted Cone 与 Port‑Restricted Cone 之间做出更精细的区分。尽管如此，通过“双 IP 映射一致性”的低开销检验，系统仍然能够稳定地区分 Symmetric 与 OpenInternet 两类边界情形，并在其余情况给出保守而实用的近似类型。这一取舍在工程上实现了探测可靠性与实现复杂度之间的平衡，同时为后续迭代到更完整的多服务器、多属性的判定方案预留了清晰的扩展路径。
