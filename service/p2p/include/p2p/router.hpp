#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include <optional>

#include "p2p/peer.hpp"

namespace p2p {

class RouterTable {
public:
    RouterTable();

    void set_superpeer(std::shared_ptr<SuperPeer> sp);
    std::shared_ptr<SuperPeer> superpeer();

    void upsert_online(std::shared_ptr<Peer> p);
    void mark_offline(const std::string &pid);

    std::shared_ptr<Peer> get(const std::string &pid);
    std::vector<PeerInfo> list_online() const;
    std::vector<PeerInfo> list_offline() const;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<Peer>> online_;
    std::unordered_map<std::string, PeerInfo> offline_;
    std::shared_ptr<SuperPeer> superpeer_;
};

} // namespace p2p


