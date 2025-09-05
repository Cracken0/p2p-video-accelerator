#include "nat_traversal/nat_detector.hpp"

namespace nat {

NatDetector::NatDetector() = default;

bool NatDetector::detect(const tcp_like::Endpoint& /*local*/, const tcp_like::Endpoint& /*stunServer*/) {
    // TODO: 这里放置 STUN 探测逻辑；当前占位返回 Unknown
    type_ = NatType::Unknown;
    return true;
}

} // namespace nat







