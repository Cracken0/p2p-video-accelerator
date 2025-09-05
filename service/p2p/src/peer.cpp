#include "p2p/peer.hpp"

namespace p2p {

Peer::Peer(const PeerInfo &info) : info_(info) {}
Peer::~Peer() = default;

const PeerInfo& Peer::info() const { return info_; }
PeerState Peer::state() const { return state_.load(); }

bool Peer::connect(nat::NatAgent &agent) {
    if (state_.load() == PeerState::Online) return true;
    state_ = PeerState::Connecting;

    auto cands = agent.gatherCandidates();
    std::vector<nat::Candidate> remoteCands;
    if (!info_.lastKnownIp.empty()) {
        remoteCands.push_back({info_.lastKnownIp, info_.lastKnownPort});
    }
    auto path = agent.negotiate(remoteCands);

    auto sess = std::make_unique<tcp_like::ReliableSession>();
    if (!agent.connectReliable(path, *sess)) {
        state_ = PeerState::Offline;
        return false;
    }
    session_ = std::move(sess);
    state_ = PeerState::Online;
    return true;
}

void Peer::disconnect() {
    if (session_) {
        session_->stop();
        session_.reset();
    }
    state_ = PeerState::Offline;
}

bool Peer::is_online() const { return state_.load() == PeerState::Online; }

bool Peer::send_command(const std::vector<std::uint8_t> &payload) {
    if (!session_ || !is_online()) return false;
    return session_->send(payload);
}

bool Peer::send_data(const std::vector<std::uint8_t> &payload) {
    if (!session_ || !is_online()) return false;
    return session_->send(payload);
}

void Peer::attach_session(std::unique_ptr<tcp_like::ReliableSession> session) {
    session_ = std::move(session);
    state_ = PeerState::Online;
}

tcp_like::ReliableSession* Peer::session() { return session_.get(); }

bool SuperPeer::is_online() const { return true; }

} // namespace p2p


