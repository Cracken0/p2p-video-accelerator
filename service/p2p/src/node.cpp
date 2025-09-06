#include "p2p/node.hpp"

#include <chrono>

namespace p2p {

Node::Node(RouterTable& router, KademliaTable& dht, store::FileStore& store, nat::NatAgent& nat)
    : router_(router), dht_(dht), store_(store), dataApi_(dht, router, store), nat_(nat) {}

Node::~Node() { stop(); }

void Node::start() {
    if (running_.exchange(true)) return;
    th_ = std::thread([this]{ this->loop(); });
}

void Node::stop() {
    if (!running_.exchange(false)) return;
    if (th_.joinable()) th_.join();
}

bool Node::request_assist(const std::string& helperPid, const std::string& targetPid) {
    auto helper = router_.get(helperPid);
    if (!helper || !helper->is_online() || !helper->session()) return false;
    std::string body = targetPid;
    auto pkt = pack_cmd(CmdCode::AskAssist, std::vector<uint8_t>(body.begin(), body.end()));
    return helper->session()->send(pkt);
}

bool Node::request_get(const std::string& pid, const std::string& sid) {
    auto p = router_.get(pid);
    if (!p || !p->is_online() || !p->session()) return false;
    auto pkt = pack_cmd(CmdCode::Get, std::vector<uint8_t>(sid.begin(), sid.end()));
    return p->session()->send(pkt);
}

void Node::loop() {
    while (running_.load()) {
        // 轮询所有在线 Peer 的会话
        for (auto& info : router_.list_online()) {
            auto p = router_.get(info.pid.id);
            if (!p || !p->session()) continue;
            p->session()->update();
            std::vector<uint8_t> buf;
            while (p->session()->receive(buf)) {
                handle_message(*p, buf);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void Node::handle_message(Peer& from, const std::vector<uint8_t>& msg) {
    MsgHeader h{}; std::vector<uint8_t> payload;
    if (!unpack(msg, h, payload)) return;
    if (h.type == static_cast<uint8_t>(MsgType::Command)) {
        on_cmd(from, static_cast<CmdCode>(h.code), payload);
    } else if (h.type == static_cast<uint8_t>(MsgType::Data)) {
        // 简单落地：Data 作为资源片段写入，sid 在 payload 前缀以 'SID:' + sid + '\n' 编码（示例）
        // 为简化，这里不实现具体分片协议
    }
}

void Node::on_cmd(Peer& from, CmdCode code, const std::vector<uint8_t>& payload) {
    switch (code) {
        case CmdCode::Hello: {
            // 上线通知：可更新路由或回应
            break;
        }
        case CmdCode::AskAssist: {
            // 收到“协助我连接 targetPid”的请求
            std::string targetPid(payload.begin(), payload.end());
            auto target = router_.get(targetPid);
            if (target && target->is_online()) {
                // 已在线，向其发送 AssistOffer，请它与请求方直连
                std::string offer = from.info().pid.id; // 请求方 id
                auto pkt = pack_cmd(CmdCode::AssistOffer, std::vector<uint8_t>(offer.begin(), offer.end()));
                target->session()->send(pkt);
            }
            break;
        }
        case CmdCode::AssistOffer: {
            // “请向某某发起连接”
            std::string toPid(payload.begin(), payload.end());
            auto peer = router_.get(toPid);
            if (peer && !peer->is_online()) {
                peer->connect(nat_);
            }
            break;
        }
        case CmdCode::Get: {
            // 对方请求资源 sid，直接从本地 store 读一段返回（演示）
            std::string sid(payload.begin(), payload.end());
            auto data = store_.read(sid, 0, 64 * 1024);
            if (!data.empty()) {
                auto pkt = pack_data(data);
                from.session()->send(pkt);
            }
            break;
        }
        case CmdCode::Put: {
            // 对方传来资源片段（这里不展开具体格式）
            break;
        }
        default: break;
    }
}

} // namespace p2p


