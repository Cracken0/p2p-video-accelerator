#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace repeater {

enum class FieldType : std::uint8_t {
    Getter = 1,
    Setter = 2,
};

struct MessageHeader {
    std::uint8_t funcType; // 1 getter, 2 setter
    char field[32];        // null-terminated if shorter than 32
    std::uint32_t len;     // big endian length of data
};

// 编码/解码在 codec.hpp/.cpp
class Codec;

// 字段注册中心：将字段名绑定到读写回调
class FieldRegistry {
public:
    using GetHandler = std::function<std::vector<std::uint8_t>()>;
    using SetHandler = std::function<bool(const std::vector<std::uint8_t>&)>;

    void register_getter(const std::string &fieldName, GetHandler getter);
    void register_setter(const std::string &fieldName, SetHandler setter);

    GetHandler find_getter(const std::string &fieldName) const;
    SetHandler find_setter(const std::string &fieldName) const;

private:
    std::unordered_map<std::string, GetHandler> getters_;
    std::unordered_map<std::string, SetHandler> setters_;
};

// 简易 Unix Domain Socket 服务器
class IpcServer {
public:
    IpcServer(const std::string &socketPath, std::shared_ptr<FieldRegistry> registry);
    ~IpcServer();

    bool start();
    void stop();

private:
    int listenFd_ = -1;
    std::string socketPath_;
    std::shared_ptr<FieldRegistry> registry_;
    bool running_ = false;

    void accept_loop();
    void handle_client(int clientFd);
};

} // namespace repeater


