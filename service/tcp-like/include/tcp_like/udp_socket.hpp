#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace tcp_like {

struct Endpoint {
    std::string address;
    uint16_t port{0};
};

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    bool open();
    void close();

    bool bind(const Endpoint& local);
    bool setNonBlocking(bool enable);
    bool setRecvTimeoutMs(int timeoutMs);

    // Returns number of bytes sent or -1 on error
    int sendTo(const void* data, size_t len, const Endpoint& remote);

    // Returns number of bytes received or -1 on error
    int recvFrom(void* buffer, size_t len, Endpoint& remoteOut);

private:
    int fd_;
};

} // namespace tcp_like







