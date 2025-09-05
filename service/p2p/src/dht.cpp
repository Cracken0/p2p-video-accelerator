#include "p2p/dht.hpp"

#include <algorithm>
#include <openssl/sha.h>

namespace p2p {

Key160 sha1_key(const std::string &sid) {
    Key160 out{};
    unsigned char digest[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(sid.data()), sid.size(), digest);
    std::copy(digest, digest + 20, out.begin());
    return out;
}

std::uint32_t xor_metric(const Key160 &a, const Key160 &b) {
    std::uint32_t score = 0;
    for (std::size_t i = 0; i < 20; ++i) {
        score = (score << 1) ^ static_cast<std::uint32_t>(a[i] ^ b[i]);
    }
    return score;
}

void KademliaTable::add_node(const KNode &n) {
    // 简化：无桶结构，直接插入，去重
    auto it = std::find_if(nodes_.begin(), nodes_.end(), [&](const KNode &x){ return x.info.pid.id == n.info.pid.id; });
    if (it == nodes_.end()) nodes_.push_back(n);
}

std::vector<KNode> KademliaTable::find_closest(const Key160 &key, std::size_t k) const {
    std::vector<KNode> v = nodes_;
    std::sort(v.begin(), v.end(), [&](const KNode &lhs, const KNode &rhs){
        auto lk = sha1_key(lhs.info.pid.id);
        auto rk = sha1_key(rhs.info.pid.id);
        return xor_metric(lk, key) < xor_metric(rk, key);
    });
    if (v.size() > k) v.resize(k);
    return v;
}

} // namespace p2p


