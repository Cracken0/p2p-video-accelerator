#include "repeater/server.hpp"
#include "repeater/codec.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <thread>

namespace repeater {

namespace {

static bool write_all(int fd, const std::uint8_t *buf, std::size_t len) {
    std::size_t off = 0;
    while (off < len) {
        ssize_t n = ::write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += static_cast<std::size_t>(n);
    }
    return true;
}

static bool read_exact(int fd, std::uint8_t *buf, std::size_t len) {
    std::size_t off = 0;
    while (off < len) {
        ssize_t n = ::read(fd, buf + off, len - off);
        if (n == 0) return false; // EOF
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += static_cast<std::size_t>(n);
    }
    return true;
}

} // namespace

IpcServer::IpcServer(const std::string &socketPath, std::shared_ptr<FieldRegistry> registry)
    : socketPath_(socketPath), registry_(std::move(registry)) {}

IpcServer::~IpcServer() { stop(); }

bool IpcServer::start() {
    if (running_) return true;
    listenFd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (listenFd_ < 0) return false;

    ::unlink(socketPath_.c_str());

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", socketPath_.c_str());
    if (::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }
    if (::listen(listenFd_, 16) < 0) {
        ::close(listenFd_);
        listenFd_ = -1;
        return false;
    }
    running_ = true;
    std::thread(&IpcServer::accept_loop, this).detach();
    return true;
}

void IpcServer::stop() {
    if (!running_) return;
    running_ = false;
    if (listenFd_ >= 0) {
        ::close(listenFd_);
        listenFd_ = -1;
    }
    ::unlink(socketPath_.c_str());
}

void IpcServer::accept_loop() {
    while (running_) {
        int cfd = ::accept(listenFd_, nullptr, nullptr);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (!running_) break;
            continue;
        }
        std::thread(&IpcServer::handle_client, this, cfd).detach();
    }
}

void IpcServer::handle_client(int clientFd) {
    std::uint8_t hdrbuf[sizeof(MessageHeader)]{};
    while (true) {
        if (!read_exact(clientFd, hdrbuf, sizeof(hdrbuf))) break;
        MessageHeader h{};
        if (!decode_header(hdrbuf, sizeof(hdrbuf), h)) break;
        std::vector<std::uint8_t> data(h.len);
        if (h.len > 0) {
            if (!read_exact(clientFd, data.data(), data.size())) break;
        }

        std::string field(h.field, h.field + 32);
        if (auto pos = field.find('\0'); pos != std::string::npos) field.resize(pos);

        std::vector<std::uint8_t> resp;
        bool ok = false;
        if (h.funcType == static_cast<std::uint8_t>(FieldType::Getter)) {
            auto g = registry_->find_getter(field);
            if (g) { resp = g(); ok = true; }
        } else if (h.funcType == static_cast<std::uint8_t>(FieldType::Setter)) {
            auto s = registry_->find_setter(field);
            if (s) { ok = s(data); }
        }

        // 回写：约定返回头 funcType=原funcType，len=resp长度，data为值；若失败则len=0
        MessageHeader rh{};
        rh.funcType = h.funcType;
        std::memset(rh.field, 0, sizeof(rh.field));
        std::snprintf(rh.field, sizeof(rh.field), "%s", field.c_str());
        rh.len = ok ? static_cast<std::uint32_t>(resp.size()) : 0u;
        std::vector<std::uint8_t> out;
        encode_header(out, rh);
        if (!write_all(clientFd, out.data(), out.size())) break;
        if (rh.len > 0) {
            if (!write_all(clientFd, resp.data(), resp.size())) break;
        }
    }
    ::close(clientFd);
}

} // namespace repeater


