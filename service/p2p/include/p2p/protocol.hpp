#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace p2p {

// 轻量级应用层协议：统一封装命令与数据
enum class MsgType : uint8_t {
    Command = 1,
    Data = 2,
};

enum class CmdCode : uint8_t {
    AskAssist = 1,        // 协助我连接 pid
    AssistOffer = 2,      // 请向某某发起连接
    Get = 3,              // 获取资源 sid
    Put = 4,              // 上行资源 sid 分片
    Hello = 5,            // 握手/上线广播
};

struct MsgHeader {
    uint8_t  type;   // MsgType
    uint8_t  code;   // CmdCode for Command; reserved for Data
    uint16_t rsv{0};
    uint32_t len;    // payload size
};

// 序列化/反序列化
std::vector<uint8_t> pack_cmd(CmdCode code, const std::vector<uint8_t>& payload);
std::vector<uint8_t> pack_data(const std::vector<uint8_t>& payload);
bool unpack(const std::vector<uint8_t>& buf, MsgHeader& hdr, std::vector<uint8_t>& payload);

} // namespace p2p


