#pragma once

#include "nat_traversal/nat_types.hpp"
#include "tcp_like/reliable_session.hpp"
#include <vector>

namespace nat {

struct Connectivity {
    enum class Type { Direct, Relay } type{Type::Direct};
    tcp_like::Endpoint localEp{};
    tcp_like::Endpoint remoteEp{};
};

struct NatConfig {
    tcp_like::Endpoint stunServer{ "0.0.0.0", 0 };
    bool enableRelayFallback{false};
};

class NatAgent {
public:
    explicit NatAgent(const NatConfig& cfg);

    bool start(const tcp_like::Endpoint& local);
    void stop();

    // 候选收集（占位实现）
    std::vector<Candidate> gatherCandidates();

    // 协商可达通路（占位：直接使用对端候选首个）
    Connectivity negotiate(const std::vector<Candidate>& remote);

    // 维护保活（NAT 映射存活，基于 tcp-like 心跳即可）
    void maintainKeepalive();

    // Demo: 打洞与建立可靠会话（占位打洞策略）
    bool connectReliable(const Connectivity& path, tcp_like::ReliableSession& session);

private:
    NatConfig cfg_{};
    tcp_like::Endpoint local_{};
    bool running_{false};
};

} // namespace nat







