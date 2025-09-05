#include "store/store.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <chrono>
#include <fstream>
#include <algorithm>

namespace store {

using Clock = std::chrono::steady_clock;

static std::uint64_t now_unix_ms() {
    using namespace std::chrono;
    auto now = time_point_cast<milliseconds>(system_clock::now());
    return static_cast<std::uint64_t>(now.time_since_epoch().count());
}

static std::uint64_t now_steady_ms() {
    using namespace std::chrono;
    auto now = time_point_cast<milliseconds>(Clock::now());
    return static_cast<std::uint64_t>(now.time_since_epoch().count());
}

static bool make_dir_if_not_exists(const std::string &path) {
    struct stat st{};
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(path.c_str(), 0755) == 0) {
        return true;
    }
    return false;
}

FileStore::FileStore(const std::string &rootDirectory)
    : rootDir_(rootDirectory), metaFilePath_(rootDirectory + "/data") {
    ensure_root_ready();
    load_meta();
    refresh_used_size_from_disk();
    timerThread_ = std::thread([this]{ this->timer_loop(); });
}

FileStore::~FileStore() {
    stop_.store(true);
    if (timerThread_.joinable()) {
        timerThread_.join();
    }
    // 关闭所有打开的fd
    std::lock_guard<std::mutex> lk(mutex_);
    for (auto &kv : openFds_) {
        if (kv.second.fd >= 0) close(kv.second.fd);
    }
}

bool FileStore::ensure_root_ready() {
    return make_dir_if_not_exists(rootDir_);
}

bool FileStore::is_valid_sid(const std::string &sid) {
    if (sid.empty() || sid.size() > 255) return false;
    for (char c : sid) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='_' || c=='-' || c=='.')) {
            return false;
        }
    }
    return true;
}

std::string FileStore::to_path(const std::string &sid) const {
    return rootDir_ + "/" + sid;
}

bool FileStore::load_meta() {
    meta_.clear();
    std::ifstream in(metaFilePath_, std::ios::binary);
    if (!in.is_open()) return true; // 第一次启动没有meta文件
    std::uint64_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    for (std::uint64_t i = 0; i < count; ++i) {
        std::uint64_t nameLen = 0; in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        std::string name(nameLen, '\0'); in.read(name.data(), nameLen);
        MetaInfo m{};
        in.read(reinterpret_cast<char*>(&m.lastModifiedEpochMs), sizeof(m.lastModifiedEpochMs));
        in.read(reinterpret_cast<char*>(&m.sizeBytes), sizeof(m.sizeBytes));
        if (!name.empty()) meta_[name] = m;
    }
    return true;
}

bool FileStore::save_meta() {
    std::ofstream out(metaFilePath_, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return false;
    std::uint64_t count = meta_.size();
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto &kv : meta_) {
        const std::string &name = kv.first;
        const MetaInfo &m = kv.second;
        std::uint64_t nameLen = name.size();
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        out.write(name.data(), name.size());
        out.write(reinterpret_cast<const char*>(&m.lastModifiedEpochMs), sizeof(m.lastModifiedEpochMs));
        out.write(reinterpret_cast<const char*>(&m.sizeBytes), sizeof(m.sizeBytes));
    }
    return true;
}

void FileStore::refresh_used_size_from_disk() {
    std::uint64_t total = 0;
    DIR *dir = opendir(rootDir_.c_str());
    if (!dir) { usedSizeBytes_ = 0; return; }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) continue;
        std::string name = entry->d_name;
        if (name == "data") continue;
        std::string path = rootDir_ + "/" + name;
        struct stat st{};
        if (stat(path.c_str(), &st) == 0) {
            total += static_cast<std::uint64_t>(st.st_size);
        }
    }
    closedir(dir);
    usedSizeBytes_ = total;
}

void FileStore::touch_modified(const std::string &sid) {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = meta_.find(sid);
    if (it != meta_.end()) {
        it->second.lastModifiedEpochMs = now_unix_ms();
    }
}

void FileStore::touch_op(const std::string &sid) {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = openFds_.find(sid);
    if (it != openFds_.end()) {
        it->second.lastOpSteadyMs = now_steady_ms();
    }
}

bool FileStore::exists(const std::string &sid) {
    if (!is_valid_sid(sid)) return false;
    std::lock_guard<std::mutex> lk(mutex_);
    return meta_.find(sid) != meta_.end();
}

bool FileStore::create(const std::string &sid) {
    if (!is_valid_sid(sid)) return false;
    std::lock_guard<std::mutex> lk(mutex_);
    if (meta_.count(sid)) return false; // 不允许重名
    const std::string path = to_path(sid);
    int fd = ::open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (fd < 0) return false;
    ::close(fd);
    MetaInfo m{};
    m.lastModifiedEpochMs = now_unix_ms();
    m.sizeBytes = 0;
    meta_[sid] = m;
    save_meta();
    return true;
}

bool FileStore::remove(const std::string &sid) {
    if (!is_valid_sid(sid)) return false;
    std::lock_guard<std::mutex> lk(mutex_);
    const std::string path = to_path(sid);
    // 关闭可能打开的fd
    auto itfd = openFds_.find(sid);
    if (itfd != openFds_.end()) {
        if (itfd->second.fd >= 0) ::close(itfd->second.fd);
        openFds_.erase(itfd);
    }
    int rc = ::unlink(path.c_str());
    bool ok = (rc == 0);
    auto it = meta_.find(sid);
    if (it != meta_.end()) {
        usedSizeBytes_ -= std::min<std::uint64_t>(it->second.sizeBytes, usedSizeBytes_);
        meta_.erase(it);
        save_meta();
    }
    return ok;
}

bool FileStore::open_for_write_if_needed(const std::string &sid) {
    std::lock_guard<std::mutex> lk(mutex_);
    auto it = openFds_.find(sid);
    if (it != openFds_.end() && it->second.fd >= 0) {
        it->second.lastOpSteadyMs = now_steady_ms();
        return true;
    }
    const std::string path = to_path(sid);
    int fd = ::open(path.c_str(), O_CREAT | O_WRONLY, 0644);
    if (fd < 0) return false;
    OpenFdInfo info{};
    info.fd = fd;
    info.lastOpSteadyMs = now_steady_ms();
    openFds_[sid] = info;
    return true;
}

bool FileStore::ensure_capacity_for_growth(std::uint64_t extraBytesNeeded) {
    if (extraBytesNeeded == 0) return true;
    std::lock_guard<std::mutex> lk(mutex_);
    if (usedSizeBytes_ + extraBytesNeeded <= storageLimitBytes_) return true;
    // 需要释放空间
    std::uint64_t need = usedSizeBytes_ + extraBytesNeeded - storageLimitBytes_;
    // 不能在持锁情况下执行删除，返回由调用方在无锁状态下执行evict
    (void)need;
    return false;
}

bool FileStore::evict_until(std::uint64_t bytesNeeded) {
    // 删除最早修改时间的文件，直到腾出 bytesNeeded 空间
    while (bytesNeeded > 0) {
        std::string victim;
        std::uint64_t victimSize = 0;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            std::uint64_t t = UINT64_MAX;
            for (const auto &kv : meta_) {
                if (kv.second.lastModifiedEpochMs < t) {
                    t = kv.second.lastModifiedEpochMs;
                    victim = kv.first;
                    victimSize = kv.second.sizeBytes;
                }
            }
        }
        if (victim.empty()) return false; // 无可删文件
        if (!remove(victim)) return false;
        if (victimSize >= bytesNeeded) return true;
        bytesNeeded -= victimSize;
    }
    return true;
}

bool FileStore::append(const std::string &sid, const std::vector<std::uint8_t> &data) {
    if (!is_valid_sid(sid)) return false;
    // 若文件不存在则创建
    if (!exists(sid)) {
        if (!create(sid)) return false;
    }

    // 容量检查：尝试直接写，不足则淘汰
    {
        std::lock_guard<std::mutex> lk(mutex_);
        // just check; eviction happens outside the lock
    }
    if (get_used_size() + data.size() > get_storage_limit()) {
        std::uint64_t need = usedSizeBytes_ + data.size() - storageLimitBytes_;
        if (!evict_until(need)) return false;
    }

    if (!open_for_write_if_needed(sid)) return false;

    int fd = -1;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        fd = openFds_[sid].fd;
        openFds_[sid].lastOpSteadyMs = now_steady_ms();
    }

    if (::lseek(fd, 0, SEEK_END) < 0) return false;
    ssize_t written = ::write(fd, data.data(), data.size());
    if (written < 0 || static_cast<std::size_t>(written) != data.size()) return false;

    {
        std::lock_guard<std::mutex> lk(mutex_);
        MetaInfo &m = meta_[sid];
        m.sizeBytes += static_cast<std::uint64_t>(data.size());
        m.lastModifiedEpochMs = now_unix_ms();
        usedSizeBytes_ += static_cast<std::uint64_t>(data.size());
        save_meta();
    }
    return true;
}

std::vector<std::uint8_t> FileStore::read(const std::string &sid, int begin, int end) {
    std::vector<std::uint8_t> out;
    if (!is_valid_sid(sid)) return out;
    if (begin < 0 || end < 0 || end < begin) return out;
    const std::string path = to_path(sid);
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0) return out;
    off_t start = static_cast<off_t>(begin);
    off_t stop = static_cast<off_t>(end);
    if (::lseek(fd, start, SEEK_SET) < 0) { ::close(fd); return out; }
    std::size_t len = static_cast<std::size_t>(stop - start);
    out.resize(len);
    ssize_t r = ::read(fd, out.data(), len);
    if (r < 0) { ::close(fd); out.clear(); return out; }
    out.resize(static_cast<std::size_t>(r));
    ::close(fd);
    return out;
}

void FileStore::set_storage_limit(std::uint64_t bytes) {
    std::lock_guard<std::mutex> lk(mutex_);
    storageLimitBytes_ = bytes;
}

std::uint64_t FileStore::get_storage_limit() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return storageLimitBytes_;
}

std::uint64_t FileStore::get_used_size() const {
    std::lock_guard<std::mutex> lk(mutex_);
    return usedSizeBytes_;
}

void FileStore::maybe_close_idle_files() {
    const std::uint64_t nowMs = now_steady_ms();
    std::vector<std::string> toClose;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        for (auto &kv : openFds_) {
            if (kv.second.fd >= 0) {
                std::uint64_t idle = nowMs - kv.second.lastOpSteadyMs;
                if (idle >= 5000) {
                    toClose.push_back(kv.first);
                }
            }
        }
        for (const auto &sid : toClose) {
            int fd = openFds_[sid].fd;
            if (fd >= 0) ::close(fd);
            openFds_[sid].fd = -1;
        }
    }
}

void FileStore::timer_loop() {
    while (!stop_.load()) {
        maybe_close_idle_files();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

} // namespace store


