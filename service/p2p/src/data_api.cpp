#include "p2p/data_api.hpp"

namespace p2p {

DataApi::DataApi(KademliaTable &dht, RouterTable &router, store::FileStore &store)
    : dht_(dht), router_(router), store_(store) {}

std::vector<std::uint8_t> DataApi::getData(const std::string &sid, int begin) {
    auto data = store_.read(sid, begin, begin + 64 * 1024); // 默认 64KB 窗口
    if (!data.empty()) return data;
    return findData(sid);
}

int DataApi::putData(const std::string &sid, const std::vector<std::uint8_t> &data) {
    bool ok = store_.append(sid, data);
    return ok ? static_cast<int>(data.size()) : 0;
}

bool DataApi::createData(const std::string &sid, const std::vector<std::uint8_t> &data) {
    if (!store_.exists(sid)) {
        if (!store_.create(sid)) return false;
    }
    return store_.append(sid, data);
}

std::vector<std::uint8_t> DataApi::findData(const std::string &sid) {
    // 简化：尝试向 DHT 最接近的若干节点请求数据
    auto key = sha1_key(sid);
    auto nodes = dht_.find_closest(key, 8);
    for (const auto &n : nodes) {
        auto p = router_.get(n.info.pid.id);
        if (p && p->is_online() && p->session()) {
            // 简易协议：发送 "GET:" + sid，等待对方通过会话返回数据
            std::string req = std::string("GET:") + sid;
            p->session()->send(std::vector<std::uint8_t>(req.begin(), req.end()));
            // 这里应有异步/回调来接收返回；当前返回空占位
        }
    }
    return {};
}

} // namespace p2p


