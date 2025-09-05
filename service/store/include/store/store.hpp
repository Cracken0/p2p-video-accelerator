#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>

namespace store {

class FileStore {
public:
    explicit FileStore(const std::string &rootDirectory);
    ~FileStore();

    // 基础接口
    bool exists(const std::string &sid);
    bool create(const std::string &sid);
    bool remove(const std::string &sid); // 与需求中的 delete 等价
    bool del(const std::string &sid) { return remove(sid); }
    bool append(const std::string &sid, const std::vector<std::uint8_t> &data);
    std::vector<std::uint8_t> read(const std::string &sid, int begin, int end);

    // 存储容量管理
    void set_storage_limit(std::uint64_t bytes);
    std::uint64_t get_storage_limit() const;
    std::uint64_t get_used_size() const;

private:
    struct MetaInfo {
        std::uint64_t lastModifiedEpochMs {0};
        std::uint64_t sizeBytes {0};
    };

    struct OpenFdInfo {
        int fd {-1};
        std::uint64_t lastOpSteadyMs {0};
    };

    // 内部工具
    bool ensure_root_ready();
    static bool is_valid_sid(const std::string &sid);
    std::string to_path(const std::string &sid) const;

    bool load_meta();
    bool save_meta();
    void refresh_used_size_from_disk();

    void touch_modified(const std::string &sid);
    void touch_op(const std::string &sid);

    bool ensure_capacity_for_growth(std::uint64_t extraBytesNeeded);
    bool evict_until(std::uint64_t bytesNeeded);
    std::pair<std::string, std::uint64_t> pick_oldest_locked() const;

    bool open_for_write_if_needed(const std::string &sid);
    void maybe_close_idle_files();
    void timer_loop();

private:
    std::string rootDir_;
    std::string metaFilePath_;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, MetaInfo> meta_; // sid -> meta
    std::unordered_map<std::string, OpenFdInfo> openFds_; // sid -> fd info

    std::uint64_t storageLimitBytes_ {static_cast<std::uint64_t>(1024ULL * 1024ULL * 1024ULL)}; // 默认1GiB
    std::uint64_t usedSizeBytes_ {0};

    std::atomic<bool> stop_ {false};
    std::thread timerThread_;
};

} // namespace store


