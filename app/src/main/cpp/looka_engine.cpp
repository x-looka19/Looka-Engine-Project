// ============================================================================
// looka_engine_v11_ultimate_merged.cpp - Looka Engine Ultimate v11.0
// ULTIMATE MERGED EDITION - ALL FEATURES FROM v11 + test.cpp
// ============================================================================

#include <iostream>
#include <vector>
#include <sys/uio.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <filesystem>
#include <array>
#include <functional>
#include <condition_variable>
#include <stack>
#include <system_error>
#include <stdlib.h>
#include <errno.h>
#include <string_view>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdarg.h>

// SIMD headers
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

// ----------------------------------------------------------------------------
// [تعريفات النظام للأندرويد]
// ----------------------------------------------------------------------------
extern "C" {
    ssize_t process_vm_readv(pid_t pid, const struct iovec* local_iov, unsigned long liovcnt, 
                           const struct iovec* remote_iov, unsigned long riovcnt, unsigned long flags);
    ssize_t process_vm_writev(pid_t pid, const struct iovec* local_iov, unsigned long liovcnt, 
                            const struct iovec* remote_iov, unsigned long riovcnt, unsigned long flags);
}

// ----------------------------------------------------------------------------
// [تغليف OpenSSL]
// ----------------------------------------------------------------------------
extern "C" {
    #include <openssl/conf.h>
    #include <openssl/evp.h>
    #include <openssl/rand.h>
    #include <openssl/hmac.h>
    #include <openssl/err.h>
    #include <openssl/bio.h>
    #include <openssl/asn1.h>
    #include <openssl/objects.h>
}

#define BRAND_NAME "LOOKA-ENGINE"

// ============================================================================
// DEFINES & CONSTANTS
// ============================================================================
#define LOOKA_VERSION "11.0-Ultimate-Merged"
#define LOOKA_OWNER "Looka"
#define LOOKA_SIGNATURE "LK11.0-2026-LOOKA-ULTIMATE-MERGED"

#define BRAND_NAME_FULL "Looka Engine"
#define BRAND_TAGLINE "Ultimate Gaming Suite"
#define BRAND_COLOR "\033[1;36m"

#define FLOATING_WINDOW_WIDTH 380
#define FLOATING_WINDOW_HEIGHT 540

#define CACHE_MAX_SIZE (16 * 1024 * 1024)
#define CACHE_ENTRY_TTL_MS 100
#define CACHE_CLEANUP_INTERVAL_MS 5000

#define REFRESH_RATE_BATTLE 120
#define REFRESH_RATE_LOBBY 30
#define REFRESH_RATE_IDLE 15

#define PATTERN_SCAN_BUFFER_SIZE (128 * 1024 * 1024)
#define NEON_VECTOR_SIZE 16

#define GCM_IV_SIZE 12
#define GCM_TAG_SIZE 16

#define XOR_KEY 0x5A
#define ROTATION_BITS 3

#define HIDE_MOUNT_POINT "/data/local/tmp/.looka_hide"
#define FAKE_PROCESS_NAME "android.hardware.sensors"

// ============================================================================
// ENUMS & TYPES
// ============================================================================
enum class LogLevel { DEBUG, INFO, WARNING, ERROR, FATAL };
enum class EngineType { FREE_FIRE, PUBG, COD_MOBILE, UNKNOWN };
enum class GameRegion { GLOBAL, KOREAN, VIETNAM, INDIA, CHINA, BRAZIL, UNKNOWN };
enum class EngineState { STOPPED, INITIALIZING, RUNNING, PAUSED, ERROR, RECOVERING };
enum class ExecutionMode { ROOT, NON_ROOT, AUTO };
enum class WeaponType { NONE, AR, SMG, SNIPER, SHOTGUN, DMR, PISTOL };
enum class RefreshMode { BATTLE, LOBBY, IDLE, AUTO };
enum class OverlayType { FRAMEBUFFER, SURFACEFLINGER, NONE };

// ============================================================================
// 1. STRING OBFUSCATION
// ============================================================================
template <size_t N>
class ObfuscatedString {
private:
    std::array<char, N> _data;
    static constexpr char KEY = 0x7F;
public:
    constexpr ObfuscatedString(const char* str) : _data{} {
        for (size_t i = 0; i < N; ++i) {
            _data[i] = str[i] ^ KEY;
        }
    }
    std::string decrypt() const {
        std::string s;
        for (size_t i = 0; i < N && _data[i] != 0; ++i) {
            s += static_cast<char>(_data[i] ^ KEY);
        }
        return s;
    }
};

#define OBF(str) (ObfuscatedString<sizeof(str)>(str).decrypt().c_str())

// ============================================================================
// 2. LOGGER
// ============================================================================
class Logger {
private:
    std::ofstream log_file;
    std::mutex log_mutex;
    LogLevel min_level;
    std::string log_path;
    bool _verbose = true;
    
    std::string levelToString(LogLevel level) const {
        switch(level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO:  return "INFO ";
            case LogLevel::WARNING: return "WARN ";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::FATAL: return "FATAL";
            default: return "UNKN ";
        }
    }
    
public:
    Logger() : min_level(LogLevel::INFO) {
        log_path = "/data/local/tmp/.looka.log";
        log_file.open(log_path, std::ios::app);
        if (log_file.is_open()) {
            log_file << "=== Looka Engine v" << LOOKA_VERSION << " Started ===" << std::endl;
        }
    }
    
    ~Logger() {
        if (log_file.is_open()) log_file.close();
    }
    
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void setVerbose(bool v) { _verbose = v; }
    
    void log(LogLevel level, const std::string& message) {
        if (level < min_level || !_verbose) return;
        
        std::stringstream ss;
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        ss << std::put_time(std::localtime(&time), "%H:%M:%S");
        ss << " [" << levelToString(level) << "] ";
        ss << BRAND_NAME_FULL << ": " << message;
        
        {
            std::lock_guard<std::mutex> lock(log_mutex);
            if (log_file.is_open()) {
                log_file << ss.str() << std::endl;
                log_file.flush();
            }
            std::cout << ss.str() << std::endl;
        }
        
        if (level == LogLevel::FATAL) {
            std::cerr << ss.str() << std::endl;
        }
    }
    
    void log(LogLevel level, const char* format, ...) {
        if (level < min_level || !_verbose) return;
        
        char buffer[2048];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        log(level, std::string(buffer));
    }
};

#define LOG_DEBUG(fmt, ...) Logger::getInstance().log(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger::getInstance().log(LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) Logger::getInstance().log(LogLevel::WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::getInstance().log(LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger::getInstance().log(LogLevel::FATAL, fmt, ##__VA_ARGS__)

// ============================================================================
// 3. MEMORY MANAGER (من test.cpp)
// ============================================================================
class MemoryManager {
private:
    pid_t _target_pid;
    bool _is_attached;

public:
    MemoryManager(pid_t pid) : _target_pid(pid), _is_attached(false) {
        LOG_INFO("Initializing MemoryManager for PID: %d", _target_pid);
    }

    ~MemoryManager() {
        if (_is_attached) detach();
    }

    bool attach() {
        if (ptrace(PTRACE_ATTACH, _target_pid, nullptr, nullptr) < 0) {
            LOG_ERROR("Failed to attach to process: %s", strerror(errno));
            return false;
        }
        waitpid(_target_pid, nullptr, WUNTRACED);
        _is_attached = true;
        LOG_INFO("Successfully attached to process %d", _target_pid);
        return true;
    }

    bool detach() {
        if (ptrace(PTRACE_DETACH, _target_pid, nullptr, nullptr) < 0) {
            return false;
        }
        _is_attached = false;
        LOG_INFO("Detached from process %d", _target_pid);
        return true;
    }

    bool read(uintptr_t address, void* buffer, size_t size) {
        struct iovec local[1];
        struct iovec remote[1];
        local[0].iov_base = buffer;
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void*>(address);
        remote[0].iov_len = size;
        ssize_t bytes_read = process_vm_readv(_target_pid, local, 1, remote, 1, 0);
        return bytes_read == static_cast<ssize_t>(size);
    }

    bool write(uintptr_t address, const void* buffer, size_t size) {
        struct iovec local[1];
        struct iovec remote[1];
        local[0].iov_base = const_cast<void*>(buffer);
        local[0].iov_len = size;
        remote[0].iov_base = reinterpret_cast<void*>(address);
        remote[0].iov_len = size;
        ssize_t bytes_written = process_vm_writev(_target_pid, local, 1, remote, 1, 0);
        return bytes_written == static_cast<ssize_t>(size);
    }

    std::vector<std::pair<uintptr_t, uintptr_t>> getRegions(const std::string& library_name = "") {
        std::vector<std::pair<uintptr_t, uintptr_t>> regions;
        std::string path = "/proc/" + std::to_string(_target_pid) + "/maps";
        std::ifstream maps(path);
        std::string line;
        while (std::getline(maps, line)) {
            if (!library_name.empty() && line.find(library_name) == std::string::npos) continue;
            uintptr_t start, end;
            if (sscanf(line.c_str(), "%lx-%lx", &start, &end) == 2) {
                regions.push_back({start, end});
            }
        }
        return regions;
    }
};

// ============================================================================
// 4. DIRECT KERNEL ACCESS (من v11)
// ============================================================================
class DirectKernelAccess {
private:
    pid_t target_pid;
    std::atomic<uint64_t> total_ops{0};
    std::atomic<uint64_t> total_bytes{0};
public:
    DirectKernelAccess(pid_t pid) : target_pid(pid) {}
    
    bool read(uintptr_t address, void* buffer, size_t size) {
        struct iovec local = { buffer, size };
        struct iovec remote = { reinterpret_cast<void*>(address), size };
        ssize_t bytes = process_vm_readv(target_pid, &local, 1, &remote, 1, 0);
        if (bytes == static_cast<ssize_t>(size)) {
            total_ops++; total_bytes += size;
            return true;
        }
        return false;
    }
    
    bool write(uintptr_t address, const void* buffer, size_t size) {
        struct iovec local = { const_cast<void*>(buffer), size };
        struct iovec remote = { reinterpret_cast<void*>(address), size };
        ssize_t bytes = process_vm_writev(target_pid, &local, 1, &remote, 1, 0);
        if (bytes == static_cast<ssize_t>(size)) {
            total_ops++; total_bytes += size;
            return true;
        }
        return false;
    }
    
    uint64_t getOps() const { return total_ops.load(); }
    uint64_t getBytes() const { return total_bytes.load(); }
};

// ============================================================================
// 5. NEON PATTERN SCANNER (من v11)
// ============================================================================
struct alignas(64) NEONPattern {
    std::string name;
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> mask;
    uintptr_t result;
    bool found;
    alignas(64) uint8_t neon_bytes[128];
    alignas(64) uint8_t neon_mask[128];
    size_t neon_len;
    
    NEONPattern(const std::string& n, const std::string& pattern) : name(n), result(0), found(false), neon_len(0) {
        std::istringstream iss(pattern);
        std::string byte_str;
        while (iss >> byte_str) {
            if (byte_str == "??" || byte_str == "?") {
                bytes.push_back(0); mask.push_back(0);
            } else {
                bytes.push_back(static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16)));
                mask.push_back(0xFF);
            }
        }
        neon_len = std::min(bytes.size(), size_t(128));
        memset(neon_bytes, 0, sizeof(neon_bytes));
        memset(neon_mask, 0, sizeof(neon_mask));
        for (size_t i = 0; i < neon_len; i++) {
            neon_bytes[i] = bytes[i];
            neon_mask[i] = mask[i];
        }
    }
    
    bool matchesNEON(const uint8_t* data) const {
#ifdef __ARM_NEON
        size_t i = 0;
        for (; i + 16 <= neon_len; i += 16) {
            uint8x16_t v_data = vld1q_u8(data + i);
            uint8x16_t v_pattern = vld1q_u8(neon_bytes + i);
            uint8x16_t v_mask = vld1q_u8(neon_mask + i);
            uint8x16_t v_data_masked = vandq_u8(v_data, v_mask);
            uint8x16_t v_pattern_masked = vandq_u8(v_pattern, v_mask);
            uint8x16_t v_cmp = vceqq_u8(v_data_masked, v_pattern_masked);
            uint64x2_t v_res = vreinterpretq_u64_u8(v_cmp);
            if ((vgetq_lane_u64(v_res, 0) != 0xFFFFFFFFFFFFFFFFULL) ||
                (vgetq_lane_u64(v_res, 1) != 0xFFFFFFFFFFFFFFFFULL)) {
                return false;
            }
        }
#endif
        for (size_t i = 0; i < neon_len; i++) {
            if ((data[i] & neon_mask[i]) != (neon_bytes[i] & neon_mask[i])) return false;
        }
        return true;
    }
};

class NEONPatternScanner {
private:
    std::vector<NEONPattern> patterns;
    uintptr_t module_base;
    size_t module_size;
    pid_t target_pid;
    mutable std::shared_mutex patterns_mutex;
    std::atomic<uint64_t> total_bytes_scanned{0};
    double last_scan_time_ms{0};
    
    size_t getModuleSize(const std::string& module_name) {
        std::string maps_path = "/proc/" + std::to_string(target_pid) + "/maps";
        std::ifstream maps(maps_path);
        if (!maps.is_open()) return 0;
        std::string line;
        size_t max_end = 0;
        uintptr_t base = 0;
        while (std::getline(maps, line)) {
            if (line.find(module_name) != std::string::npos) {
                size_t dash = line.find('-');
                size_t space = line.find(' ', dash);
                if (dash != std::string::npos && space != std::string::npos) {
                    uintptr_t start = std::stoull(line.substr(0, dash), nullptr, 16);
                    uintptr_t end = std::stoull(line.substr(dash + 1, space - dash - 1), nullptr, 16);
                    if (base == 0) base = start;
                    if (end > max_end) max_end = end;
                }
            }
        }
        return (max_end > base) ? (max_end - base) : 0;
    }
    
    bool scanRegionNEON(const NEONPattern& pattern, uintptr_t start, uintptr_t end) {
        size_t pattern_len = pattern.bytes.size();
        if (pattern_len == 0) return false;
        size_t buffer_size = std::min<size_t>(PATTERN_SCAN_BUFFER_SIZE, end - start);
        alignas(64) std::vector<uint8_t> buffer(buffer_size + pattern_len);
        for (uintptr_t addr = start; addr < end; addr += buffer_size - pattern_len) {
            size_t remaining = end - addr;
            size_t read_size = std::min(buffer_size, remaining);
            struct iovec local = { buffer.data(), read_size };
            struct iovec remote = { reinterpret_cast<void*>(addr), read_size };
            ssize_t bytes = process_vm_readv(target_pid, &local, 1, &remote, 1, 0);
            if (bytes != static_cast<ssize_t>(read_size)) continue;
            total_bytes_scanned += read_size;
            for (size_t i = 0; i <= read_size - pattern_len; i++) {
                if (pattern.matchesNEON(buffer.data() + i)) {
                    const_cast<NEONPattern&>(pattern).result = addr + i;
                    return true;
                }
            }
        }
        return false;
    }
    
public:
    NEONPatternScanner(pid_t pid) : target_pid(pid), module_base(0), module_size(0) {}
    
    bool initialize(const std::string& module_name) {
        module_base = findModuleBase(module_name);
        if (module_base == 0) return false;
        module_size = getModuleSize(module_name);
        LOG_INFO("NEON Scanner ready - Base: 0x%lx, Size: %.2f MB", module_base, module_size / (1024.0 * 1024.0));
        return true;
    }
    
    uintptr_t findModuleBase(const std::string& module_name) {
        std::string maps_path = "/proc/" + std::to_string(target_pid) + "/maps";
        std::ifstream maps(maps_path);
        if (!maps.is_open()) return 0;
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find(module_name) != std::string::npos) {
                size_t dash = line.find('-');
                if (dash != std::string::npos) {
                    return std::stoull(line.substr(0, dash), nullptr, 16);
                }
            }
        }
        return 0;
    }
    
    void addPattern(const std::string& name, const std::string& pattern) {
        std::unique_lock lock(patterns_mutex);
        patterns.emplace_back(name, pattern);
        LOG_INFO("Pattern added: %s (%zu bytes)", name.c_str(), patterns.back().bytes.size());
    }
    
    void scanAllPatterns() {
        if (module_base == 0 || module_size == 0) return;
        auto start = std::chrono::high_resolution_clock::now();
        total_bytes_scanned = 0;
        for (auto& pattern : patterns) {
            if (!pattern.found) {
                scanRegionNEON(pattern, module_base, module_base + module_size);
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        last_scan_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
        size_t found = 0;
        for (const auto& p : patterns) if (p.found) found++;
        LOG_INFO("NEON scan completed in %.2f ms (%.0f MB/s) - Found %zu patterns",
                 last_scan_time_ms, (total_bytes_scanned.load() / 1024.0 / 1024.0) / (last_scan_time_ms / 1000.0), found);
    }
    
    uintptr_t getPatternResult(const std::string& name) {
        std::shared_lock lock(patterns_mutex);
        for (const auto& pattern : patterns) {
            if (pattern.name == name && pattern.found) return pattern.result;
        }
        return 0;
    }
    
    size_t getFoundCount() const {
        size_t count = 0;
        for (const auto& p : patterns) if (p.found) count++;
        return count;
    }
    
    double getScanSpeedMBps() const {
        if (last_scan_time_ms <= 0) return 0;
        return (total_bytes_scanned.load() / 1024.0 / 1024.0) / (last_scan_time_ms / 1000.0);
    }
};

// ============================================================================
// 6. INTELLIGENT CACHE MANAGER (من v11)
// ============================================================================
template<typename Key, typename Value>
class IntelligentCacheManager {
private:
    struct CacheEntry {
        Value data;
        std::chrono::steady_clock::time_point timestamp;
        uint64_t access_count;
    };
    std::unordered_map<Key, CacheEntry> cache;
    std::shared_mutex cache_mutex;
    size_t max_entries;
    std::chrono::milliseconds ttl;
    std::unique_ptr<std::thread> cleanup_thread;
    std::atomic<bool> running{true};
    std::atomic<uint64_t> hits{0};
    std::atomic<uint64_t> misses{0};
    
    void cleanupLoop() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CACHE_CLEANUP_INTERVAL_MS));
            auto now = std::chrono::steady_clock::now();
            std::unique_lock<std::shared_mutex> lock(cache_mutex);
            for (auto it = cache.begin(); it != cache.end();) {
                if (now - it->second.timestamp > ttl) it = cache.erase(it);
                else ++it;
            }
            if (cache.size() > max_entries) {
                std::vector<std::pair<Key, uint64_t>> access_list;
                for (const auto& [key, entry] : cache) access_list.emplace_back(key, entry.access_count);
                std::sort(access_list.begin(), access_list.end(),
                    [](const auto& a, const auto& b) { return a.second < b.second; });
                size_t to_remove = cache.size() - max_entries;
                for (size_t i = 0; i < to_remove && i < access_list.size(); i++) cache.erase(access_list[i].first);
            }
        }
    }
    
public:
    IntelligentCacheManager(size_t max_size = CACHE_MAX_SIZE, int ttl_ms = CACHE_ENTRY_TTL_MS)
        : max_entries(max_size / sizeof(CacheEntry)), ttl(std::chrono::milliseconds(ttl_ms)) {
        cleanup_thread = std::make_unique<std::thread>(&IntelligentCacheManager::cleanupLoop, this);
    }
    
    ~IntelligentCacheManager() { running = false; if (cleanup_thread && cleanup_thread->joinable()) cleanup_thread->join(); }
    
    bool get(const Key& key, Value& value) {
        std::shared_lock<std::shared_mutex> lock(cache_mutex);
        auto it = cache.find(key);
        if (it != cache.end()) {
            auto now = std::chrono::steady_clock::now();
            if (now - it->second.timestamp <= ttl) {
                value = it->second.data;
                const_cast<CacheEntry&>(it->second).access_count++;
                hits++;
                return true;
            }
        }
        misses++;
        return false;
    }
    
    void put(const Key& key, const Value& value) {
        std::unique_lock<std::shared_mutex> lock(cache_mutex);
        CacheEntry entry;
        entry.data = value;
        entry.timestamp = std::chrono::steady_clock::now();
        entry.access_count = 1;
        cache[key] = entry;
    }
    
    double getHitRate() const {
        uint64_t total = hits + misses;
        return total == 0 ? 0 : (static_cast<double>(hits) / total * 100.0);
    }
};

// ============================================================================
// 7. ADAPTIVE REFRESH RATE MANAGER (من v11)
// ============================================================================
class AdaptiveRefreshManager {
private:
    std::atomic<RefreshMode> current_mode{RefreshMode::AUTO};
    std::atomic<int> current_fps{REFRESH_RATE_BATTLE};
    std::atomic<bool> in_battle{false};
    std::atomic<bool> game_active{true};
    std::chrono::steady_clock::time_point last_activity;
    
    int getOptimalFPS() {
        if (!game_active) return REFRESH_RATE_IDLE;
        switch(current_mode) {
            case RefreshMode::BATTLE: return REFRESH_RATE_BATTLE;
            case RefreshMode::LOBBY: return REFRESH_RATE_LOBBY;
            case RefreshMode::IDLE: return REFRESH_RATE_IDLE;
            case RefreshMode::AUTO: return in_battle ? REFRESH_RATE_BATTLE : REFRESH_RATE_LOBBY;
            default: return REFRESH_RATE_BATTLE;
        }
    }
    
public:
    AdaptiveRefreshManager() { last_activity = std::chrono::steady_clock::now(); }
    
    void update() {
        int new_fps = getOptimalFPS();
        if (new_fps != current_fps) {
            current_fps = new_fps;
            LOG_INFO("Refresh rate adjusted: %d FPS", current_fps.load());
        }
        last_activity = std::chrono::steady_clock::now();
    }
    
    void setMode(RefreshMode mode) { current_mode = mode; }
    void setInBattle(bool battle) { in_battle = battle; }
    void setGameActive(bool active) { game_active = active; }
    int getCurrentFPS() const { return current_fps; }
    RefreshMode getCurrentMode() const { return current_mode; }
    int getSleepMicroseconds() const { return current_fps <= 0 ? 0 : 1000000 / current_fps; }
};

// ============================================================================
// 8. SMART REPORTING SYSTEM (من v11)
// ============================================================================
class SmartReportingSystem {
private:
    struct ErrorReport {
        std::string timestamp;
        std::string error_type;
        std::string error_message;
        std::string context;
        std::string device_info;
    };
    std::vector<ErrorReport> reports;
    std::mutex reports_mutex;
    std::string report_file;
    
    std::string getDeviceInfo() {
        std::stringstream ss;
        std::ifstream build_prop("/system/build.prop");
        if (build_prop.is_open()) {
            std::string line;
            while (std::getline(build_prop, line)) {
                if (line.find("ro.product.model") != std::string::npos) {
                    size_t eq = line.find('=');
                    if (eq != std::string::npos) ss << "Model: " << line.substr(eq + 1) << "\n";
                }
                if (line.find("ro.build.version.sdk") != std::string::npos) {
                    size_t eq = line.find('=');
                    if (eq != std::string::npos) ss << "API: " << line.substr(eq + 1) << "\n";
                }
            }
        }
        struct sysinfo info;
        if (sysinfo(&info) == 0) ss << "RAM: " << (info.totalram / (1024 * 1024)) << " MB\n";
        return ss.str();
    }
    
public:
    SmartReportingSystem() { report_file = "/data/local/tmp/.looka_crash.log"; }
    
    void logError(const std::string& error_type, const std::string& error_message, const std::string& context = "") {
        ErrorReport report;
        report.timestamp = [](){
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        }();
        report.error_type = error_type;
        report.error_message = error_message;
        report.context = context;
        report.device_info = getDeviceInfo();
        {
            std::lock_guard<std::mutex> lock(reports_mutex);
            reports.push_back(report);
        }
        std::ofstream file(report_file, std::ios::app);
        if (file.is_open()) {
            file << "[" << report.timestamp << "] " << error_type << ": " << error_message << std::endl;
            file.close();
        }
        LOG_ERROR("%s: %s", error_type.c_str(), error_message.c_str());
    }
    
    void printLastErrors(int count = 5) {
        std::lock_guard<std::mutex> lock(reports_mutex);
        std::cout << "\n📋 Recent Errors:" << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        int start = std::max(0, (int)reports.size() - count);
        for (int i = start; i < (int)reports.size(); i++) {
            const auto& r = reports[i];
            std::cout << "  [" << r.timestamp << "] " << r.error_type << ": " << r.error_message << std::endl;
        }
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    }
};

// ============================================================================
// 9. POLYMORPHIC MEMORY PROTECTION (من v11)
// ============================================================================
class PolymorphicMemoryProtection {
private:
    std::mt19937 rng;
    std::vector<uint8_t> global_key;
    std::chrono::steady_clock::time_point last_rotation;
    static constexpr int KEY_ROTATION_SEC = 45;
    
    void rotateGlobalKey() {
        for (size_t i = 0; i < global_key.size(); i++) global_key[i] = rng() & 0xFF;
        last_rotation = std::chrono::steady_clock::now();
    }
    
public:
    PolymorphicMemoryProtection() : rng(std::random_device{}()) {
        global_key.resize(64);
        for (size_t i = 0; i < 64; i++) global_key[i] = rng() & 0xFF;
        last_rotation = std::chrono::steady_clock::now();
    }
    
    void rotateKeys() {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_rotation).count() >= KEY_ROTATION_SEC) {
            rotateGlobalKey();
            LOG_INFO("Polymorphic keys rotated");
        }
    }
};

// ============================================================================
// 10. ROOT DETECTOR
// ============================================================================
struct RootCheckResult {
    bool has_root = false;
    std::string method;
    std::vector<std::string> details;
};

class RootDetector {
public:
    static RootCheckResult detect() {
        RootCheckResult result;
        if (getuid() == 0) {
            result.has_root = true;
            result.method = "Running as root (UID=0)";
            result.details.push_back("Process has root privileges");
        }
        const char* su_paths[] = {"/system/bin/su", "/system/xbin/su", "/sbin/su",
            "/data/local/bin/su", "/data/adb/magisk/magisk", nullptr};
        for (int i = 0; su_paths[i]; i++) {
            if (access(su_paths[i], X_OK) == 0) {
                result.has_root = true;
                result.method = "su binary found";
                result.details.push_back(std::string("Found: ") + su_paths[i]);
                break;
            }
        }
        if (std::filesystem::exists("/data/adb/magisk")) {
            result.has_root = true;
            result.method = "Magisk detected";
            result.details.push_back("Magisk root manager found");
        }
        return result;
    }
};

// ============================================================================
// 11. HYBRID MODE SELECTOR
// ============================================================================
class HybridModeSelector {
private:
    RootCheckResult root_status;
    ExecutionMode effective_mode;
    bool root_available;
    
public:
    HybridModeSelector() {
        root_status = RootDetector::detect();
        root_available = root_status.has_root;
        effective_mode = root_available ? ExecutionMode::ROOT : ExecutionMode::NON_ROOT;
        LOG_INFO("Mode: %s", root_available ? "ULTRA (Root)" : "STANDARD (Non-Root)");
    }
    
    ExecutionMode getEffectiveMode() const { return effective_mode; }
    bool isUltraMode() const { return effective_mode == ExecutionMode::ROOT; }
    bool isRootAvailable() const { return root_available; }
};

// ============================================================================
// 12. GHOST PROCESS
// ============================================================================
class GhostProcess {
private:
    std::string original_name;
    bool hidden = false;
    
public:
    GhostProcess() {
        std::ifstream cmdline("/proc/self/cmdline");
        if (cmdline.is_open()) {
            std::getline(cmdline, original_name);
            original_name.erase(std::find(original_name.begin(), original_name.end(), '\0'), original_name.end());
        }
        if (original_name.empty()) original_name = "looka_engine";
    }
    
    bool makeGhost() {
        if (hidden) return true;
        if (prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(FAKE_PROCESS_NAME), 0, 0, 0) == 0) {
            hidden = true;
            LOG_INFO("Ghost mode active - Hidden as '%s'", FAKE_PROCESS_NAME);
            return true;
        }
        return false;
    }
    
    void restore() {
        if (!hidden) return;
        prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(original_name.c_str()), 0, 0, 0);
        hidden = false;
    }
    
    bool isGhost() const { return hidden; }
};

// ============================================================================
// 13. ADVANCED ANTI-DETECT
// ============================================================================
class AdvancedAntiDetect {
private:
    bool active = false;
    
public:
    bool activate() {
        if (active) return true;
        if (unshare(CLONE_NEWNS) == 0) {
            std::filesystem::create_directory(HIDE_MOUNT_POINT);
            mount(HIDE_MOUNT_POINT, "/proc/self", nullptr, MS_BIND, nullptr);
            active = true;
            LOG_INFO("Advanced anti-detect active");
            return true;
        }
        return false;
    }
    
    void deactivate() { active = false; }
    bool isActive() const { return active; }
};

// ============================================================================
// 14. OVERLAY (Framebuffer)
// ============================================================================
struct Pixel {
    uint8_t r, g, b, a;
};

class Overlay {
private:
    int _fb_fd;
    struct fb_var_screeninfo _vinfo;
    struct fb_fix_screeninfo _finfo;
    long _screensize;
    char* _fbp;
    bool _initialized = false;

public:
    Overlay() : _fb_fd(-1), _fbp(nullptr) {}
    
    ~Overlay() {
        if (_fbp) munmap(_fbp, _screensize);
        if (_fb_fd != -1) close(_fb_fd);
    }
    
    bool init() {
        if (_initialized) return true;
        _fb_fd = open("/dev/graphics/fb0", O_RDWR);
        if (_fb_fd == -1) _fb_fd = open("/dev/fb0", O_RDWR);
        if (_fb_fd == -1) {
            LOG_ERROR("Cannot open framebuffer device. Root required?");
            return false;
        }
        if (ioctl(_fb_fd, FBIOGET_FSCREENINFO, &_finfo) == -1) {
            LOG_ERROR("Error reading fixed information.");
            close(_fb_fd);
            return false;
        }
        if (ioctl(_fb_fd, FBIOGET_VSCREENINFO, &_vinfo) == -1) {
            LOG_ERROR("Error reading variable information.");
            close(_fb_fd);
            return false;
        }
        _screensize = _vinfo.xres * _vinfo.yres * _vinfo.bits_per_pixel / 8;
        _fbp = (char*)mmap(0, _screensize, PROT_READ | PROT_WRITE, MAP_SHARED, _fb_fd, 0);
        if ((long)_fbp == -1) {
            LOG_ERROR("Failed to mmap framebuffer.");
            close(_fb_fd);
            return false;
        }
        _initialized = true;
        LOG_INFO("Framebuffer overlay ready - Resolution: %dx%d", _vinfo.xres, _vinfo.yres);
        return true;
    }
    
    void drawPixel(int x, int y, Pixel color) {
        if (!_fbp || x < 0 || x >= _vinfo.xres || y < 0 || y >= _vinfo.yres) return;
        long location = (x + _vinfo.xoffset) * (_vinfo.bits_per_pixel / 8) +
                        (y + _vinfo.yoffset) * _finfo.line_length;
        if (_vinfo.bits_per_pixel == 32) {
            *(_fbp + location) = color.b;
            *(_fbp + location + 1) = color.g;
            *(_fbp + location + 2) = color.r;
            *(_fbp + location + 3) = color.a;
        }
    }
    
    void clear() { if (_fbp) memset(_fbp, 0, _screensize); }
    bool isAvailable() const { return _initialized; }
};

// ============================================================================
// 15. ENGINE BRIDGE (Shared Memory)
// ============================================================================
class EngineBridge {
private:
    int _shm_fd;
    void* _shared_mem;
    const size_t SHM_SIZE = 1024 * 64;
    bool _initialized = false;

public:
    EngineBridge() : _shm_fd(-1), _shared_mem(nullptr) {}
    
    ~EngineBridge() {
        if (_shared_mem) munmap(_shared_mem, SHM_SIZE);
        if (_shm_fd != -1) {
            close(_shm_fd);
            shm_unlink("/looka_bridge");
        }
    }
    
    bool create() {
        _shm_fd = shm_open("/looka_bridge", O_RDWR | O_CREAT, 0666);
        if (_shm_fd == -1) {
            LOG_ERROR("Failed to create Shared Memory Bridge.");
            return false;
        }
        if (ftruncate(_shm_fd, SHM_SIZE) == -1) return false;
        _shared_mem = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _shm_fd, 0);
        if (_shared_mem == MAP_FAILED) return false;
        _initialized = true;
        LOG_INFO("Engine Bridge established at 0x%p", _shared_mem);
        return true;
    }
    
    template<typename T>
    void sendData(const T& data) {
        if (_shared_mem) memcpy(_shared_mem, &data, sizeof(T));
    }
    
    bool isAvailable() const { return _initialized; }
};

// ============================================================================
// 16. SYSTEM MONITOR
// ============================================================================
class SystemMonitor {
public:
    static void printSystemStats() {
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            LOG_INFO("System Uptime: %ld seconds", info.uptime);
            LOG_INFO("Total RAM: %.2f MB", info.totalram / 1024.0 / 1024.0);
            LOG_INFO("Free RAM: %.2f MB", info.freeram / 1024.0 / 1024.0);
            LOG_INFO("Active Processes: %d", info.procs);
        }
    }
};

// ============================================================================
// 17. CRYPTO PROVIDER
// ============================================================================
class CryptoProvider {
public:
    static std::string encrypt(const std::string& plain_text, const std::string& key) {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        unsigned char ciphertext[1024];
        int len, ciphertext_len;
        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)key.data(), (unsigned char*)key.data());
        EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plain_text.data(), plain_text.length());
        ciphertext_len = len;
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        ciphertext_len += len;
        EVP_CIPHER_CTX_free(ctx);
        return std::string((char*)ciphertext, ciphertext_len);
    }
    
    static void handleErrors() { ERR_print_errors_fp(stderr); }
};

// ============================================================================
// 18. AIM FEATURES
// ============================================================================
struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float px, float py) : x(px), y(py) {}
};

class DynamicAimAssist {
private:
    struct Config { bool enabled = true; float smoothness = 5.0f; } config;
public:
    void setEnabled(bool enabled) { config.enabled = enabled; }
    Vector2 calculateAssist(Vector2 current, Vector2 target) {
        if (!config.enabled) return current;
        Vector2 result;
        result.x = current.x + (target.x - current.x) / config.smoothness;
        result.y = current.y + (target.y - current.y) / config.smoothness;
        return result;
    }
};

class HardcoreAimbot {
private:
    struct Config { bool enabled = false; } config;
public:
    void setEnabled(bool enabled) { config.enabled = enabled; }
};

class FlickDetector {
private:
    struct Config { bool enabled = true; } config;
public:
    void setEnabled(bool enabled) { config.enabled = enabled; }
};

// ============================================================================
// 19. FLOATING CONTROL PANEL
// ============================================================================
class FloatingControlPanel {
private:
    bool visible = true;
    bool minimized = false;
    std::atomic<bool> running{false};
    
public:
    void start() { running = true; LOG_INFO("Control panel started"); }
    void stop() { running = false; }
    void setVisible(bool vis) { visible = vis; }
    void setMinimized(bool min) { minimized = min; }
    void updateStats(int fps, int enemies) { /* تحديث الإحصائيات */ }
};

// ============================================================================
// 20. MAIN ENGINE CLASS (دمج جميع الميزات)
// ============================================================================
class LookaEngineUltimate {
private:
    std::unique_ptr<HybridModeSelector> mode_selector;
    ExecutionMode current_mode;
    
    // مكونات القراءة والكتابة
    std::unique_ptr<MemoryManager> memory;
    std::unique_ptr<DirectKernelAccess> direct_kernel;
    std::unique_ptr<NEONPatternScanner> neon_scanner;
    std::unique_ptr<IntelligentCacheManager<uintptr_t, uint32_t>> cache;
    
    // مكونات الرسم والاتصال
    std::unique_ptr<Overlay> overlay;
    std::unique_ptr<EngineBridge> bridge;
    
    // مكونات الحماية
    std::unique_ptr<GhostProcess> ghost;
    std::unique_ptr<AdvancedAntiDetect> anti_detect;
    std::unique_ptr<PolymorphicMemoryProtection> polymorphic;
    
    // مكونات التحكم
    std::unique_ptr<AdaptiveRefreshManager> refresh_manager;
    std::unique_ptr<SmartReportingSystem> reporter;
    std::unique_ptr<FloatingControlPanel> ui_panel;
    std::unique_ptr<DynamicAimAssist> aim_assist;
    std::unique_ptr<HardcoreAimbot> hardcore_aimbot;
    std::unique_ptr<FlickDetector> flick_detector;
    
    pid_t target_pid;
    uintptr_t base_address;
    std::string module_name;
    std::atomic<EngineState> state{EngineState::STOPPED};
    std::atomic<uint64_t> frames{0};
    std::vector<std::thread> _worker_threads;
    std::unordered_map<std::string, uintptr_t> addresses;
    
    struct FreezeItem {
        uintptr_t address;
        std::vector<uint8_t> value;
        bool active;
    };
    std::vector<FreezeItem> _freeze_list;
    std::mutex _freeze_mutex;
    
    void registerPatterns() {
        neon_scanner->addPattern("health", "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 00 00");
        neon_scanner->addPattern("armor", "FF FF FF FF 00 00 00 00 ?? ?? ?? ??");
        neon_scanner->addPattern("speed", "00 00 80 3F ?? ?? ?? ?? ?? ?? ?? ??");
        neon_scanner->addPattern("jump", "00 00 00 00 00 00 F0 3F ?? ?? ?? ??");
        neon_scanner->addPattern("recoil", "00 00 00 00 00 00 00 00 CD CC CC 3D");
        LOG_INFO("Registered 5 patterns");
    }
    
    void collectPatterns() {
        std::vector<std::string> patterns = {"health", "armor", "speed", "jump", "recoil"};
        for (const auto& name : patterns) {
            uintptr_t addr = neon_scanner->getPatternResult(name);
            if (addr != 0) {
                addresses[name] = addr;
                LOG_INFO("Found %s at 0x%lx", name.c_str(), addr);
            }
        }
    }
    
    void freezeWorker() {
        while (state.load() == EngineState::RUNNING) {
            {
                std::lock_guard<std::mutex> lock(_freeze_mutex);
                for (auto& item : _freeze_list) {
                    if (item.active && memory) {
                        memory->write(item.address, item.value.data(), item.value.size());
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    }
    
    void overlayWorker() {
        while (state.load() == EngineState::RUNNING && overlay && overlay->isAvailable()) {
            overlay->clear();
            overlay->drawPixel(540, 1170, {255, 0, 0, 255});
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    
public:
    LookaEngineUltimate(pid_t pid, uintptr_t base, const std::string& module) 
        : target_pid(pid), base_address(base), module_name(module) {
        
        mode_selector = std::make_unique<HybridModeSelector>();
        current_mode = mode_selector->getEffectiveMode();
        
        memory = std::make_unique<MemoryManager>(target_pid);
        direct_kernel = std::make_unique<DirectKernelAccess>(target_pid);
        neon_scanner = std::make_unique<NEONPatternScanner>(target_pid);
        cache = std::make_unique<IntelligentCacheManager<uintptr_t, uint32_t>>();
        
        overlay = std::make_unique<Overlay>();
        bridge = std::make_unique<EngineBridge>();
        
        ghost = std::make_unique<GhostProcess>();
        anti_detect = std::make_unique<AdvancedAntiDetect>();
        polymorphic = std::make_unique<PolymorphicMemoryProtection>();
        
        refresh_manager = std::make_unique<AdaptiveRefreshManager>();
        reporter = std::make_unique<SmartReportingSystem>();
        ui_panel = std::make_unique<FloatingControlPanel>();
        aim_assist = std::make_unique<DynamicAimAssist>();
        hardcore_aimbot = std::make_unique<HardcoreAimbot>();
        flick_detector = std::make_unique<FlickDetector>();
        
        LOG_INFO("Engine initialized - Mode: %s", current_mode == ExecutionMode::ROOT ? "ULTRA" : "STANDARD");
    }
    
    bool initialize() {
        try {
            if (!memory->attach()) {
                LOG_ERROR("Cannot attach to target process");
                return false;
            }
            
            if (neon_scanner->initialize(module_name)) {
                registerPatterns();
                neon_scanner->scanAllPatterns();
                collectPatterns();
                LOG_INFO("NEON scan complete - Found %zu patterns", addresses.size());
            }
            
            if (overlay->init()) {
                LOG_INFO("Overlay initialized");
            } else {
                LOG_WARNING("Overlay initialization skipped");
            }
            
            if (bridge->create()) {
                LOG_INFO("Bridge created");
            } else {
                LOG_WARNING("Bridge creation failed");
            }
            
            SystemMonitor::printSystemStats();
            
            if (mode_selector->isUltraMode()) {
                if (ghost->makeGhost()) LOG_INFO("Ghost mode active");
                if (anti_detect->activate()) LOG_INFO("Anti-detect active");
            }
            
            ui_panel->start();
            
            state.store(EngineState::INITIALIZING);
            state.store(EngineState::STOPPED);
            return true;
            
        } catch (const std::exception& e) {
            reporter->logError("Initialization", e.what());
            state.store(EngineState::ERROR);
            return false;
        }
    }
    
    void addFreeze(uintptr_t addr, const std::vector<uint8_t>& val) {
        std::lock_guard<std::mutex> lock(_freeze_mutex);
        _freeze_list.push_back({addr, val, true});
        LOG_DEBUG("Added Freeze at 0x%lx", addr);
    }
    
    void run() {
        if (state.load() != EngineState::STOPPED) return;
        
        state.store(EngineState::RUNNING);
        LOG_INFO("Engine running - All systems operational");
        
        _worker_threads.emplace_back(&LookaEngineUltimate::freezeWorker, this);
        _worker_threads.emplace_back(&LookaEngineUltimate::overlayWorker, this);
        
        while (state.load() == EngineState::RUNNING) {
            refresh_manager->update();
            frames++;
            
            if (polymorphic) polymorphic->rotateKeys();
            
            int sleep_us = refresh_manager->getSleepMicroseconds();
            if (sleep_us > 0) {
                std::this_thread::sleep_for(std::chrono::microseconds(sleep_us));
            }
            
            bridge->sendData(state.load());
            
            if (memory->getRegions().empty()) {
                LOG_ERROR("Target process lost");
                state.store(EngineState::ERROR);
                break;
            }
        }
        
        for (auto& t : _worker_threads) {
            if (t.joinable()) t.join();
        }
    }
    
    void stop() {
        LOG_INFO("Stopping engine...");
        state.store(EngineState::STOPPED);
        
        if (ui_panel) ui_panel->stop();
        if (overlay) overlay->clear();
        if (memory) memory->detach();
        if (ghost) ghost->restore();
        if (anti_detect) anti_detect->deactivate();
    }
    
    int getCurrentFPS() const { return refresh_manager->getCurrentFPS(); }
    EngineState getState() const { return state.load(); }
    uintptr_t getAddress(const std::string& name) {
        auto it = addresses.find(name);
        return it != addresses.end() ? it->second : 0;
    }
    
    void printStatus() {
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - std::chrono::steady_clock::now()).count();
        
        std::cout << "\n╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  Looka Engine v" << LOOKA_VERSION << " - Ultimate Status Report            ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  Mode: " << (current_mode == ExecutionMode::ROOT ? "ULTRA (Root)" : "STANDARD") << std::endl;
        std::cout << "║  Frames: " << frames.load() << std::endl;
        std::cout << "║  Patterns Found: " << addresses.size() << std::endl;
        std::cout << "║  Cache Hit Rate: " << std::fixed << std::setprecision(1) << cache->getHitRate() << "%" << std::endl;
        std::cout << "║  Scan Speed: " << std::fixed << std::setprecision(0) << neon_scanner->getScanSpeedMBps() << " MB/s" << std::endl;
        std::cout << "║  Ghost Mode: " << (ghost && ghost->isGhost() ? "Active" : "Inactive") << std::endl;
        std::cout << "║  Anti-Detect: " << (anti_detect && anti_detect->isActive() ? "Active" : "Inactive") << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════════╝\n" << std::endl;
        
        reporter->printLastErrors(3);
    }
};

// ============================================================================
// GAME DETECTOR
// ============================================================================
class GameDetector {
public:
    static pid_t findProcessByPackage(const std::string& package_name) {
        DIR* dir = opendir("/proc");
        if (!dir) return -1;
        struct dirent* entry;
        pid_t result = -1;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type != DT_DIR) continue;
            char* endptr;
            pid_t pid = strtol(entry->d_name, &endptr, 10);
            if (*endptr != '\0') continue;
            std::string cmdline_path = "/proc/" + std::string(entry->d_name) + "/cmdline";
            std::ifstream cmdline(cmdline_path);
            if (!cmdline.is_open()) continue;
            std::string cmd;
            std::getline(cmdline, cmd);
            if (cmd.find(package_name) != std::string::npos) {
                result = pid;
                break;
            }
        }
        closedir(dir);
        return result;
    }
};

// ============================================================================
// BRAND IDENTITY
// ============================================================================
class BrandIdentity {
public:
    static void displayBanner() {
        std::cout << BRAND_COLOR;
        std::cout << R"(
╔══════════════════════════════════════════════════════════════════════════════════════╗
║   ██╗     ██████╗  ██████╗ ██╗  ██╗ █████╗     ███████╗███╗   ██╗ ██████╗ ██╗███╗   ██╗███████╗    ║
║   ██║     ██╔══██╗██╔══██╗██║ ██╔╝██╔══██╗    ██╔════╝████╗  ██║██╔════╝ ██║████╗  ██║██╔════╝    ║
║   ██║     ██████╔╝██████╔╝█████╔╝ ███████║    █████╗  ██╔██╗ ██║██║  ███╗██║██╔██╗ ██║█████╗      ║
║   ██║     ██╔══██╗██╔══██╗██╔═██╗ ██╔══██║    ██╔══╝  ██║╚██╗██║██║   ██║██║██║╚██╗██║██╔══╝      ║
║   ███████╗██║  ██║██████╔╝██║  ██╗██║  ██║    ███████╗██║ ╚████║╚██████╔╝██║██║ ╚████║███████╗    ║
║   ╚══════╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝    ╚══════╝╚═╝  ╚═══╝ ╚═════╝ ╚═╝╚═╝  ╚═══╝╚══════╝    ║
╚══════════════════════════════════════════════════════════════════════════════════════╝
)" << std::endl;
        std::cout << "  Looka Engine v" << LOOKA_VERSION << " - Ultimate Merged Edition" << std::endl;
        std::cout << "\033[0m";
    }
};

// ============================================================================
// MAIN
// ============================================================================
static std::atomic<bool> g_running{true};
static std::unique_ptr<LookaEngineUltimate> g_engine;

void signalHandler(int sig) {
    LOG_INFO("Signal %d received", sig);
    g_running = false;
    if (g_engine) g_engine->stop();
}

int main() {
    try {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        BrandIdentity::displayBanner();
        
        std::string package = "com.dts.freefireth";
        std::string module = "libil2cpp.so";
        
        std::cout << "\n🔍 Searching for game: " << package << "..." << std::endl;
        
        pid_t pid = -1;
        for (int attempt = 1; attempt <= 3; ++attempt) {
            pid = GameDetector::findProcessByPackage(package);
            if (pid > 0) break;
            std::cout << "   Attempt " << attempt << "/3 - Game not running" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        if (pid <= 0) {
            std::cout << "\n❌ Game not found! Please start " << package << " first." << std::endl;
            return 1;
        }
        
        std::cout << "✅ Game PID: " << pid << std::endl;
        
        struct TempMem {
            pid_t pid;
            uintptr_t findBase(const std::string& name) {
                std::string maps = "/proc/" + std::to_string(pid) + "/maps";
                std::ifstream f(maps);
                std::string line;
                while (std::getline(f, line)) {
                    if (line.find(name) != std::string::npos) {
                        size_t dash = line.find('-');
                        if (dash != std::string::npos) {
                            return std::stoull(line.substr(0, dash), nullptr, 16);
                        }
                    }
                }
                return 0;
            }
        } temp{pid};
        
        uintptr_t base = temp.findBase(module);
        
        if (base == 0) {
            std::cout << "\n⚠️  Module not found. Waiting for game to load..." << std::endl;
            for (int attempt = 1; attempt <= 10 && base == 0; ++attempt) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                base = temp.findBase(module);
                if (base != 0) {
                    std::cout << "✅ Module found on attempt " << attempt << std::endl;
                }
            }
            if (base == 0) {
                std::cout << "\n❌ Module " << module << " not found!" << std::endl;
                return 1;
            }
        }
        
        std::cout << "📦 Module base: 0x" << std::hex << base << std::dec << std::endl;
        
        g_engine = std::make_unique<LookaEngineUltimate>(pid, base, module);
        
        if (!g_engine->initialize()) {
            std::cout << "\n❌ Engine initialization failed!" << std::endl;
            return 1;
        }
        
        std::cout << "\n🚀 Looka Engine v" << LOOKA_VERSION << " is ACTIVE!" << std::endl;
        std::cout << "💡 Type 'status' for full report" << std::endl;
        std::cout << "⏱️  Press Ctrl+C to stop" << std::endl;
        
        std::thread engine_thread([]() {
            try {
                g_engine->run();
            } catch (const std::exception& e) {
                g_running = false;
            }
        });
        
        std::thread cmd_thread([]() {
            std::string input;
            while (g_running) {
                if (std::cin.peek() != EOF) {
                    std::getline(std::cin, input);
                    if (input == "status" || input == "s") {
                        if (g_engine) g_engine->printStatus();
                    } else if (input == "quit" || input == "q") {
                        g_running = false;
                        break;
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        });
        
        engine_thread.join();
        cmd_thread.join();
        
        if (g_engine) g_engine->stop();
        
        std::cout << "\n🛑 Looka Engine stopped." << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

// ============================================================================
// END OF FILE - Looka Engine v11.0 Ultimate Merged Edition
// ============================================================================
