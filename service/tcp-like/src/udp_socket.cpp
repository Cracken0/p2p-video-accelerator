#include "tcp_like/udp_socket.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

namespace tcp_like {

UdpSocket::UdpSocket() : fd_(-1) {}
UdpSocket::~UdpSocket() { close(); }

bool UdpSocket::open() {
    if (fd_ != -1) return true;
    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    return fd_ != -1;
}

void UdpSocket::close() {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool UdpSocket::bind(const Endpoint& local) {
    if (fd_ == -1 && !open()) return false;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(local.port);
    if (::inet_pton(AF_INET, local.address.c_str(), &addr.sin_addr) != 1) {
        return false;
    }
    int enable = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    return ::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
}

bool UdpSocket::setNonBlocking(bool enable) {
    if (fd_ == -1) return false;
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) return false;
    if (enable) flags |= O_NONBLOCK; else flags &= ~O_NONBLOCK;
    return fcntl(fd_, F_SETFL, flags) == 0;
}

bool UdpSocket::setRecvTimeoutMs(int timeoutMs) {
    if (fd_ == -1) return false;
    timeval tv{};
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    return ::setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == 0;
}

int UdpSocket::sendTo(const void* data, size_t len, const Endpoint& remote) {
    if (fd_ == -1) return -1;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote.port);
    if (::inet_pton(AF_INET, remote.address.c_str(), &addr.sin_addr) != 1) {
        return -1;
    }
    ssize_t sent = ::sendto(fd_, data, len, 0, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    return static_cast<int>(sent);
}

int UdpSocket::recvFrom(void* buffer, size_t len, Endpoint& remoteOut) {
    if (fd_ == -1) return -1;
    sockaddr_in addr{};
    socklen_t addrlen = sizeof(addr);
    ssize_t recvd = ::recvfrom(fd_, buffer, len, 0, reinterpret_cast<sockaddr*>(&addr), &addrlen);
    if (recvd >= 0) {
        char ip[INET_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
        remoteOut.address = ip;
        remoteOut.port = ntohs(addr.sin_port);
    }
    return static_cast<int>(recvd);
}

} // namespace tcp_like







