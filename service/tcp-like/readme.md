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
- 重传机制：定时器驱动的超时重传与重试上限。
- 乱序重排：缓冲乱序包，按 `expectedSequence` 顺序交付应用层。
- 三次握手：`SYN / SYN-ACK / ACK` 建链，暴露连接状态机。
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
- 示例端口为同机演示，可在实际部署中调整并结合 NAT 模块进行打洞后再建立可靠会话。

