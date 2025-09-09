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
    // 基于 NatDetector 的轻量占位探测，返回一个本端候选（若映射存在则返回映射）
    NatDetector det;
    det.detect(local_, cfg_.stunServer);
    auto mapped = det.mappedCandidate();
    if (mapped) return { *mapped };
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
    // relay fallback 由上层决定，当前未实现
    return false;
}

} // namespace nat







