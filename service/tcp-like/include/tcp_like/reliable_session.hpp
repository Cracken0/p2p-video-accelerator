#pragma once

#include "tcp_like/udp_socket.hpp"

#include <cstdint>
#include <vector>
#include <deque>
#include <chrono>
#include <unordered_map>

namespace tcp_like {

struct PacketHeader {
    uint32_t sequence;
    uint32_t ack;
    uint32_t ackBits; // selective ACK bitmap (last 32 pkts)
    uint32_t flags;    // SYN, ACK, FIN, RST, DATA
};

class ReliableSession {
public:
    struct Stats {
        uint64_t bytesSentTotal{0};
        uint64_t bytesRecvTotal{0};
        double   sendRateBps{0.0};
        double   recvRateBps{0.0};
    };

    enum class State {
        Closed,
        SynSent,
        SynReceived,
        Established,
        FinWait,
        TimeWait
    };

    ReliableSession();

    // Client: connect(); Server: listen(local) + acceptFrom(remote)
    bool connect(const Endpoint& local, const Endpoint& remote);
    bool listen(const Endpoint& local);
    void acceptFrom(const Endpoint& remote);

    void stop();

    // Non-blocking send/recv (only works in Established)
    bool send(const std::vector<uint8_t>& payload);
    bool receive(std::vector<uint8_t>& outPayload);

    // Drive timers and retransmit/handshake, must be called periodically
    void update();

    // Config
    void setIdleTimeoutMs(int ms);
    void setHeartbeatEnabled(bool enabled);
    void setHeartbeatIntervalMs(int ms);

    // Query
    bool isEstablished() const;
    State state() const { return state_; }
    Stats getStats() const { return stats_; }

private:
    struct InflightPacket {
        uint32_t sequence;
        std::vector<uint8_t> data;
        std::chrono::steady_clock::time_point lastSendAt;
        int retries{0};
    };

    UdpSocket socket_;
    Endpoint remote_;
    bool running_;

    uint32_t nextSequence_;
    uint32_t lastReceivedSequence_;
    uint32_t recvMask_;

    // Handshake/state
    State state_;
    std::chrono::steady_clock::time_point lastActivityAt_;
    int idleTimeoutMs_{10000};
    bool heartbeatEnabled_{true};
    int heartbeatIntervalMs_{1000};
    std::chrono::steady_clock::time_point lastHeartbeatSentAt_;
    std::chrono::steady_clock::time_point lastHeartbeatRecvAt_;

    // Stats
    Stats stats_;
    std::chrono::steady_clock::time_point rateWindowStart_;
    uint64_t bytesSentWindow_{0};
    uint64_t bytesRecvWindow_{0};

    std::deque<InflightPacket> sendQueue_;
    std::deque<std::vector<uint8_t>> recvQueue_;
    std::unordered_map<uint32_t, std::vector<uint8_t>> reorderBuffer_;
    uint32_t expectedSequence_{1};

    void handleIncoming(const uint8_t* data, size_t len);
    void sendPacket(const std::vector<uint8_t>& payload, uint32_t sequence, uint32_t ack, uint32_t ackBits, uint32_t flags);
    void processHandshake(uint32_t flags);
    void deliverInOrder(uint32_t seq, const uint8_t* payload, size_t len);
    void updateRates();
    void maybeSendHeartbeat(uint32_t ack, uint32_t ackBits);
};

} // namespace tcp_like


