#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace repeater {

struct MessageHeader;

// 将 host uint32 转 big-endian 序列化
std::uint32_t to_be32(std::uint32_t v);
std::uint32_t from_be32(std::uint32_t v);

bool encode_header(std::vector<std::uint8_t> &out, const MessageHeader &h);
bool decode_header(const std::uint8_t *buf, std::size_t len, MessageHeader &h);

// 简化：data 直接原样传输

} // namespace repeater


