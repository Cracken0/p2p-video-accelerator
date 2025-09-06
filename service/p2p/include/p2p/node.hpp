#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

#include "p2p/router.hpp"
#include "p2p/protocol.hpp"
#include "p2p/data_api.hpp"
#include "nat_traversal/nat_agent.hpp"

namespace p2p {

class Node {
public:
    Node(RouterTable& router, KademliaTable& dht, store::FileStore& store, nat::NatAgent& nat);
    ~Node();

    void start();
    void stop();

    // 主动请求：协助我连接某 pid
    bool request_assist(const std::string& helperPid, const std::string& targetPid);

    // 主动请求：向 pid 请求资源
    bool request_get(const std::string& pid, const std::string& sid);

private:
    void loop();
    void handle_message(Peer& from, const std::vector<uint8_t>& msg);
    void on_cmd(Peer& from, CmdCode code, const std::vector<uint8_t>& payload);

    RouterTable& router_;
    KademliaTable& dht_;
    store::FileStore& store_;
    DataApi dataApi_;
    nat::NatAgent& nat_;

    std::atomic<bool> running_{false};
    std::thread th_;
};

} // namespace p2p


