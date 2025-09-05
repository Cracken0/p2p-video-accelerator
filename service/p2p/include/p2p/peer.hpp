#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <atomic>

#include "tcp_like/reliable_session.hpp"
#include "nat_traversal/nat_agent.hpp"

namespace p2p {

enum class PeerState {
    Offline,
    Connecting,
    Online
};

struct PeerId {
    std::string id;
};

struct PeerInfo {
    PeerId pid;
    nat::NatType natType{nat::NatType::Unknown};
    std::string lastKnownIp;
    std::uint16_t lastKnownPort{0};
};

class Peer {
public:
    explicit Peer(const PeerInfo &info);
    virtual ~Peer();

    const PeerInfo& info() const;
    PeerState state() const;

    // 主动发起连接（使用 NAT 模块 + 可靠 UDP）
    virtual bool connect(nat::NatAgent &agent);
    virtual void disconnect();
    virtual bool is_online() const;

    // 发送命令与数据（同一会话之上，可在上层区分 channel）
    bool send_command(const std::vector<std::uint8_t> &payload);
    bool send_data(const std::vector<std::uint8_t> &payload);

    // 处理接入连接（对方主动连入）
    void attach_session(std::unique_ptr<tcp_like::ReliableSession> session);

    tcp_like::ReliableSession* session();

protected:
    PeerInfo info_;
    std::unique_ptr<tcp_like::ReliableSession> session_;
    std::atomic<PeerState> state_{PeerState::Offline};
};

class SuperPeer : public Peer {
public:
    using Peer::Peer;
    bool is_online() const override; // 始终在线
};

} // namespace p2p


