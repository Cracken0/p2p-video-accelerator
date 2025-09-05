#include "repeater/codec.hpp"
#include "repeater/server.hpp"

#include <arpa/inet.h>
#include <cstring>

namespace repeater {

std::uint32_t to_be32(std::uint32_t v) { return htonl(v); }
std::uint32_t from_be32(std::uint32_t v) { return ntohl(v); }

bool encode_header(std::vector<std::uint8_t> &out, const MessageHeader &h) {
    out.resize(sizeof(MessageHeader));
    std::uint8_t *p = out.data();
    std::memset(p, 0, sizeof(MessageHeader));
    p[0] = h.funcType;
    std::memcpy(p + 1, h.field, 32);
    std::uint32_t len_be = to_be32(h.len);
    std::memcpy(p + 33, &len_be, sizeof(len_be));
    return true;
}

bool decode_header(const std::uint8_t *buf, std::size_t len, MessageHeader &h) {
    if (len < sizeof(MessageHeader)) return false;
    std::memset(&h, 0, sizeof(h));
    h.funcType = buf[0];
    std::memcpy(h.field, buf + 1, 32);
    std::uint32_t len_be = 0;
    std::memcpy(&len_be, buf + 33, sizeof(len_be));
    h.len = from_be32(len_be);
    return true;
}

} // namespace repeater


