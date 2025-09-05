# C++ Repeater 服务端使用文档

- 套接字路径：`/tmp/p2p_repeater.sock`
- 报文结构：`func_type(1) + field(32) + len(4,BE) + data(len)`
- `func_type`: 1=getter, 2=setter

示例字段：
- `store.used` getter: 返回 8 字节 big-endian 的 used size
- `store.put` setter: data=[sid_len(1)] + sid + payload

集成：
- 参考 `service/bin/main.cpp`，注册字段并启动 `IpcServer`。
