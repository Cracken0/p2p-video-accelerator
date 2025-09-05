#pragma once

#include "nat_traversal/nat_types.hpp"
#include "tcp_like/udp_socket.hpp"

namespace nat {

class NatDetector {
public:
    NatDetector();
    // 使用 STUN 服务器地址进行 NAT 类型探测（占位接口）
    bool detect(const tcp_like::Endpoint& local, const tcp_like::Endpoint& stunServer);
    NatType natType() const { return type_; }

private:
    NatType type_{NatType::Unknown};
};

} // namespace nat







