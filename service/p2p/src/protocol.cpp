#include "p2p/protocol.hpp"

#include <cstring>

namespace p2p {

static void write_u16(uint8_t* p, uint16_t v) { p[0]=uint8_t((v>>8)&0xFF); p[1]=uint8_t(v&0xFF); }
static void write_u32(uint8_t* p, uint32_t v) { p[0]=uint8_t((v>>24)&0xFF); p[1]=uint8_t((v>>16)&0xFF); p[2]=uint8_t((v>>8)&0xFF); p[3]=uint8_t(v&0xFF); }
static uint16_t read_u16(const uint8_t* p){ return (uint16_t(p[0])<<8) | uint16_t(p[1]); }
static uint32_t read_u32(const uint8_t* p){ return (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|uint32_t(p[3]); }

std::vector<uint8_t> pack_cmd(CmdCode code, const std::vector<uint8_t>& payload) {
    MsgHeader h{}; h.type = static_cast<uint8_t>(MsgType::Command); h.code = static_cast<uint8_t>(code); h.len = static_cast<uint32_t>(payload.size());
    std::vector<uint8_t> out(sizeof(MsgHeader) + payload.size());
    uint8_t* p = out.data();
    p[0]=h.type; p[1]=h.code; write_u16(p+2, h.rsv); write_u32(p+4, h.len);
    if (!payload.empty()) std::memcpy(p+sizeof(MsgHeader), payload.data(), payload.size());
    return out;
}

std::vector<uint8_t> pack_data(const std::vector<uint8_t>& payload) {
    MsgHeader h{}; h.type = static_cast<uint8_t>(MsgType::Data); h.code = 0; h.len = static_cast<uint32_t>(payload.size());
    std::vector<uint8_t> out(sizeof(MsgHeader) + payload.size());
    uint8_t* p = out.data();
    p[0]=h.type; p[1]=h.code; write_u16(p+2, h.rsv); write_u32(p+4, h.len);
    if (!payload.empty()) std::memcpy(p+sizeof(MsgHeader), payload.data(), payload.size());
    return out;
}

bool unpack(const std::vector<uint8_t>& buf, MsgHeader& hdr, std::vector<uint8_t>& payload) {
    if (buf.size() < sizeof(MsgHeader)) return false;
    const uint8_t* p = buf.data();
    hdr.type = p[0]; hdr.code = p[1]; hdr.rsv = read_u16(p+2); hdr.len = read_u32(p+4);
    if (buf.size() < sizeof(MsgHeader) + hdr.len) return false;
    payload.assign(buf.begin() + sizeof(MsgHeader), buf.begin() + sizeof(MsgHeader) + hdr.len);
    return true;
}

} // namespace p2p


