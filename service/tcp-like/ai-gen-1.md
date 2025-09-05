如何编译与运行（同之前）

- 构建：

  bash

   cmake -S /home/cracken/project/p2p-video-accelerator/service -B /home/cracken/project/p2p-video-accelerator/service/build && cmake --build /home/cracken/project/p2p-video-accelerator/service/build -j

- 两个终端

  运行：

  bash

   /home/cracken/project/p2p-video-accelerator/service/build/tcp-like/echo_server

  bash

   /home/cracken/project/p2p-video-accelerator/service/build/tcp-like/echo_client

使用要点

- 服务端：

- listen(local) 后调用 acceptFrom(remote) 指定要接受的远端（临时demo用法）。

- 连接建立后 isEstablished() 为 true，再进行 receive/send。

- 客户端：

- connect(local, remote) 发起握手；建立后正常 send/receive。

- 统计信息：

- getStats() 获取实时速率与累计字节。

后续可加

- 拥塞控制/流控、超时RTO估计、快速重传/快速恢复、MTU探测/分片、FIN/优雅关闭、应用层心跳、连接/重连管理、HMAC与重放保护。

需要我再补一个更真实的服务端“动态accept”示例（自动从首个收到的SYN提取 remote）吗？