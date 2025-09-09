#include "nat_traversal/nat_detector.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <vector>
#include <set>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>

namespace nat {

NatDetector::NatDetector() = default;

bool NatDetector::detect(const tcp_like::Endpoint& local, const tcp_like::Endpoint& stunServer) {
    // 保留现有单 STUN 绑定查询，加入多IP对称性测试，支持域名地址
    constexpr std::uint16_t BINDING_REQUEST = 0x0001;
    constexpr std::uint16_t BINDING_SUCCESS = 0x0101;
    constexpr std::uint32_t MAGIC_COOKIE = 0x2112A442;
    constexpr std::uint16_t ATTR_XOR_MAPPED_ADDRESS = 0x0020;
    constexpr std::uint16_t ATTR_MAPPED_ADDRESS = 0x0001;

    auto resolveIPv4 = [](const std::string& host) -> std::vector<std::string> {
        std::vector<std::string> out;
        addrinfo hints{}; hints.ai_family = AF_INET; hints.ai_socktype = SOCK_DGRAM;
        addrinfo* res = nullptr;
        if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) return out;
        std::set<std::string> uniq;
        for (auto p = res; p != nullptr; p = p->ai_next) {
            char ip[INET_ADDRSTRLEN]{};
            auto* sin = reinterpret_cast<sockaddr_in*>(p->ai_addr);
            if (inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip))) uniq.insert(ip);
        }
        freeaddrinfo(res);
        out.assign(uniq.begin(), uniq.end());
        return out;
    };

    auto hton32 = [](std::uint32_t v){ return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) | ((v & 0x00FF0000u) >> 8) | ((v & 0xFF000000u) >> 24); };

    auto send_binding = [&](tcp_like::UdpSocket& sock, const std::string& ip, uint16_t port, std::string& outIp, std::uint16_t& outPort) -> bool {
        std::array<std::uint8_t, 20> req{};
        req[0] = 0x00; req[1] = 0x01; // Binding Request
        req[2] = 0x00; req[3] = 0x00; // length=0
        std::uint32_t cookie_be = hton32(MAGIC_COOKIE);
        std::memcpy(req.data() + 4, &cookie_be, 4);
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int i = 0; i < 12; ++i) req[8 + i] = static_cast<std::uint8_t>((now >> (i * 5)) & 0xFF);

        tcp_like::Endpoint dst{ip, port};
        if (sock.sendTo(req.data(), req.size(), dst) != static_cast<int>(req.size())) return false;

        tcp_like::Endpoint from{};
        std::uint8_t buf[1024];
        int n = sock.recvFrom(buf, sizeof(buf), from);
        if (n < 20) return false;
        std::uint16_t mtype = (static_cast<std::uint16_t>(buf[0]) << 8) | buf[1];
        if (mtype != BINDING_SUCCESS) return false;

        int pos = 20;
        while (pos + 4 <= n) {
            std::uint16_t at = (static_cast<std::uint16_t>(buf[pos]) << 8) | buf[pos+1];
            std::uint16_t alen = (static_cast<std::uint16_t>(buf[pos+2]) << 8) | buf[pos+3];
            pos += 4;
            if (pos + alen > n) break;
            if ((at == ATTR_XOR_MAPPED_ADDRESS || at == ATTR_MAPPED_ADDRESS) && alen >= 8) {
                std::uint8_t family = buf[pos+1];
                if (family == 0x01) {
                    std::uint16_t portN = (static_cast<std::uint16_t>(buf[pos+2]) << 8) | buf[pos+3];
                    std::uint16_t port = (at == ATTR_XOR_MAPPED_ADDRESS) ? static_cast<std::uint16_t>(portN ^ ((MAGIC_COOKIE >> 16) & 0xFFFF)) : portN;
                    std::uint32_t addrN = (static_cast<std::uint32_t>(buf[pos+4]) << 24) | (static_cast<std::uint32_t>(buf[pos+5]) << 16) | (static_cast<std::uint32_t>(buf[pos+6]) << 8) | static_cast<std::uint32_t>(buf[pos+7]);
                    std::uint32_t addr = (at == ATTR_XOR_MAPPED_ADDRESS) ? (addrN ^ MAGIC_COOKIE) : addrN;
                    std::string ipOut = std::to_string((addr >> 24) & 0xFF) + "." + std::to_string((addr >> 16) & 0xFF) + "." + std::to_string((addr >> 8) & 0xFF) + "." + std::to_string(addr & 0xFF);
                    outIp = ipOut; outPort = port; return true;
                }
            }
            int pad = (4 - (alen % 4)) % 4; pos += alen + pad;
        }
        return false;
    };

    auto ips = resolveIPv4(stunServer.address);
    uint16_t stunPort = stunServer.port == 0 ? 3478 : stunServer.port;
    if (ips.empty()) { type_ = NatType::Unknown; mapped_.reset(); return false; }

    tcp_like::UdpSocket sock;
    if (!sock.open()) { type_ = NatType::Unknown; mapped_.reset(); return false; }
    if (!sock.bind(local)) { type_ = NatType::Unknown; mapped_.reset(); return false; }
    sock.setRecvTimeoutMs(1500);

    std::string ip1_map; std::uint16_t port1_map = 0;
    if (!send_binding(sock, ips[0], stunPort, ip1_map, port1_map)) { type_ = NatType::Unknown; mapped_.reset(); return false; }
    mapped_ = Candidate{ip1_map, port1_map};

    auto isPrivate = [](const std::string& ip){
        if (ip.rfind("10.", 0) == 0) return true;
        if (ip.rfind("192.168.", 0) == 0) return true;
        if (ip.rfind("172.", 0) == 0) { auto dot = ip.find('.', 4); if (dot != std::string::npos) { int second = std::atoi(ip.substr(4, dot-4).c_str()); if (second >= 16 && second <= 31) return true; } }
        if (ip.rfind("127.", 0) == 0) return true; return false; };

    if (local.address == ip1_map && local.port == port1_map) { type_ = NatType::OpenInternet; return true; }

    if (ips.size() >= 2) {
        std::string ip2_map; std::uint16_t port2_map = 0;
        if (send_binding(sock, ips[1], stunPort, ip2_map, port2_map)) {
            if (ip2_map != ip1_map || port2_map != port1_map) { type_ = NatType::Symmetric; return true; }
        }
    }
    type_ = NatType::PortRestrictedCone; // 最小化判定：保持保守
    return true;
}

} // namespace nat







