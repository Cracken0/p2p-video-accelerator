#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <optional>

#include "p2p/peer.hpp"

namespace p2p {

// 简化版 Kademlia 键（160bit）
using Key160 = std::array<std::uint8_t, 20>;

Key160 sha1_key(const std::string &sid);
std::uint32_t xor_metric(const Key160 &a, const Key160 &b);

struct KNode {
    PeerInfo info;
};

class KademliaTable {
public:
    void add_node(const KNode &n);
    std::vector<KNode> find_closest(const Key160 &key, std::size_t k) const;

private:
    std::vector<KNode> nodes_;
};

} // namespace p2p


