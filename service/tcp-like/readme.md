# tcp-like

实现一个基于udp的可靠连接的轻量级网络库，需求如下

## 技术栈要求
使用cpp，环境是linux，从sokect级别起步开发。


## 接口要求
按照主流的tcp-like的可靠udp去做就行。要求轻量级。


## 项目录结构要求

目录结构清晰，模块划分清晰，接口设计与实现清晰，内外部接口权限划分清晰；

---


## 可靠UDP模块功能与接口说明（tcp-like）

### 功能概览
- 可靠传输：序列号、ACK 与 选择性 ACK 位图（最近 32 个包）。
- 重传机制：定时器驱动的超时重传与重试上限（固定超时，未含 RTT/RTO 自适应）。
- 乱序重排：缓冲乱序包，按 `expectedSequence` 顺序交付应用层。
- 三次握手：`SYN / SYN-ACK / ACK` 建链，暴露连接状态机。
- ACK-only：收到数据后即刻回 ACK-only 包，加速对端确认与清理在途包。
- 心跳保活：`PING/PONG` 可配置，空闲时自动发送，维持 NAT 映射与连接活性。
- 空闲超时：长时间无收发/心跳将进入 `TimeWait` 并关闭。
- 流量统计：实时发送/接收速率（bps）与累计上下行字节数。

> 说明：当前为轻量演示实现，未包含拥塞控制、RTO 估计/自适应、分片/MTU 探测、优雅关闭（FIN 状态完整流转）等。

### 主要类型与接口
命名空间：`tcp_like`

- 端点
  - `struct Endpoint { std::string address; uint16_t port; }`

- 会话状态与统计
  - `enum class ReliableSession::State { Closed, SynSent, SynReceived, Established, FinWait, TimeWait }`
  - `struct ReliableSession::Stats { uint64_t bytesSentTotal; uint64_t bytesRecvTotal; double sendRateBps; double recvRateBps; }`

- 会话类 `ReliableSession`
  - 连接/监听
    - `bool connect(const Endpoint& local, const Endpoint& remote)`：客户端发起连接（会发送 `SYN`）。
    - `bool listen(const Endpoint& local)`：服务端在本地地址监听（无阻塞）。
    - `void acceptFrom(const Endpoint& remote)`：指定期望的对端（Demo 用法）；收到 `SYN` 后回 `SYN-ACK`。
    - `void stop()`：关闭会话。
  - 数据收发（非阻塞）
    - `bool send(const std::vector<uint8_t>& payload)`：入发送队列，由 `update()` 驱动实际发送/重传。
    - `bool receive(std::vector<uint8_t>& outPayload)`：从重排后的接收队列取出一条完整消息。
  - 驱动/查询
    - `void update()`：必须被周期性调用，用于收包、发送/重传、心跳、超时、速率统计。
    - `bool isEstablished() const`：是否已建链。
    - `ReliableSession::State state() const`：返回当前状态机状态。
    - `ReliableSession::Stats getStats() const`：获取统计信息。
  - 配置
    - `void setIdleTimeoutMs(int ms)`：配置空闲超时（默认 10000ms）。
    - `void setHeartbeatEnabled(bool enabled)`：启用/禁用心跳（默认启用）。
    - `void setHeartbeatIntervalMs(int ms)`：心跳间隔（默认 1000ms）。
  - 其他行为
    - 仅处理与期望远端 `Endpoint` 匹配的数据包（基本防骚扰）。
    - 内部使用安全的 32 位序列号比较（考虑环回）。

（底层 `UdpSocket` 也以头文件形式暴露，常规使用建议直接通过 `ReliableSession`。）

### 用法示例
服务端（节选自 `examples/echo_server.cpp`）：

```cpp
tcp_like::ReliableSession server;
tcp_like::Endpoint local{"0.0.0.0", 50000};
tcp_like::Endpoint clientHint{"127.0.0.1", 50001};

server.listen(local);
server.acceptFrom(clientHint);

for (;;) {
    server.update();
    if (server.isEstablished()) {
        std::vector<uint8_t> payload;
        while (server.receive(payload)) {
            server.send(payload); // echo
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
```

客户端（节选自 `examples/echo_client.cpp`）：

```cpp
tcp_like::ReliableSession client;
tcp_like::Endpoint local{"0.0.0.0", 50001};
tcp_like::Endpoint remote{"127.0.0.1", 50000};

client.connect(local, remote);

int counter = 0;
for (;;) {
    client.update();
    if (client.isEstablished()) {
        std::string msg = "hello " + std::to_string(counter++);
        client.send(std::vector<uint8_t>(msg.begin(), msg.end()));
        std::vector<uint8_t> payload;
        while (client.receive(payload)) {
            // handle echo
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
```

### 构建与运行
```bash
cmake -S /home/cracken/project/p2p-video-accelerator/service -B /home/cracken/project/p2p-video-accelerator/service/build
cmake --build /home/cracken/project/p2p-video-accelerator/service/build -j

/home/cracken/project/p2p-video-accelerator/service/build/tcp-like/echo_server
/home/cracken/project/p2p-video-accelerator/service/build/tcp-like/echo_client
```

### 注意事项
- 必须定期调用 `update()`；建议 10–50ms 调用一次，取决于实时性需求。
- `send/receive` 为非阻塞语义；`receive` 返回 false 表示当前无可读数据。
- 心跳与空闲超时会共同影响连接生存；在高抖动网络可适当调大心跳间隔与超时。
- 当前未实现：RTT/RTO 估计、拥塞控制、分片、FIN 优雅关闭。
- 示例端口为同机演示，可在实际部署中调整并结合 NAT 模块进行打洞后再建立可靠会话。

### 超时控制与统计流量（代码位置与说明）

超时控制逻辑集中在 `src/reliable_session.cpp` 的 `update()` 例程与收包路径：

1) 重传与丢弃：`update()` 内部遍历 `sendQueue_`，当当前时间与 `InflightPacket::lastSendAt` 相差超过常量 `kResendTimeoutMs`（300ms）时，重新发送携带 `FLAG_DATA|FLAG_ACK` 的数据包并累加 `retries`；超过 `kMaxRetries`（10）后在同函数末尾通过 `remove_if` 丢弃在途包。

2) ACK-only 快速反馈：`handleIncoming()` 在成功解析包头后，除处理心跳外，若有数据则调用 `deliverInOrder()` 与统计接收字节，并无论是否有数据负载，都会立即发送一个仅含 `FLAG_ACK` 的确认，缩短确认路径、减少冗余重传。

3) 心跳与空闲超时：`maybeSendHeartbeat()` 按 `heartbeatIntervalMs_`（默认 1000ms）在空闲时发送 `PING`；`update()` 根据 `lastActivityAt_` 与 `idleTimeoutMs_`（默认 10000ms）判断是否进入 `TimeWait` 并 `stop()` 释放资源。

4) 速率与累计：发送路径在 `send()` 中累加 `stats_.bytesSentTotal` 与窗口计数 `bytesSentWindow_`；接收路径在 `handleIncoming()` 中累加 `stats_.bytesRecvTotal/bytesRecvWindow_`。`updateRates()` 以 0.5s 滑动窗口计算 `sendRateBps/recvRateBps` 并清空窗口计数，暴露给 `getStats()`。

关键成员与常量：`kResendTimeoutMs/kMaxRetries`、`lastActivityAt_/idleTimeoutMs_`、`heartbeatEnabled_/heartbeatIntervalMs_`、`bytesSentWindow_/bytesRecvWindow_/rateWindowStart_`，均定义于 `src/reliable_session.cpp` 与 `include/tcp_like/reliable_session.hpp`。

