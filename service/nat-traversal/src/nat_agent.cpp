#include "nat_traversal/nat_agent.hpp"
#include "nat_traversal/nat_detector.hpp"

namespace nat {

NatAgent::NatAgent(const NatConfig& cfg) : cfg_(cfg) {}

bool NatAgent::start(const tcp_like::Endpoint& local) {
    local_ = local;
    running_ = true;
    return true;
}

void NatAgent::stop() {
    running_ = false;
}

std::vector<Candidate> NatAgent::gatherCandidates() {
    // TODO: 通过 STUN 收集 server-reflexive 候选；当前返回一个占位候选
    return { Candidate{"127.0.0.1", local_.port} };
}

Connectivity NatAgent::negotiate(const std::vector<Candidate>& remote) {
    Connectivity c{};
    c.type = Connectivity::Type::Direct;
    c.localEp = local_;
    if (!remote.empty()) {
        c.remoteEp.address = remote.front().ip;
        c.remoteEp.port = remote.front().port;
    }
    return c;
}

void NatAgent::maintainKeepalive() {
    // 由 tcp_like::ReliableSession 自带心跳维持映射；此处可扩展 STUN Binding Indication
}

bool NatAgent::connectReliable(const Connectivity& path, tcp_like::ReliableSession& session) {
    if (!running_) return false;
    if (path.type == Connectivity::Type::Direct) {
        return session.connect(path.localEp, path.remoteEp);
    }
    // TODO: relay fallback (TURN or custom repeater)
    return false;
}

} // namespace nat







