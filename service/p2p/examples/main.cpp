#include "p2p/peer.hpp"
#include "p2p/router.hpp"
#include "p2p/dht.hpp"
#include "p2p/data_api.hpp"
#include "nat_traversal/nat_agent.hpp"
#include "store/store.hpp"

#include <iostream>

int main() {
    // 基础组件
    store::FileStore fs("/tmp/p2p_store");
    fs.set_storage_limit(1024ull * 1024ull * 1024ull);

    p2p::RouterTable router;
    p2p::KademliaTable dht;

    // SuperPeer（示意）
    p2p::PeerInfo spInfo{ {"super"}, nat::NatType::Unknown, "127.0.0.1", 60000 };
    auto sp = std::make_shared<p2p::SuperPeer>(spInfo);
    router.set_superpeer(sp);
    dht.add_node({spInfo});

    // NatAgent
    nat::NatConfig cfg; cfg.stunServer = {"1.1.1.1", 3478};
    nat::NatAgent agent(cfg);
    tcp_like::Endpoint local{"0.0.0.0", 52000};
    agent.start(local);

    // 一个普通 peer
    p2p::PeerInfo peerInfo{ {"peerA"}, nat::NatType::Unknown, "127.0.0.1", 52001 };
    auto peerA = std::make_shared<p2p::Peer>(peerInfo);
    router.upsert_online(peerA);
    dht.add_node({peerInfo});

    // 连接尝试
    if (peerA->connect(agent)) {
        std::cout << "peerA online\n";
    } else {
        std::cout << "peerA connect failed\n";
    }

    // 数据接口
    p2p::DataApi api(dht, router, fs);
    api.createData("hello.txt", std::vector<uint8_t>{'h','e','l','l','o'});
    auto v = api.getData("hello.txt", 0);
    std::cout << "local read size=" << v.size() << "\n";

    return 0;
}


