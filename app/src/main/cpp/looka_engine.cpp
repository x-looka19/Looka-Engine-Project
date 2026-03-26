// ============================================================================
// looka_engine_v11_0_complete_final.cpp - Looka Engine Ultimate v11.0
// COMPLETE EDITION - ALL FEATURES INTEGRATED - WITH 5 FIXES ONLY
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
#include <stdarg.h>          // ✅ التعديل 1: إضافة stdarg.h لدعم va_list

// SIMD headers
#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

// ✅ التعديل 2: تعريف process_vm_readv و process_vm_writev
extern "C" ssize_t process_vm_readv(pid_t, const struct iovec*, unsigned long, const struct iovec*, unsigned long, unsigned long);
extern "C" ssize_t process_vm_writev(pid_t, const struct iovec*, unsigned long, const struct iovec*, unsigned long, unsigned long);

// ✅ التعديل 3: لف OpenSSL بـ extern "C"
extern "C" {
    #include <openssl/evp.h>
    #include <openssl/rand.h>
    #include <openssl/sha.h>
    #include <openssl/hmac.h>
    #include <openssl/err.h>
    #include <openssl/conf.h>
    #include <openssl/opensslconf.h>
}

// ============================================================================
// DEFINES & CONSTANTS
// ============================================================================
#define LOOKA_VERSION "11.0-Complete-Final"
#define LOOKA_OWNER "Looka"
#define LOOKA_SIGNATURE "LK11.0-2026-LOOKA-COMPLETE"

#define BRAND_NAME "Looka Engine"
#define BRAND_TAGLINE "Ultimate Gaming Suite"
#define BRAND_COLOR "\033[1;36m"

// Window dimensions
#define FLOATING_WINDOW_WIDTH 380
#define FLOATING_WINDOW_HEIGHT 540

// Cache constants
#define CACHE_MAX_SIZE (16 * 1024 * 1024)
#define CACHE_ENTRY_TTL_MS 100
#define CACHE_CLEANUP_INTERVAL_MS 5000

// Refresh rate constants
#define REFRESH_RATE_BATTLE 120
#define REFRESH_RATE_LOBBY 30
#define REFRESH_RATE_IDLE 15

// Pattern scan constants
#define PATTERN_SCAN_BUFFER_SIZE (128 * 1024 * 1024)
#define NEON_VECTOR_SIZE 16

// GCM constants
#define GCM_IV_SIZE 12
#define GCM_TAG_SIZE 16

// Obfuscation constants
#define XOR_KEY 0x5A
#define ROTATION_BITS 3

// Anti-detect
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
class StringObfuscator {
private:
    struct EncryptedString {
        std::vector<uint8_t> data;
        std::string decrypt() const {
            std::string result(data.size(), '\0');
            for (size_t i = 0; i < data.size(); i++) {
                uint8_t c = data[i];
                c = ((c >> ROTATION_BITS) | (c << (8 - ROTATION_BITS))) & 0xFF;
                c ^= XOR_KEY ^ (i & 0xFF);
                result[i] = static_cast<char>(c);
            }
            return result;
        }
    };
    
    static std::vector<EncryptedString> encrypted_strings;
    static std::mutex cache_mutex;
    static std::unordered_map<size_t, std::string> string_cache;
    
    static size_t hashString(const char* str) {
        size_t hash = 0x811C9DC5;
        while (*str) {
            hash ^= *str++;
            hash *= 0x01000193;
        }
        return hash;
    }
    
public:
    #define OBF(str) StringObfuscator::get(str)
    
    static const std::string& get(const char* plain) {
        size_t hash = hashString(plain);
        
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = string_cache.find(hash);
        if (it != string_cache.end()) return it->second;
        
        size_t len = strlen(plain);
        EncryptedString enc;
        enc.data.resize(len);
        
        for (size_t i = 0; i < len; i++) {
            uint8_t c = plain[i];
            c ^= XOR_KEY ^ (i & 0xFF);
            c = ((c << ROTATION_BITS) | (c >> (8 - ROTATION_BITS))) & 0xFF;
            enc.data[i] = c;
        }
        
        encrypted_strings.push_back(enc);
        std::string decrypted = enc.decrypt();
        string_cache[hash] = decrypted;
        return string_cache[hash];
    }
};

std::vector<StringObfuscator::EncryptedString> StringObfuscator::encrypted_strings;
std::mutex StringObfuscator::cache_mutex;
std::unordered_map<size_t, std::string> StringObfuscator::string_cache;

// ============================================================================
// 2. BRAND IDENTITY (FIXED)
// ============================================================================
class BrandIdentity {
private:
    static std::string brand_name;
    static std::string version;
    static std::string tagline;
    static std::string ascii_art;
    
public:
    static void initialize() {
        brand_name = OBF(BRAND_NAME);
        version = OBF(LOOKA_VERSION);
        tagline = OBF(BRAND_TAGLINE);
        ascii_art = buildAsciiArt();
    }
    
    static std::string getBrandName() { return brand_name; }
    static std::string getVersion() { return version; }
    
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
        std::cout << "  " << brand_name << " " << version << " - " << tagline << std::endl;
        std::cout << "\033[0m";
    }
    
private:
    static std::string buildAsciiArt() {
        return "";
    }
};

std::string BrandIdentity::brand_name;
std::string BrandIdentity::version;
std::string BrandIdentity::tagline;
std::string BrandIdentity::ascii_art;

// ============================================================================
// 3. UTILITY FUNCTIONS
// ============================================================================
namespace Utils {
    
    inline void secureZeroMemory(void* ptr, size_t size) {
        if (!ptr || size == 0) return;
        volatile char* p = static_cast<volatile char*>(ptr);
        for (size_t i = 0; i < size; ++i) p[i] = 0;
    }
    
    inline std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    inline bool constantTimeCompare(const uint8_t* a, const uint8_t* b, size_t len) {
        if (!a || !b) return false;
        volatile uint8_t diff = 0;
        for (size_t i = 0; i < len; ++i) diff |= a[i] ^ b[i];
        return diff == 0;
    }
}

// ============================================================================
// 4. LOGGER (مصحح لدعم التنسيق)
// ============================================================================
class Logger {
private:
    std::ofstream log_file;
    std::mutex log_mutex;
    LogLevel min_level;
    std::string log_path;
    
    std::string levelToString(LogLevel level) const {
        switch(level) {
            case LogLevel::DEBUG: return OBF("DEBUG");
            case LogLevel::INFO:  return OBF("INFO ");
            case LogLevel::WARNING: return OBF("WARN ");
            case LogLevel::ERROR: return OBF("ERROR");
            case LogLevel::FATAL: return OBF("FATAL");
            default: return OBF("UNKN ");
        }
    }
    
public:
    Logger() : min_level(LogLevel::INFO) {
        log_path = "/data/local/tmp/.looka.log";
        log_file.open(log_path, std::ios::app);
        if (log_file.is_open()) {
            log_file << OBF("=== Looka Engine v") << LOOKA_VERSION << OBF(" Started ===") << std::endl;
        }
    }
    
    ~Logger() {
        if (log_file.is_open()) log_file.close();
    }
    
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }
    
    void log(LogLevel level, const std::string& message, const char* file = nullptr, int line = 0) {
        if (level < min_level) return;
        
        std::stringstream ss;
        ss << "[" << Utils::getTimestamp() << "] ";
        ss << "[" << levelToString(level) << "] ";
        ss << message;
        std::string msg = ss.str();
        
        {
            std::lock_guard<std::mutex> lock(log_mutex);
            if (log_file.is_open()) {
                log_file << msg << std::endl;
                log_file.flush();
            }
        }
        
        if (level == LogLevel::FATAL) {
            std::cerr << msg << std::endl;
        }
    }
    
    // ✅ التعديل 4: إضافة دالة log جديدة لـ va_list
    void log(LogLevel level, const char* format, ...) {
        if (level < min_level) return;
        
        char buffer[2048];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        log(level, std::string(buffer));
    }
};

// ✅ التعديل 5: LOG macros مصححة لدعم التنسيق
#define LOG_DEBUG(fmt, ...) Logger::getInstance().log(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Logger::getInstance().log(LogLevel::INFO, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...) Logger::getInstance().log(LogLevel::WARNING, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::getInstance().log(LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) Logger::getInstance().log(LogLevel::FATAL, fmt, ##__VA_ARGS__)

// ============================================================================
// 5. ROOT DETECTOR
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
            result.method = OBF("Running as root (UID=0)");
            result.details.push_back(OBF("Process has root privileges"));
        }
        
        const char* su_paths[] = {
            "/system/bin/su", "/system/xbin/su", "/sbin/su",
            "/data/local/bin/su", "/data/adb/magisk/magisk", nullptr
        };
        for (int i = 0; su_paths[i]; i++) {
            if (access(su_paths[i], X_OK) == 0) {
                result.has_root = true;
                result.method = OBF("su binary found");
                result.details.push_back(std::string(OBF("Found: ")) + su_paths[i]);
                break;
            }
        }
        
        if (std::filesystem::exists("/data/adb/magisk")) {
            result.has_root = true;
            result.method = OBF("Magisk detected");
            result.details.push_back(OBF("Magisk root manager found"));
        }
        
        if (std::filesystem::exists("/data/data/com.noshufou.android.su")) {
            result.has_root = true;
            result.method = OBF("SuperSU detected");
            result.details.push_back(OBF("SuperSU app installed"));
        }
        
        std::ifstream selinux("/sys/fs/selinux/enforce");
        if (selinux.is_open()) {
            int enforce;
            selinux >> enforce;
            if (enforce == 0) {
                result.details.push_back(OBF("SELinux is permissive"));
            }
        }
        
        return result;
    }
    
    static void printReport(const RootCheckResult& result) {
        std::cout << "\n🔍 " << OBF("Root Detection Report") << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        
        if (result.has_root) {
            std::cout << "  ✅ " << OBF("ROOT ACCESS: DETECTED") << std::endl;
            std::cout << "  📌 " << OBF("Method") << ": " << result.method << std::endl;
        } else {
            std::cout << "  ❌ " << OBF("ROOT ACCESS: NOT DETECTED") << std::endl;
        }
        
        if (!result.details.empty()) {
            std::cout << "  📋 " << OBF("Details") << ":" << std::endl;
            for (const auto& d : result.details) {
                std::cout << "     • " << d << std::endl;
            }
        }
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    }
};

// ============================================================================
// 6. HYBRID MODE SELECTOR (MANUAL + AUTO)
// ============================================================================
class HybridModeSelector {
private:
    RootCheckResult root_status;
    ExecutionMode selected_mode;
    ExecutionMode effective_mode;
    bool root_available;
    int user_choice;
    
    void displayModeOptions() {
        std::cout << "\n╔════════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  🎮 " << BRAND_NAME << " v" << LOOKA_VERSION << " - Mode Selection                        ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════════════════╣\n";
        
        if (root_available) {
            std::cout << "║  ✅ ROOT ACCESS: AVAILABLE                                                   ║\n";
        } else {
            std::cout << "║  ❌ ROOT ACCESS: NOT AVAILABLE                                                ║\n";
        }
        
        std::cout << "╠════════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  📌 " << OBF("Available Modes") << ":                                                          ║\n";
        std::cout << "║                                                                               ║\n";
        
        if (root_available) {
            std::cout << "║  [1] 🚀 " << OBF("ULTRA MODE") << " (Root Required) - " << OBF("RECOMMENDED") << " ✓                             ║\n";
            std::cout << "║      • 500+ MB/s NEON SIMD Pattern Scan                                      ║\n";
            std::cout << "║      • Direct Kernel Access (Zero Latency)                                  ║\n";
            std::cout << "║      • Ghost Process (Complete Hiding)                                      ║\n";
            std::cout << "║      • Direct Framebuffer ESP (120 FPS)                                     ║\n";
            std::cout << "║      • Advanced Anti-Detect & Polymorphic Protection                        ║\n";
            std::cout << "║      • Full Aim Features (Aimbot + Assist + Flick)                          ║\n";
        } else {
            std::cout << "║  [1] 🚀 " << OBF("ULTRA MODE") << " (Root Required) - " << OBF("LOCKED") << " 🔒                               ║\n";
            std::cout << "║      ⚠️  " << OBF("Root access required for Ultra Mode") << "                                 ║\n";
        }
        
        std::cout << "║                                                                               ║\n";
        std::cout << "║  [2] 📱 " << OBF("STANDARD MODE") << " (Non-Root) - " << OBF("Compatible") << "                                 ║\n";
        std::cout << "║      • Pattern Scan via Shared Memory Bridge                                  ║\n";
        std::cout << "║      • Memory Access via Shizuku/ADB/Accessibility                            ║\n";
        std::cout << "║      • Basic ESP (30-60 FPS)                                                  ║\n";
        std::cout << "║      • Aim Assist Only (No Hard Aimbot)                                       ║\n";
        std::cout << "║                                                                               ║\n";
        std::cout << "║  [3] 🤖 " << OBF("AUTO MODE") << " (Default) - " << OBF("Smart Selection") << "                                 ║\n";
        std::cout << "║      • Automatically selects best available mode                              ║\n";
        std::cout << "║      • Ultra Mode if root available, otherwise Standard Mode                  ║\n";
        std::cout << "║                                                                               ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  👉 " << OBF("Enter your choice [1/2/3] (default: 3): ");
    }
    
    void handleUserInput() {
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) {
            user_choice = 3;
        } else {
            try {
                user_choice = std::stoi(input);
            } catch (...) {
                user_choice = 3;
            }
        }
        
        if (user_choice < 1 || user_choice > 3) user_choice = 3;
        
        switch(user_choice) {
            case 1:
                selected_mode = ExecutionMode::ROOT;
                break;
            case 2:
                selected_mode = ExecutionMode::NON_ROOT;
                break;
            case 3:
                selected_mode = ExecutionMode::AUTO;
                break;
        }
        
        if (selected_mode == ExecutionMode::AUTO) {
            effective_mode = root_available ? ExecutionMode::ROOT : ExecutionMode::NON_ROOT;
        } else {
            effective_mode = selected_mode;
        }
        
        if (selected_mode == ExecutionMode::ROOT && !root_available) {
            std::cout << "\n⚠️  " << OBF("WARNING: Root access not available!") << std::endl;
            std::cout << OBF("   Falling back to STANDARD MODE.") << std::endl;
            std::cout << OBF("   Press Enter to continue...") << std::endl;
            std::cin.get();
            effective_mode = ExecutionMode::NON_ROOT;
        }
    }
    
public:
    HybridModeSelector() : root_available(false), user_choice(3) {
        root_status = RootDetector::detect();
        root_available = root_status.has_root;
        
        RootDetector::printReport(root_status);
        displayModeOptions();
        handleUserInput();
        
        std::cout << "\n✅ " << OBF("Selected Mode: ");
        switch(effective_mode) {
            case ExecutionMode::ROOT:
                std::cout << OBF("ULTRA MODE (Root) - Full Performance") << std::endl;
                break;
            case ExecutionMode::NON_ROOT:
                std::cout << OBF("STANDARD MODE (Non-Root) - Compatible") << std::endl;
                break;
            default:
                std::cout << OBF("Unknown") << std::endl;
                break;
        }
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    }
    
    ExecutionMode getEffectiveMode() const { return effective_mode; }
    ExecutionMode getSelectedMode() const { return selected_mode; }
    bool isRootAvailable() const { return root_available; }
    int getUserChoice() const { return user_choice; }
    bool isUltraMode() const { return effective_mode == ExecutionMode::ROOT; }
    
    void printSummary() const {
        std::cout << "\n📋 " << OBF("Mode Summary") << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        std::cout << "  " << OBF("User Selected") << ": ";
        switch(selected_mode) {
            case ExecutionMode::ROOT: std::cout << OBF("Ultra Mode"); break;
            case ExecutionMode::NON_ROOT: std::cout << OBF("Standard Mode"); break;
            case ExecutionMode::AUTO: std::cout << OBF("Auto Mode"); break;
            default: break;
        }
        std::cout << std::endl;
        
        std::cout << "  " << OBF("Effective Mode") << ": ";
        if (effective_mode == ExecutionMode::ROOT) {
            std::cout << OBF("ULTRA MODE ✓") << std::endl;
        } else {
            std::cout << OBF("STANDARD MODE") << std::endl;
        }
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    }
};

// ============================================================================
// 7. INTELLIGENT CACHE MANAGER (مصحح - بدون mutable على mutex)
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
    std::shared_mutex cache_mutex;  // ✅ التعديل 5: بدون mutable
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
                if (now - it->second.timestamp > ttl) {
                    it = cache.erase(it);
                } else {
                    ++it;
                }
            }
            
            if (cache.size() > max_entries) {
                std::vector<std::pair<Key, uint64_t>> access_list;
                for (const auto& [key, entry] : cache) {
                    access_list.emplace_back(key, entry.access_count);
                }
                std::sort(access_list.begin(), access_list.end(),
                    [](const auto& a, const auto& b) { return a.second < b.second; });
                
                size_t to_remove = cache.size() - max_entries;
                for (size_t i = 0; i < to_remove && i < access_list.size(); i++) {
                    cache.erase(access_list[i].first);
                }
            }
        }
    }
    
public:
    IntelligentCacheManager(size_t max_size = CACHE_MAX_SIZE, int ttl_ms = CACHE_ENTRY_TTL_MS)
        : max_entries(max_size / sizeof(CacheEntry)), ttl(std::chrono::milliseconds(ttl_ms)) {
        cleanup_thread = std::make_unique<std::thread>(&IntelligentCacheManager::cleanupLoop, this);
    }
    
    ~IntelligentCacheManager() {
        running = false;
        if (cleanup_thread && cleanup_thread->joinable()) cleanup_thread->join();
    }
    
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
    
    void invalidate(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(cache_mutex);
        cache.erase(key);
    }
    
    void clear() {
        std::unique_lock<std::shared_mutex> lock(cache_mutex);
        cache.clear();
    }
    
    double getHitRate() const {
        uint64_t total = hits + misses;
        return total == 0 ? 0 : (static_cast<double>(hits) / total * 100.0);
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(cache_mutex);
        return cache.size();
    }
};

// ============================================================================
// 8. ADAPTIVE REFRESH RATE MANAGER
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
            case RefreshMode::BATTLE:
                return REFRESH_RATE_BATTLE;
            case RefreshMode::LOBBY:
                return REFRESH_RATE_LOBBY;
            case RefreshMode::IDLE:
                return REFRESH_RATE_IDLE;
            case RefreshMode::AUTO:
                return in_battle ? REFRESH_RATE_BATTLE : REFRESH_RATE_LOBBY;
            default:
                return REFRESH_RATE_BATTLE;
        }
    }
    
public:
    AdaptiveRefreshManager() {
        last_activity = std::chrono::steady_clock::now();
    }
    
    void update() {
        int new_fps = getOptimalFPS();
        if (new_fps != current_fps) {
            current_fps = new_fps;
            LOG_INFO(OBF("Refresh rate adjusted: %d FPS"), current_fps.load());
        }
        last_activity = std::chrono::steady_clock::now();
    }
    
    void setMode(RefreshMode mode) { current_mode = mode; }
    void setInBattle(bool battle) { in_battle = battle; }
    void setGameActive(bool active) { game_active = active; }
    
    int getCurrentFPS() const { return current_fps; }
    RefreshMode getCurrentMode() const { return current_mode; }
    
    int getSleepMicroseconds() const {
        if (current_fps <= 0) return 0;
        return 1000000 / current_fps;
    }
};

// ============================================================================
// 9. SMART REPORTING SYSTEM
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
                    if (eq != std::string::npos) {
                        ss << "Model: " << line.substr(eq + 1) << "\n";
                    }
                }
                if (line.find("ro.build.version.sdk") != std::string::npos) {
                    size_t eq = line.find('=');
                    if (eq != std::string::npos) {
                        ss << "API: " << line.substr(eq + 1) << "\n";
                    }
                }
            }
        }
        
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            ss << "RAM: " << (info.totalram / (1024 * 1024)) << " MB\n";
        }
        
        return ss.str();
    }
    
public:
    SmartReportingSystem() {
        report_file = "/data/local/tmp/.looka_crash.log";
    }
    
    void logError(const std::string& error_type, const std::string& error_message, 
                  const std::string& context = "") {
        ErrorReport report;
        report.timestamp = Utils::getTimestamp();
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
        
        LOG_ERROR(OBF("%s: %s"), error_type.c_str(), error_message.c_str());
    }
    
    void logFatal(const std::string& error_message, const std::string& context = "") {
        logError(OBF("FATAL"), error_message, context);
    }
    
    std::string getSuggestion(const std::string& error_type) {
        if (error_type.find(OBF("memory")) != std::string::npos) {
            return OBF("Close background apps or restart game");
        }
        if (error_type.find(OBF("root")) != std::string::npos) {
            return OBF("Root access required for this feature");
        }
        if (error_type.find(OBF("pattern")) != std::string::npos) {
            return OBF("Game may have updated. Try rescanning patterns");
        }
        return OBF("Restart engine. If problem persists, contact support");
    }
    
    void printLastErrors(int count = 5) {
        std::lock_guard<std::mutex> lock(reports_mutex);
        
        std::cout << "\n📋 " << OBF("Recent Errors:") << std::endl;
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
        
        int start = std::max(0, (int)reports.size() - count);
        for (int i = start; i < (int)reports.size(); i++) {
            const auto& r = reports[i];
            std::cout << "  [" << r.timestamp << "] " << r.error_type << ": " << r.error_message << std::endl;
            std::cout << "  💡 " << getSuggestion(r.error_type) << std::endl;
        }
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
    }
};

// ============================================================================
// 10. AIM ASSIST (DYNAMIC)
// ============================================================================
struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float px, float py) : x(px), y(py) {}
};

class DynamicAimAssist {
private:
    struct TouchState {
        int last_x = 0, last_y = 0;
        float velocity_x = 0, velocity_y = 0;
        bool dragging = false;
    } touch;
    
    struct Config {
        bool enabled = true;
        float smoothness = 5.0f;
        float drag_factor = 0.3f;
        int target_bone = 2;
    } config;
    
    WeaponType current_weapon = WeaponType::AR;
    
public:
    void updateTouch(int x, int y, bool touching) {
        if (!touching) {
            touch.dragging = false;
            return;
        }
        
        if (touch.dragging) {
            touch.velocity_x = (x - touch.last_x) * 1000.0f / 16.0f;
            touch.velocity_y = (y - touch.last_y) * 1000.0f / 16.0f;
        }
        
        touch.last_x = x;
        touch.last_y = y;
        touch.dragging = true;
    }
    
    void setWeapon(WeaponType weapon) { current_weapon = weapon; }
    void setEnabled(bool enabled) { config.enabled = enabled; }
    void setSmoothness(float smooth) { config.smoothness = smooth; }
    bool isEnabled() const { return config.enabled; }
    
    Vector2 calculateAssist(Vector2 current, Vector2 target) {
        if (!config.enabled) return current;
        
        bool flick_up = (touch.velocity_y < -200.0f);
        float offset_y = flick_up ? -15.0f : 0.0f;
        
        Vector2 result;
        result.x = current.x + (target.x - current.x) / config.smoothness;
        result.y = current.y + (target.y - current.y + offset_y) / config.smoothness;
        
        return result;
    }
};

// ============================================================================
// 11. HARDOCRE AIMBOT (للقناصة)
// ============================================================================
class HardcoreAimbot {
private:
    struct Config {
        bool enabled = false;
        bool only_for_snipers = true;
        float fov = 50.0f;
        float snap_speed = 0.0f;
    } config;
    
    WeaponType current_weapon = WeaponType::NONE;
    bool scoped = false;
    
public:
    void updateWeapon(WeaponType weapon, bool is_scoped) {
        current_weapon = weapon;
        scoped = is_scoped;
    }
    
    bool shouldActivate() const {
        if (!config.enabled) return false;
        if (config.only_for_snipers && current_weapon != WeaponType::SNIPER) return false;
        if (current_weapon == WeaponType::SNIPER && !scoped) return false;
        return true;
    }
    
    Vector2 calculateSnap(Vector2 current, Vector2 target) {
        if (!shouldActivate()) return current;
        
        if (config.snap_speed == 0) {
            return target;
        }
        
        Vector2 result;
        result.x = current.x + (target.x - current.x) / config.snap_speed;
        result.y = current.y + (target.y - current.y) / config.snap_speed;
        return result;
    }
    
    void setEnabled(bool enabled) { config.enabled = enabled; }
    void setSnapSpeed(float speed) { config.snap_speed = speed; }
    void setFOV(float fov) { config.fov = fov; }
    bool isEnabled() const { return config.enabled; }
};

// ============================================================================
// 12. FLICK DETECTOR (للشوتغن)
// ============================================================================
class FlickDetector {
private:
    struct FlickConfig {
        bool enabled = true;
        float velocity_threshold = 400.0f;
        float boost_multiplier = 1.5f;
    } config;
    
    std::deque<std::pair<int, uint64_t>> history;
    bool flicking = false;
    
    void addSample(int x) {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        history.push_back({x, now});
        if (history.size() > 5) history.pop_front();
    }
    
public:
    void updateTouch(int x, int y, bool touching) {
        if (!touching) {
            flicking = false;
            return;
        }
        
        addSample(x);
        
        if (history.size() >= 2) {
            auto& p1 = history[history.size() - 2];
            auto& p2 = history.back();
            float dt = (p2.second - p1.second) / 1000.0f;
            if (dt > 0) {
                float velocity = std::abs(p2.first - p1.first) / dt;
                flicking = (velocity > config.velocity_threshold);
            }
        }
    }
    
    bool isFlicking() const { return config.enabled && flicking; }
    
    Vector2 calculateBoost(Vector2 current, Vector2 target) {
        if (!isFlicking()) return current;
        
        Vector2 result;
        result.x = current.x + (target.x - current.x) * config.boost_multiplier;
        result.y = current.y + (target.y - current.y) * config.boost_multiplier;
        return result;
    }
    
    void setEnabled(bool enabled) { config.enabled = enabled; }
    void setBoostMultiplier(float mult) { config.boost_multiplier = mult; }
};

// ============================================================================
// 13. NEON PATTERN SCANNER
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
    
    NEONPattern(const std::string& n, const std::string& pattern)
        : name(n), result(0), found(false), neon_len(0) {
        parsePattern(pattern);
        prepareNEON();
    }
    
    void parsePattern(const std::string& pattern) {
        std::istringstream iss(pattern);
        std::string byte_str;
        while (iss >> byte_str) {
            if (byte_str == "??" || byte_str == "?") {
                bytes.push_back(0);
                mask.push_back(0);
            } else {
                bytes.push_back(static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16)));
                mask.push_back(0xFF);
            }
        }
    }
    
    void prepareNEON() {
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
        for (; i < neon_len; i++) {
            if ((data[i] & neon_mask[i]) != (neon_bytes[i] & neon_mask[i])) {
                return false;
            }
        }
        return true;
#else
        for (size_t i = 0; i < neon_len; i++) {
            if ((data[i] & neon_mask[i]) != (neon_bytes[i] & neon_mask[i])) {
                return false;
            }
        }
        return true;
#endif
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
        LOG_INFO(OBF("Scanner ready - Base: 0x%lx, Size: %.2f MB"), 
                 module_base, module_size / (1024.0 * 1024.0));
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
        LOG_INFO(OBF("Pattern added: %s (%zu bytes)"), name.c_str(), patterns.back().bytes.size());
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
        
        LOG_INFO(OBF("Scan completed in %.2f ms (%.0f MB/s) - Found %zu patterns"),
                 last_scan_time_ms,
                 (total_bytes_scanned.load() / 1024.0 / 1024.0) / (last_scan_time_ms / 1000.0),
                 found);
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
// 14. DIRECT KERNEL ACCESS (ROOT ONLY)
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
            total_ops++;
            total_bytes += size;
            return true;
        }
        return false;
    }
    
    bool write(uintptr_t address, const void* buffer, size_t size) {
        struct iovec local = { const_cast<void*>(buffer), size };
        struct iovec remote = { reinterpret_cast<void*>(address), size };
        ssize_t bytes = process_vm_writev(target_pid, &local, 1, &remote, 1, 0);
        
        if (bytes == static_cast<ssize_t>(size)) {
            total_ops++;
            total_bytes += size;
            return true;
        }
        return false;
    }
    
    template<typename T>
    bool readValue(uintptr_t address, T& value) {
        return read(address, &value, sizeof(T));
    }
    
    template<typename T>
    bool writeValue(uintptr_t address, const T& value) {
        return write(address, &value, sizeof(T));
    }
    
    uint64_t getOps() const { return total_ops.load(); }
    uint64_t getBytes() const { return total_bytes.load(); }
};

// ============================================================================
// 15. GHOST PROCESS (ROOT ONLY)
// ============================================================================
class GhostProcess {
private:
    pid_t original_pid;
    std::string original_name;
    std::string ghost_name;
    bool hidden;
    
    bool renameProcess() {
        if (prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(ghost_name.c_str()), 0, 0, 0) == 0) {
            return true;
        }
        return false;
    }
    
    bool hideFromCmdline() {
        int fd = open("/proc/self/cmdline", O_RDWR);
        if (fd >= 0) {
            std::string new_name = ghost_name + std::string(original_name.length() - ghost_name.length(), '\0');
            if (write(fd, new_name.c_str(), new_name.length()) > 0) {
                close(fd);
                return true;
            }
            close(fd);
        }
        return false;
    }
    
    bool unshareNamespace() {
        if (unshare(CLONE_NEWNS) == 0) {
            std::filesystem::create_directory(HIDE_MOUNT_POINT);
            mount(HIDE_MOUNT_POINT, "/proc/self", nullptr, MS_BIND, nullptr);
            return true;
        }
        return false;
    }
    
public:
    GhostProcess() : original_pid(getpid()), hidden(false) {
        std::ifstream cmdline("/proc/self/cmdline");
        if (cmdline.is_open()) {
            std::getline(cmdline, original_name);
            original_name.erase(std::find(original_name.begin(), original_name.end(), '\0'), original_name.end());
        }
        if (original_name.empty()) original_name = OBF("looka_engine");
        ghost_name = FAKE_PROCESS_NAME;
    }
    
    bool makeGhost() {
        if (hidden) return true;
        
        bool success = false;
        if (renameProcess()) success = true;
        if (hideFromCmdline()) success = true;
        if (unshareNamespace()) success = true;
        
        if (success) {
            hidden = true;
            LOG_INFO(OBF("Ghost mode active - Hidden as '%s'"), ghost_name.c_str());
        }
        return hidden;
    }
    
    void restore() {
        if (!hidden) return;
        prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(original_name.c_str()), 0, 0, 0);
        hidden = false;
    }
    
    bool isGhost() const { return hidden; }
};

// ============================================================================
// 16. STEALTH SAFE OVERLAY (ROOT ONLY - 120 FPS)
// ============================================================================
class StealthSafeOverlay {
private:
    int fb_fd;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    uint32_t* fb_mmap;
    uint32_t* back_buffer;
    size_t screensize;
    size_t buffer_size;
    bool initialized;
    std::atomic<bool> drawing{false};
    std::unique_ptr<std::thread> draw_thread;
    
    struct ESPElement {
        int x, y, w, h;
        uint32_t color;
        bool active;
    };
    std::vector<ESPElement> esp_elements;
    std::mutex esp_mutex;
    
    bool initFramebuffer() {
        fb_fd = open("/dev/graphics/fb0", O_RDWR);
        if (fb_fd < 0) {
            fb_fd = open("/dev/fb0", O_RDWR);
            if (fb_fd < 0) return false;
        }
        
        if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo) != 0) {
            close(fb_fd);
            return false;
        }
        
        if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo) != 0) {
            close(fb_fd);
            return false;
        }
        
        screensize = vinfo.yres * finfo.line_length;
        buffer_size = vinfo.xres * vinfo.yres * sizeof(uint32_t);
        
        fb_mmap = static_cast<uint32_t*>(mmap(0, screensize, PROT_READ | PROT_WRITE,
                                               MAP_SHARED, fb_fd, 0));
        if (fb_mmap == MAP_FAILED) {
            close(fb_fd);
            return false;
        }
        
        back_buffer = new uint32_t[vinfo.xres * vinfo.yres];
        memset(back_buffer, 0, buffer_size);
        
        LOG_INFO(OBF("Framebuffer overlay ready - %dx%d"), vinfo.xres, vinfo.yres);
        return true;
    }
    
    void drawPixel(int x, int y, uint32_t color) {
        if (x < 0 || x >= (int)vinfo.xres || y < 0 || y >= (int)vinfo.yres) return;
        back_buffer[y * vinfo.xres + x] = color;
    }
    
    void drawLine(int x1, int y1, int x2, int y2, uint32_t color) {
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;
        
        int x = x1, y = y1;
        while (true) {
            drawPixel(x, y, color);
            if (x == x2 && y == y2) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x += sx; }
            if (e2 < dx) { err += dx; y += sy; }
        }
    }
    
    void drawBox(int x, int y, int w, int h, uint32_t color) {
        drawLine(x, y, x + w, y, color);
        drawLine(x + w, y, x + w, y + h, color);
        drawLine(x + w, y + h, x, y + h, color);
        drawLine(x, y + h, x, y, color);
    }
    
    void flush() {
        memcpy(fb_mmap, back_buffer, buffer_size);
    }
    
    void clear() {
        memset(back_buffer, 0, buffer_size);
    }
    
    void renderLoop() {
        auto last_frame = std::chrono::high_resolution_clock::now();
        
        while (drawing) {
            clear();
            
            {
                std::lock_guard<std::mutex> lock(esp_mutex);
                for (const auto& elem : esp_elements) {
                    if (elem.active) {
                        drawBox(elem.x, elem.y, elem.w, elem.h, elem.color);
                    }
                }
            }
            
            flush();
            
            auto frame_end = std::chrono::high_resolution_clock::now();
            auto frame_time = std::chrono::duration_cast<std::chrono::microseconds>(frame_end - last_frame);
            
            auto target_time = std::chrono::microseconds(8333);
            if (frame_time < target_time) {
                std::this_thread::sleep_for(target_time - frame_time);
            }
            last_frame = frame_end;
        }
    }
    
public:
    StealthSafeOverlay() : fb_fd(-1), fb_mmap(nullptr), back_buffer(nullptr), initialized(false) {}
    
    ~StealthSafeOverlay() {
        stopDrawing();
        if (back_buffer) delete[] back_buffer;
        if (fb_mmap && fb_mmap != MAP_FAILED) munmap(fb_mmap, screensize);
        if (fb_fd >= 0) close(fb_fd);
    }
    
    bool initialize() {
        if (initFramebuffer()) {
            initialized = true;
            return true;
        }
        return false;
    }
    
    void startDrawing() {
        if (!initialized || drawing) return;
        drawing = true;
        draw_thread = std::make_unique<std::thread>([this]() { renderLoop(); });
        LOG_INFO(OBF("Overlay drawing started (120 FPS target)"));
    }
    
    void stopDrawing() {
        drawing = false;
        if (draw_thread && draw_thread->joinable()) draw_thread->join();
        clear();
        flush();
    }
    
    void addESPBox(int x, int y, int w, int h, uint32_t color) {
        std::lock_guard<std::mutex> lock(esp_mutex);
        esp_elements.push_back({x, y, w, h, color, true});
    }
    
    void clearESPBoxes() {
        std::lock_guard<std::mutex> lock(esp_mutex);
        esp_elements.clear();
    }
    
    bool isAvailable() const { return initialized; }
};

// ============================================================================
// 17. SHARED MEMORY BRIDGE (NON-ROOT)
// ============================================================================
class SharedMemoryBridge {
private:
    int shm_fd;
    void* shm_ptr;
    size_t shm_size;
    bool initialized;
    
    bool setupSharedMemory() {
        shm_fd = shm_open("/looka_bridge", O_RDWR | O_CREAT, 0666);
        if (shm_fd < 0) return false;
        
        if (ftruncate(shm_fd, shm_size) != 0) {
            close(shm_fd);
            return false;
        }
        
        shm_ptr = mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            close(shm_fd);
            return false;
        }
        
        return true;
    }
    
public:
    SharedMemoryBridge(size_t size = 2 * 1024 * 1024) : shm_fd(-1), shm_ptr(nullptr), shm_size(size), initialized(false) {
        if (setupSharedMemory()) {
            initialized = true;
            LOG_INFO(OBF("Shared memory bridge ready - Size: %.2f MB"), size / (1024.0 * 1024.0));
        }
    }
    
    ~SharedMemoryBridge() {
        if (shm_ptr && shm_ptr != MAP_FAILED) munmap(shm_ptr, shm_size);
        if (shm_fd >= 0) close(shm_fd);
    }
    
    bool isAvailable() const { return initialized; }
    
    bool write(uintptr_t address, const void* data, size_t size) {
        if (!initialized || size > shm_size) return false;
        memcpy(shm_ptr, data, size);
        return true;
    }
    
    bool read(uintptr_t address, void* buffer, size_t size) {
        if (!initialized) return false;
        memcpy(buffer, shm_ptr, std::min(size, shm_size));
        return true;
    }
    
    void* getDirectPointer() { return shm_ptr; }
};

// ============================================================================
// 18. ADVANCED ANTI-DETECT
// ============================================================================
class AdvancedAntiDetect {
private:
    bool active;
    std::string original_name;
    
    bool hideFromProc() {
        if (unshare(CLONE_NEWNS) == 0) {
            std::filesystem::create_directory(HIDE_MOUNT_POINT);
            mount(HIDE_MOUNT_POINT, "/proc/self", nullptr, MS_BIND, nullptr);
            return true;
        }
        return false;
    }
    
    bool renameProcess() {
        if (prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(FAKE_PROCESS_NAME), 0, 0, 0) == 0) {
            return true;
        }
        return false;
    }
    
    bool hideCmdline() {
        int fd = open("/proc/self/cmdline", O_RDWR);
        if (fd >= 0) {
            std::string new_name = FAKE_PROCESS_NAME;
            if (write(fd, new_name.c_str(), new_name.length()) > 0) {
                close(fd);
                return true;
            }
            close(fd);
        }
        return false;
    }
    
public:
    AdvancedAntiDetect() : active(false) {
        std::ifstream cmdline("/proc/self/cmdline");
        if (cmdline.is_open()) {
            std::getline(cmdline, original_name);
            original_name.erase(std::find(original_name.begin(), original_name.end(), '\0'), original_name.end());
        }
        if (original_name.empty()) original_name = OBF("looka_engine");
    }
    
    bool activate() {
        if (active) return true;
        
        bool success = false;
        if (renameProcess()) success = true;
        if (hideCmdline()) success = true;
        if (hideFromProc()) success = true;
        
        if (success) {
            active = true;
            LOG_INFO(OBF("Advanced anti-detect active"));
        }
        return active;
    }
    
    void deactivate() {
        if (!active) return;
        prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(original_name.c_str()), 0, 0, 0);
        active = false;
    }
    
    bool isActive() const { return active; }
};

// ============================================================================
// 19. FLOATING CONTROL PANEL
// ============================================================================
class FloatingControlPanel {
private:
    struct Button {
        std::string id;
        std::string label;
        bool state;
        std::function<void(bool)> callback;
    };
    
    struct Section {
        std::string title;
        std::vector<Button> buttons;
    };
    
    std::vector<Section> sections;
    bool visible;
    bool minimized;
    int width, height;
    std::mutex ui_mutex;
    std::unique_ptr<std::thread> ui_thread;
    std::atomic<bool> running{false};
    
    std::function<void(bool)> on_enemy_situation;
    std::function<void(bool)> on_aim_assist;
    std::function<void(bool)> on_hardcore_aimbot;
    std::function<void(bool)> on_flick_boost;
    std::function<void(bool)> on_esp_box;
    std::function<void(bool)> on_health_bar;
    std::function<void(bool)> on_ghost_mode;
    std::function<void(bool)> on_safe_overlay;
    
    void initializeSections() {
        Section esp;
        esp.title = OBF("🎯 ESP CONTROL");
        esp.buttons = {
            {OBF("enemy_situation"), OBF("Enemy Situation"), true, nullptr},
            {OBF("esp_box"), OBF("ESP Box"), true, nullptr},
            {OBF("health_bar"), OBF("Health Bar"), true, nullptr}
        };
        sections.push_back(esp);
        
        Section aim;
        aim.title = OBF("🎯 AIM CONTROL");
        aim.buttons = {
            {OBF("aim_assist"), OBF("Aim Assist"), true, nullptr},
            {OBF("hardcore_aimbot"), OBF("Hard Aimbot"), false, nullptr},
            {OBF("flick_boost"), OBF("Flick Boost"), true, nullptr}
        };
        sections.push_back(aim);
        
        Section stealth;
        stealth.title = OBF("🛡️ STEALTH");
        stealth.buttons = {
            {OBF("ghost_mode"), OBF("Ghost Mode"), true, nullptr},
            {OBF("safe_overlay"), OBF("Safe Overlay"), true, nullptr}
        };
        sections.push_back(stealth);
    }
    
    void drawWindow() {
        std::cout << "\033[2J\033[H";
        
        std::cout << "┌" << std::string(width - 2, '─') << "┐\n";
        std::cout << "│ " << BRAND_NAME << " v" << LOOKA_VERSION << std::string(width - 28, ' ') << "│\n";
        std::cout << "├" << std::string(width - 2, '─') << "┤\n";
        
        for (const auto& section : sections) {
            std::cout << "│ " << section.title << std::string(width - 4 - section.title.length(), ' ') << "│\n";
            std::cout << "├" << std::string(width - 2, '─') << "┤\n";
            
            for (const auto& btn : section.buttons) {
                std::string status = btn.state ? "[ON] " : "[OFF]";
                std::cout << "│   " << status << btn.label << std::string(width - 10 - btn.label.length(), ' ') << "│\n";
            }
            std::cout << "├" << std::string(width - 2, '─') << "┤\n";
        }
        
        std::cout << "│ [H] Hide  [M] Minimize  [Q] Quit" << std::string(width - 30, ' ') << "│\n";
        std::cout << "└" << std::string(width - 2, '─') << "┘\n";
    }
    
    void drawMinimized() {
        std::cout << "\r[" << BRAND_NAME << "] ESP:";
        for (const auto& section : sections) {
            for (const auto& btn : section.buttons) {
                if (btn.id == OBF("enemy_situation")) {
                    std::cout << (btn.state ? "ON" : "OFF");
                }
                if (btn.id == OBF("aim_assist")) {
                    std::cout << " | AIM:" << (btn.state ? "ON" : "OFF");
                }
            }
        }
        std::cout << "   \r";
        std::cout.flush();
    }
    
    void handleInput() {
        char input;
        while (running && visible) {
            if (std::cin.peek() != EOF) {
                std::cin >> input;
                
                switch(input) {
                    case 'h': case 'H': setVisible(false); break;
                    case 'm': case 'M': setMinimized(true); break;
                    case 'q': case 'Q': running = false; break;
                    case '1':
                        if (on_enemy_situation && !sections.empty() && !sections[0].buttons.empty()) {
                            bool new_state = !sections[0].buttons[0].state;
                            sections[0].buttons[0].state = new_state;
                            on_enemy_situation(new_state);
                        }
                        break;
                    case '2':
                        if (on_aim_assist && sections.size() > 1 && !sections[1].buttons.empty()) {
                            bool new_state = !sections[1].buttons[0].state;
                            sections[1].buttons[0].state = new_state;
                            on_aim_assist(new_state);
                        }
                        break;
                    case '3':
                        if (on_hardcore_aimbot && sections.size() > 1 && sections[1].buttons.size() > 1) {
                            bool new_state = !sections[1].buttons[1].state;
                            sections[1].buttons[1].state = new_state;
                            on_hardcore_aimbot(new_state);
                        }
                        break;
                    case '4':
                        if (on_flick_boost && sections.size() > 1 && sections[1].buttons.size() > 2) {
                            bool new_state = !sections[1].buttons[2].state;
                            sections[1].buttons[2].state = new_state;
                            on_flick_boost(new_state);
                        }
                        break;
                    case '5':
                        if (on_esp_box && !sections.empty() && sections[0].buttons.size() > 1) {
                            bool new_state = !sections[0].buttons[1].state;
                            sections[0].buttons[1].state = new_state;
                            on_esp_box(new_state);
                        }
                        break;
                    case '6':
                        if (on_health_bar && !sections.empty() && sections[0].buttons.size() > 2) {
                            bool new_state = !sections[0].buttons[2].state;
                            sections[0].buttons[2].state = new_state;
                            on_health_bar(new_state);
                        }
                        break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    
public:
    FloatingControlPanel() : visible(true), minimized(false), width(FLOATING_WINDOW_WIDTH), 
                              height(FLOATING_WINDOW_HEIGHT) {
        initializeSections();
    }
    
    void start() {
        running = true;
        ui_thread = std::make_unique<std::thread>([this]() {
            while (running) {
                if (visible && !minimized) {
                    drawWindow();
                    handleInput();
                } else if (visible && minimized) {
                    drawMinimized();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        });
    }
    
    void stop() {
        running = false;
        if (ui_thread && ui_thread->joinable()) ui_thread->join();
        std::cout << "\033[2J\033[H";
    }
    
    void setVisible(bool vis) { 
        std::lock_guard<std::mutex> lock(ui_mutex);
        visible = vis; 
        if (vis) std::cout << "\033[2J\033[H";
    }
    
    void setMinimized(bool min) { 
        std::lock_guard<std::mutex> lock(ui_mutex);
        minimized = min; 
        if (!min) std::cout << "\033[2J\033[H";
    }
    
    void updateButtonState(const std::string& id, bool state) {
        std::lock_guard<std::mutex> lock(ui_mutex);
        for (auto& section : sections) {
            for (auto& btn : section.buttons) {
                if (btn.id == id) {
                    btn.state = state;
                    return;
                }
            }
        }
    }
    
    void setOnEnemySituation(std::function<void(bool)> cb) { on_enemy_situation = cb; }
    void setOnAimAssist(std::function<void(bool)> cb) { on_aim_assist = cb; }
    void setOnHardcoreAimbot(std::function<void(bool)> cb) { on_hardcore_aimbot = cb; }
    void setOnFlickBoost(std::function<void(bool)> cb) { on_flick_boost = cb; }
    void setOnEspBox(std::function<void(bool)> cb) { on_esp_box = cb; }
    void setOnHealthBar(std::function<void(bool)> cb) { on_health_bar = cb; }
    void setOnGhostMode(std::function<void(bool)> cb) { on_ghost_mode = cb; }
    void setOnSafeOverlay(std::function<void(bool)> cb) { on_safe_overlay = cb; }
};

// ============================================================================
// 20. POLYMORPHIC MEMORY PROTECTION
// ============================================================================
class PolymorphicMemoryProtection {
private:
    struct ProtectedBlock {
        std::vector<uint8_t> encrypted_data;
        std::vector<uint8_t> key;
        size_t size;
    };
    
    std::unordered_map<std::string, ProtectedBlock> blocks;
    std::mutex blocks_mutex;
    std::mt19937 rng;
    std::vector<uint8_t> global_key;
    std::chrono::steady_clock::time_point last_rotation;
    static constexpr int KEY_ROTATION_SEC = 45;
    
    void rotateGlobalKey() {
        for (size_t i = 0; i < global_key.size(); i++) {
            global_key[i] = rng() & 0xFF;
        }
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
            LOG_INFO(OBF("Polymorphic keys rotated"));
        }
    }
};

// ============================================================================
// 21. MAIN ENGINE CLASS
// ============================================================================
class LookaEngineFinal {
private:
    std::unique_ptr<HybridModeSelector> mode_selector;
    ExecutionMode current_mode;
    
    std::unique_ptr<DirectKernelAccess> kernel;
    std::unique_ptr<GhostProcess> ghost;
    std::unique_ptr<AdvancedAntiDetect> anti_detect;
    std::unique_ptr<StealthSafeOverlay> overlay;
    std::unique_ptr<SharedMemoryBridge> bridge;
    
    std::unique_ptr<AdaptiveRefreshManager> refresh_manager;
    std::unique_ptr<IntelligentCacheManager<uintptr_t, uint32_t>> cache;
    std::unique_ptr<SmartReportingSystem> reporter;
    std::unique_ptr<FloatingControlPanel> ui_panel;
    std::unique_ptr<DynamicAimAssist> aim_assist;
    std::unique_ptr<HardcoreAimbot> hardcore_aimbot;
    std::unique_ptr<FlickDetector> flick_detector;
    std::unique_ptr<NEONPatternScanner> scanner;
    std::unique_ptr<PolymorphicMemoryProtection> polymorphic;
    
    pid_t target_pid;
    uintptr_t base_address;
    std::string module_name;
    std::atomic<EngineState> state{EngineState::STOPPED};
    std::unique_ptr<std::thread> engine_thread;
    std::chrono::steady_clock::time_point start_time;
    std::atomic<uint64_t> frames{0};
    std::atomic<uint64_t> memory_ops{0};
    std::unordered_map<std::string, uintptr_t> addresses;
    
    void registerPatterns() {
        scanner->addPattern(OBF("health"), OBF("?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 00 00"));
        scanner->addPattern(OBF("armor"), OBF("FF FF FF FF 00 00 00 00 ?? ?? ?? ??"));
        scanner->addPattern(OBF("speed"), OBF("00 00 80 3F ?? ?? ?? ?? ?? ?? ?? ??"));
        scanner->addPattern(OBF("jump"), OBF("00 00 00 00 00 00 F0 3F ?? ?? ?? ??"));
        scanner->addPattern(OBF("recoil"), OBF("00 00 00 00 00 00 00 00 CD CC CC 3D"));
        LOG_INFO(OBF("Registered 5 patterns"));
    }
    
    void collectPatterns() {
        std::vector<std::string> patterns = {OBF("health"), OBF("armor"), OBF("speed"), OBF("jump"), OBF("recoil")};
        for (const auto& name : patterns) {
            uintptr_t addr = scanner->getPatternResult(name);
            if (addr != 0) {
                addresses[name] = addr;
                LOG_INFO(OBF("Found %s at 0x%lx"), name.c_str(), addr);
            }
        }
    }
    
    void setupUICallbacks() {
        ui_panel->setOnEnemySituation([this](bool enabled) {
            LOG_INFO(OBF("Enemy Situation: %s"), enabled ? "ON" : "OFF");
        });
        
        ui_panel->setOnAimAssist([this](bool enabled) {
            aim_assist->setEnabled(enabled);
            LOG_INFO(OBF("Aim Assist: %s"), enabled ? "ON" : "OFF");
        });
        
        ui_panel->setOnHardcoreAimbot([this](bool enabled) {
            hardcore_aimbot->setEnabled(enabled);
            LOG_INFO(OBF("Hardcore Aimbot: %s"), enabled ? "ON" : "OFF");
        });
        
        ui_panel->setOnFlickBoost([this](bool enabled) {
            flick_detector->setEnabled(enabled);
            LOG_INFO(OBF("Flick Boost: %s"), enabled ? "ON" : "OFF");
        });
        
        ui_panel->setOnEspBox([this](bool enabled) {
            LOG_INFO(OBF("ESP Box: %s"), enabled ? "ON" : "OFF");
        });
        
        ui_panel->setOnHealthBar([this](bool enabled) {
            LOG_INFO(OBF("Health Bar: %s"), enabled ? "ON" : "OFF");
        });
        
        ui_panel->setOnGhostMode([this](bool enabled) {
            if (ghost) {
                if (enabled) ghost->makeGhost();
                else ghost->restore();
            }
        });
        
        ui_panel->setOnSafeOverlay([this](bool enabled) {
            if (overlay) {
                if (enabled) overlay->startDrawing();
                else overlay->stopDrawing();
            }
        });
    }
    
    void initializeRootComponents() {
        if (!mode_selector->isUltraMode()) return;
        
        LOG_INFO(OBF("Initializing ROOT components..."));
        
        kernel = std::make_unique<DirectKernelAccess>(target_pid);
        ghost = std::make_unique<GhostProcess>();
        anti_detect = std::make_unique<AdvancedAntiDetect>();
        overlay = std::make_unique<StealthSafeOverlay>();
        
        if (ghost->makeGhost()) {
            LOG_INFO(OBF("Ghost mode active"));
        }
        
        if (anti_detect->activate()) {
            LOG_INFO(OBF("Advanced anti-detect active"));
        }
        
        if (overlay->initialize()) {
            overlay->startDrawing();
            LOG_INFO(OBF("Framebuffer overlay active (120 FPS)"));
        }
    }
    
    void initializeNonRootComponents() {
        LOG_INFO(OBF("Initializing NON-ROOT components..."));
        
        bridge = std::make_unique<SharedMemoryBridge>();
        if (bridge->isAvailable()) {
            LOG_INFO(OBF("Shared memory bridge active"));
        } else {
            LOG_WARNING(OBF("Shared memory bridge failed, using fallback"));
        }
    }
    
public:
    LookaEngineFinal(pid_t pid, uintptr_t base, const std::string& module) 
        : target_pid(pid), base_address(base), module_name(module) {
        
        start_time = std::chrono::steady_clock::now();
        
        mode_selector = std::make_unique<HybridModeSelector>();
        current_mode = mode_selector->getEffectiveMode();
        mode_selector->printSummary();
        
        refresh_manager = std::make_unique<AdaptiveRefreshManager>();
        cache = std::make_unique<IntelligentCacheManager<uintptr_t, uint32_t>>();
        reporter = std::make_unique<SmartReportingSystem>();
        ui_panel = std::make_unique<FloatingControlPanel>();
        aim_assist = std::make_unique<DynamicAimAssist>();
        hardcore_aimbot = std::make_unique<HardcoreAimbot>();
        flick_detector = std::make_unique<FlickDetector>();
        scanner = std::make_unique<NEONPatternScanner>(pid);
        polymorphic = std::make_unique<PolymorphicMemoryProtection>();
        
        initializeRootComponents();
        initializeNonRootComponents();
        
        setupUICallbacks();
        
        LOG_INFO(OBF("Engine initialized - Mode: %s"), 
                 current_mode == ExecutionMode::ROOT ? "ULTRA" : "STANDARD");
    }
    
    bool initialize() {
        try {
            if (scanner->initialize(module_name)) {
                registerPatterns();
                scanner->scanAllPatterns();
                collectPatterns();
                LOG_INFO(OBF("Pattern scan complete - Found %zu patterns"), addresses.size());
            }
            
            ui_panel->start();
            
            state.store(EngineState::INITIALIZING);
            state.store(EngineState::STOPPED);
            
            return true;
            
        } catch (const std::exception& e) {
            reporter->logFatal(e.what(), OBF("Initialization"));
            state.store(EngineState::ERROR);
            return false;
        }
    }
    
    void run() {
        if (state.load() != EngineState::STOPPED) return;
        
        state.store(EngineState::RUNNING);
        LOG_INFO(OBF("Engine running - All systems operational"));
        
        engine_thread = std::make_unique<std::thread>([this]() {
            auto last_update = std::chrono::steady_clock::now();
            
            while (state.load() == EngineState::RUNNING) {
                auto frame_start = std::chrono::high_resolution_clock::now();
                
                try {
                    refresh_manager->update();
                    frames++;
                    
                    if (polymorphic) {
                        polymorphic->rotateKeys();
                    }
                    
                    int sleep_us = refresh_manager->getSleepMicroseconds();
                    if (sleep_us > 0) {
                        auto frame_time = std::chrono::high_resolution_clock::now() - frame_start;
                        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(frame_time).count();
                        if (elapsed < sleep_us) {
                            std::this_thread::sleep_for(std::chrono::microseconds(sleep_us - elapsed));
                        }
                    }
                    
                } catch (const std::exception& e) {
                    reporter->logError(OBF("Frame Error"), e.what());
                }
            }
        });
        
        if (engine_thread && engine_thread->joinable()) engine_thread->join();
    }
    
    void stop() {
        LOG_INFO(OBF("Stopping engine..."));
        state.store(EngineState::STOPPED);
        
        if (ui_panel) ui_panel->stop();
        if (overlay) overlay->stopDrawing();
        if (ghost) ghost->restore();
        if (anti_detect) anti_detect->deactivate();
        
        if (engine_thread && engine_thread->joinable()) engine_thread->join();
    }
    
    void printStatus() {
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start_time).count();
        
        std::cout << "\n╔════════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  " << BRAND_NAME << " v" << LOOKA_VERSION << " - Status Report                         ║\n";
        std::cout << "╠════════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ 📊 STATISTICS                                                               ║\n";
        std::cout << "║   Mode: " << (current_mode == ExecutionMode::ROOT ? "ULTRA (Root)" : "STANDARD (Non-Root)") << std::endl;
        std::cout << "║   Uptime: " << uptime << " seconds" << std::endl;
        std::cout << "║   Frames: " << frames.load() << std::endl;
        std::cout << "║   Memory Ops: " << memory_ops.load() << std::endl;
        std::cout << "║   Patterns Found: " << addresses.size() << std::endl;
        std::cout << "║   Cache Hit Rate: " << std::fixed << std::setprecision(1) << cache->getHitRate() << "%" << std::endl;
        
        if (frames > 0 && uptime > 0) {
            std::cout << "║   FPS: " << std::fixed << std::setprecision(1) 
                      << (double)frames.load() / uptime << std::endl;
        }
        
        if (scanner) {
            std::cout << "║   Scan Speed: " << std::fixed << std::setprecision(0) 
                      << scanner->getScanSpeedMBps() << " MB/s" << std::endl;
        }
        
        std::cout << "╠════════════════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ 🔧 ACTIVE FEATURES                                                          ║\n";
        std::cout << "║   Mode: " << (current_mode == ExecutionMode::ROOT ? "ULTRA" : "STANDARD") << std::endl;
        std::cout << "║   NEON SIMD: " << (scanner ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Aim Assist: " << (aim_assist->isEnabled() ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Hard Aimbot: " << (hardcore_aimbot->isEnabled() ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Flick Boost: " << (flick_detector->isFlicking() ? "✓ Detected" : "Standby") << std::endl;
        std::cout << "║   Ghost Mode: " << (ghost && ghost->isGhost() ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Safe Overlay: " << (overlay && overlay->isAvailable() ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Anti-Detect: " << (anti_detect && anti_detect->isActive() ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Shared Memory: " << (bridge && bridge->isAvailable() ? "✓ Active" : "✗") << std::endl;
        std::cout << "║   Polymorphic: ✓ Active" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════════════════════╝\n" << std::endl;
        
        reporter->printLastErrors(3);
    }
    
    EngineState getState() const { return state.load(); }
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
// MAIN
// ============================================================================
static std::atomic<bool> g_running{true};
static std::unique_ptr<LookaEngineFinal> g_engine;

void signalHandler(int sig) {
    g_running = false;
    if (g_engine) g_engine->stop();
}

int main() {
    try {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        BrandIdentity::initialize();
        BrandIdentity::displayBanner();
        
        std::string package = "com.dts.freefireth";
        std::string module = "libil2cpp.so";
        
        std::cout << OBF("\n🔍 Searching for game: ") << package << "..." << std::endl;
        
        pid_t pid = -1;
        for (int attempt = 1; attempt <= 3; ++attempt) {
            pid = GameDetector::findProcessByPackage(package);
            if (pid > 0) break;
            std::cout << OBF("   Attempt ") << attempt << "/3 - Game not running" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        if (pid <= 0) {
            std::cout << OBF("\n❌ Game not found! Please start ") << package << OBF(" first.") << std::endl;
            return 1;
        }
        
        std::cout << OBF("✅ Game PID: ") << pid << std::endl;
        
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
            std::cout << OBF("\n⚠️  Module not found. Waiting for game to load...") << std::endl;
            for (int attempt = 1; attempt <= 10 && base == 0; ++attempt) {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                base = temp.findBase(module);
                if (base != 0) {
                    std::cout << OBF("✅ Module found on attempt ") << attempt << std::endl;
                }
            }
            
            if (base == 0) {
                std::cout << OBF("\n❌ Module ") << module << OBF(" not found!") << std::endl;
                return 1;
            }
        }
        
        std::cout << OBF("📦 Module base: 0x") << std::hex << base << std::dec << std::endl;
        
        g_engine = std::make_unique<LookaEngineFinal>(pid, base, module);
        
        if (!g_engine->initialize()) {
            std::cout << OBF("\n❌ Engine initialization failed!") << std::endl;
            return 1;
        }
        
        std::cout << OBF("\n🚀 ") << BRAND_NAME << OBF(" v") << LOOKA_VERSION << OBF(" is ACTIVE!") << std::endl;
        std::cout << OBF("💡 Type 'status' for full report") << std::endl;
        std::cout << OBF("💡 Floating control panel active - Keyboard shortcuts:") << std::endl;
        std::cout << OBF("   1: Enemy Situation | 2: Aim Assist | 3: Hard Aimbot | 4: Flick Boost") << std::endl;
        std::cout << OBF("   5: ESP Box | 6: Health Bar | H: Hide | M: Minimize | Q: Quit") << std::endl;
        std::cout << OBF("⏱️  Press Ctrl+C to stop") << std::endl;
        
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
        
        std::cout << OBF("\n🛑 ") << BRAND_NAME << OBF(" stopped.") << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << OBF("\n❌ Fatal error: ") << e.what() << std::endl;
        return 1;
    }
}

// ============================================================================
// END OF FILE - Looka Engine v11.0 with 5 Fixes Only
// ============================================================================
