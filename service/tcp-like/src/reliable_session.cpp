#include "tcp_like/reliable_session.hpp"

#include <algorithm>
#include <cstring>

namespace tcp_like {

namespace {
constexpr int kResendTimeoutMs = 300;
constexpr int kMaxRetries = 10;
enum PacketFlags : uint32_t { FLAG_NONE=0, FLAG_SYN=1u<<0, FLAG_ACK=1u<<1, FLAG_FIN=1u<<2, FLAG_RST=1u<<3, FLAG_DATA=1u<<4, FLAG_PING=1u<<5, FLAG_PONG=1u<<6 };
}

ReliableSession::ReliableSession()
    : running_(false), nextSequence_(1), lastReceivedSequence_(0), recvMask_(0), state_(State::Closed) {}

bool ReliableSession::connect(const Endpoint& local, const Endpoint& remote) {
    if (!socket_.open()) return false;
    if (!socket_.bind(local)) return false;
    socket_.setNonBlocking(true);
    socket_.setRecvTimeoutMs(1);
    remote_ = remote;
    running_ = true;
    state_ = State::SynSent;
    lastActivityAt_ = std::chrono::steady_clock::now();
    rateWindowStart_ = lastActivityAt_;
    lastHeartbeatSentAt_ = lastActivityAt_;
    lastHeartbeatRecvAt_ = lastActivityAt_;
    // client: send initial SYN
    sendPacket({}, nextSequence_++, 0, 0, FLAG_SYN);
    return true;
}

bool ReliableSession::listen(const Endpoint& local) {
    if (!socket_.open()) return false;
    if (!socket_.bind(local)) return false;
    socket_.setNonBlocking(true);
    socket_.setRecvTimeoutMs(1);
    running_ = true;
    state_ = State::Closed; // waiting for acceptFrom(remote)
    lastActivityAt_ = std::chrono::steady_clock::now();
    rateWindowStart_ = lastActivityAt_;
    lastHeartbeatSentAt_ = lastActivityAt_;
    return true;
}

void ReliableSession::acceptFrom(const Endpoint& remote) {
    remote_ = remote;
    state_ = State::SynReceived; // expect SYN, then reply SYN-ACK
}

void ReliableSession::stop() {
    running_ = false;
    socket_.close();
    state_ = State::Closed;
}

bool ReliableSession::send(const std::vector<uint8_t>& payload) {
    if (!running_) return false;
    if (state_ != State::Established) return false;
    InflightPacket pkt{nextSequence_++, payload, std::chrono::steady_clock::now(), 0};
    sendQueue_.push_back(pkt);
    stats_.bytesSentTotal += payload.size();
    bytesSentWindow_ += payload.size();
    return true;
}

bool ReliableSession::receive(std::vector<uint8_t>& outPayload) {
    if (recvQueue_.empty()) return false;
    outPayload = std::move(recvQueue_.front());
    recvQueue_.pop_front();
    return true;
}

void ReliableSession::update() {
    if (!running_) return;

    uint8_t buffer[1500];
    Endpoint from{};
    int n = 0;
    while ((n = socket_.recvFrom(buffer, sizeof(buffer), from)) > 0) {
        handleIncoming(buffer, static_cast<size_t>(n));
    }

    uint32_t ack = lastReceivedSequence_;
    uint32_t ackBits = recvMask_;

    for (auto& pkt : sendQueue_) {
        auto now = std::chrono::steady_clock::now();
        if (pkt.retries == 0 || std::chrono::duration_cast<std::chrono::milliseconds>(now - pkt.lastSendAt).count() >= kResendTimeoutMs) {
            sendPacket(pkt.data, pkt.sequence, ack, ackBits, FLAG_DATA | FLAG_ACK);
            pkt.lastSendAt = now;
            pkt.retries++;
        }
    }

    sendQueue_.erase(
        std::remove_if(sendQueue_.begin(), sendQueue_.end(), [&](const InflightPacket& pkt){
            return pkt.retries > kMaxRetries; // give up after too many retries
        }),
        sendQueue_.end()
    );

    // heartbeat + idle timeout
    auto now = std::chrono::steady_clock::now();
    maybeSendHeartbeat(ack, ackBits);
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastActivityAt_).count() > idleTimeoutMs_) {
        state_ = State::TimeWait;
        stop();
    }

    updateRates();
}

void ReliableSession::handleIncoming(const uint8_t* data, size_t len) {
    if (len < sizeof(PacketHeader)) return;
    PacketHeader hdr{};
    std::memcpy(&hdr, data, sizeof(hdr));

    // Handshake processing
    processHandshake(hdr.flags);

    // ACK our inflight packets
    sendQueue_.erase(
        std::remove_if(sendQueue_.begin(), sendQueue_.end(), [&](const InflightPacket& pkt){
            if (pkt.sequence == hdr.ack) return true;
            if (pkt.sequence < hdr.ack) {
                uint32_t diff = hdr.ack - pkt.sequence;
                if (diff <= 32 && (hdr.ackBits & (1u << (diff - 1)))) return true;
            }
            return false;
        }),
        sendQueue_.end()
    );

    // Receive payload
    const uint8_t* payload = data + sizeof(PacketHeader);
    size_t payloadLen = len - sizeof(PacketHeader);

    if (hdr.flags & FLAG_PING) {
        // reply PONG
        sendPacket({}, nextSequence_++, lastReceivedSequence_, recvMask_, FLAG_PONG | FLAG_ACK);
    } else if (hdr.flags & FLAG_PONG) {
        lastHeartbeatRecvAt_ = std::chrono::steady_clock::now();
    } else if (payloadLen > 0) {
        deliverInOrder(hdr.sequence, payload, payloadLen);
        stats_.bytesRecvTotal += payloadLen;
        bytesRecvWindow_ += payloadLen;
    }

    lastActivityAt_ = std::chrono::steady_clock::now();
}

void ReliableSession::sendPacket(const std::vector<uint8_t>& payload, uint32_t sequence, uint32_t ack, uint32_t ackBits, uint32_t flags) {
    std::vector<uint8_t> buf(sizeof(PacketHeader) + payload.size());
    PacketHeader hdr{sequence, ack, ackBits, flags};
    std::memcpy(buf.data(), &hdr, sizeof(hdr));
    if (!payload.empty()) std::memcpy(buf.data() + sizeof(hdr), payload.data(), payload.size());
    socket_.sendTo(buf.data(), buf.size(), remote_);
}

void ReliableSession::processHandshake(uint32_t flags) {
    using S = State;
    if (state_ == S::SynSent) {
        if ((flags & FLAG_SYN) && (flags & FLAG_ACK)) {
            // Received SYN-ACK, reply ACK
            sendPacket({}, nextSequence_++, lastReceivedSequence_, recvMask_, FLAG_ACK);
            state_ = S::Established;
            lastHeartbeatRecvAt_ = std::chrono::steady_clock::now();
        }
    } else if (state_ == S::SynReceived) {
        if (flags & FLAG_SYN) {
            // reply SYN-ACK
            sendPacket({}, nextSequence_++, lastReceivedSequence_, recvMask_, FLAG_SYN | FLAG_ACK);
        } else if (flags & FLAG_ACK) {
            state_ = S::Established;
            lastHeartbeatRecvAt_ = std::chrono::steady_clock::now();
        }
    }
}

void ReliableSession::deliverInOrder(uint32_t seq, const uint8_t* payload, size_t len) {
    // Update ack window
    if (seq > lastReceivedSequence_) {
        uint32_t shift = seq - lastReceivedSequence_;
        if (shift >= 32) recvMask_ = 0; else recvMask_ <<= shift;
        lastReceivedSequence_ = seq;
        recvMask_ |= 1u;
    } else {
        uint32_t diff = lastReceivedSequence_ - seq;
        if (diff < 32) recvMask_ |= (1u << diff);
    }

    // Reassembly
    if (seq == expectedSequence_) {
        recvQueue_.emplace_back(payload, payload + len);
        expectedSequence_++;
        // drain buffered consecutive packets
        while (true) {
            auto it = reorderBuffer_.find(expectedSequence_);
            if (it == reorderBuffer_.end()) break;
            recvQueue_.push_back(std::move(it->second));
            reorderBuffer_.erase(it);
            expectedSequence_++;
        }
    } else if (seq > expectedSequence_) {
        reorderBuffer_[seq] = std::vector<uint8_t>(payload, payload + len);
    } // else duplicate, drop
}

void ReliableSession::setIdleTimeoutMs(int ms) { idleTimeoutMs_ = ms; }
bool ReliableSession::isEstablished() const { return state_ == State::Established; }

void ReliableSession::setHeartbeatEnabled(bool enabled) { heartbeatEnabled_ = enabled; }
void ReliableSession::setHeartbeatIntervalMs(int ms) { heartbeatIntervalMs_ = ms; }

void ReliableSession::updateRates() {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - rateWindowStart_).count() / 1000.0;
    if (elapsed >= 0.5) {
        stats_.sendRateBps = bytesSentWindow_ * 8.0 / elapsed;
        stats_.recvRateBps = bytesRecvWindow_ * 8.0 / elapsed;
        bytesSentWindow_ = 0;
        bytesRecvWindow_ = 0;
        rateWindowStart_ = now;
    }
}

void ReliableSession::maybeSendHeartbeat(uint32_t ack, uint32_t ackBits) {
    if (!heartbeatEnabled_) return;
    auto now = std::chrono::steady_clock::now();
    if (lastHeartbeatSentAt_.time_since_epoch().count() == 0) {
        lastHeartbeatSentAt_ = now;
        return;
    }
    auto since = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHeartbeatSentAt_).count();
    if (since >= heartbeatIntervalMs_) {
        sendPacket({}, nextSequence_++, ack, ackBits, FLAG_PING | FLAG_ACK);
        lastHeartbeatSentAt_ = now;
    }
}

} // namespace tcp_like


