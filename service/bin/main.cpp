#include "repeater/server.hpp"
#include "p2p/data_api.hpp"
#include "p2p/dht.hpp"
#include "p2p/router.hpp"
#include "store/store.hpp"

#include <csignal>
#include <iostream>
#include <memory>
#include <thread>

static bool g_stop = false;
static void on_sigint(int) { g_stop = true; }

int main() {
    using namespace repeater;
    std::signal(SIGINT, on_sigint);
    std::signal(SIGTERM, on_sigint);

    // 初始化底层存储与 p2p 组件（简化，使用默认/占位）
    store::FileStore store("/tmp/p2p_store");
    p2p::KademliaTable dht; // 假定存在默认构造
    p2p::RouterTable router; // 假定存在默认构造
    p2p::DataApi dataApi(dht, router, store);

    auto registry = std::make_shared<FieldRegistry>();

    // 示例字段：获取存储使用情况
    registry->register_getter("store.used", [&]() {
        std::uint64_t used = store.get_used_size();
        std::vector<std::uint8_t> out(8);
        for (int i = 0; i < 8; ++i) out[7 - i] = static_cast<std::uint8_t>((used >> (i * 8)) & 0xFF);
        return out;
    });

    // 示例字段：写入数据到 store（setter 写入，field 固定为 "store.put:<sid>")
    registry->register_setter("store.put", [&](const std::vector<std::uint8_t> &data) {
        // data 前 1 字节是 sid 长度，后面是 sid 字符串，然后剩余是 payload
        if (data.size() < 1) return false;
        std::size_t sidLen = data[0];
        if (data.size() < 1 + sidLen) return false;
        std::string sid(reinterpret_cast<const char*>(data.data() + 1), sidLen);
        std::vector<std::uint8_t> payload(data.begin() + 1 + sidLen, data.end());
        int n = dataApi.putData(sid, payload);
        return n > 0;
    });

    // 示例字段：读取数据（getter，field 固定为 "store.get:<sid>"，通过 data 传递 begin 高位序）
    registry->register_getter("store.get", [&]() {
        // 简化：固定 sid="demo", begin=0
        std::string sid = "demo";
        return dataApi.getData(sid, 0);
    });

    IpcServer server("/tmp/p2p_repeater.sock", registry);
    if (!server.start()) {
        std::cerr << "failed to start repeater ipc server" << std::endl;
        return 1;
    }

    std::cout << "service running. unix socket: /tmp/p2p_repeater.sock" << std::endl;
    while (!g_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    server.stop();
    std::cout << "service stopped" << std::endl;
    return 0;
}


