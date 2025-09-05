#include "p2p/router.hpp"

namespace p2p {

RouterTable::RouterTable() = default;

void RouterTable::set_superpeer(std::shared_ptr<SuperPeer> sp) {
    std::lock_guard<std::mutex> lk(mutex_);
    superpeer_ = std::move(sp);
}

std::shared_ptr<SuperPeer> RouterTable::superpeer() {
    std::lock_guard<std::mutex> lk(mutex_);
    return superpeer_;
}

void RouterTable::upsert_online(std::shared_ptr<Peer> p) {
    std::lock_guard<std::mutex> lk(mutex_);
    online_[p->info().pid.id] = std::move(p);
}

void RouterTable::mark_offline(const std::string &pid) {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = online_.find(pid);
    if (it != online_.end()) {
        offline_[pid] = it->second->info();
        online_.erase(it);
    }
}

std::shared_ptr<Peer> RouterTable::get(const std::string &pid) {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = online_.find(pid);
    if (it != online_.end()) return it->second;
    return nullptr;
}

std::vector<PeerInfo> RouterTable::list_online() const {
    std::vector<PeerInfo> v;
    std::lock_guard<std::mutex> lk(mutex_);
    v.reserve(online_.size());
    for (const auto &kv : online_) v.push_back(kv.second->info());
    return v;
}

std::vector<PeerInfo> RouterTable::list_offline() const {
    std::vector<PeerInfo> v;
    std::lock_guard<std::mutex> lk(mutex_);
    v.reserve(offline_.size());
    for (const auto &kv : offline_) v.push_back(kv.second);
    return v;
}

} // namespace p2p


