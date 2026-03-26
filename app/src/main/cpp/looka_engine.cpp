// ============================================================================
// looka_engine_v11_merged.cpp - Looka Engine Ultimate v11.0
// MERGED EDITION - ALL FEATURES FROM test.cpp + v11
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
// [إصلاحات التوافق مع أندرويد NDK]
// ----------------------------------------------------------------------------
extern "C" {
    ssize_t process_vm_readv(pid_t pid, const struct iovec* local_iov, unsigned long liovcnt, 
                           const struct iovec* remote_iov, unsigned long riovcnt, unsigned long flags);
    ssize_t process_vm_writev(pid_t pid, const struct iovec* local_iov, unsigned long liovcnt, 
                            const struct iovec* remote_iov, unsigned long riovcnt, unsigned long flags);
}

// ----------------------------------------------------------------------------
// [تغليف مكتبة OpenSSL لضمان عملها في بيئة C++]
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
#define LOOKA_VERSION "11.0-Merged"
#define LOOKA_OWNER "Looka"
#define LOOKA_SIGNATURE "LK11.0-2026-LOOKA-MERGED"

#define BRAND_NAME_FULL "Looka Engine"
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
// 1. STRING OBFUSCATION (محسن)
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
// 2. LOGGER (مصحح لدعم va_list)
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
            if (!library_name.empty() && line.find(library_name) == std::string::npos)
                continue;

            uintptr_t start, end;
            if (sscanf(line.c_str(), "%lx-%lx", &start, &end) == 2) {
                regions.push_back({start, end});
            }
        }
        return regions;
    }
};

// ============================================================================
// 4. PATTERN SCANNER (من test.cpp)
// ============================================================================
class PatternScanner {
private:
    MemoryManager& _mem;
    std::shared_mutex _scanner_mutex;

public:
    PatternScanner(MemoryManager& mem) : _mem(mem) {}

    std::vector<std::optional<uint8_t>> parsePattern(const std::string& pattern) {
        std::vector<std::optional<uint8_t>> bytes;
        std::stringstream ss(pattern);
        std::string item;
        while (ss >> item) {
            if (item == "?" || item == "??") {
                bytes.push_back(std::nullopt);
            } else {
                bytes.push_back(static_cast<uint8_t>(std::stoul(item, nullptr, 16)));
            }
        }
        return bytes;
    }

    uintptr_t findPattern(uintptr_t start, uintptr_t end, const std::string& pattern) {
        std::unique_lock lock(_scanner_mutex);
        auto signature = parsePattern(pattern);
        size_t size = end - start;
        std::vector<uint8_t> buffer(size);

        if (!_mem.read(start, buffer.data(), size)) return 0;

        for (size_t i = 0; i < size - signature.size(); ++i) {
            bool found = true;
            for (size_t j = 0; j < signature.size(); ++j) {
                if (signature[j].has_value() && buffer[i + j] != *signature[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return start + i;
        }
        return 0;
    }
};

// ============================================================================
// 5. OVERLAY (Framebuffer - من test.cpp)
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

public:
    Overlay() : _fb_fd(-1), _fbp(nullptr) {
        LOG_INFO("Initializing Framebuffer Overlay...");
    }

    ~Overlay() {
        if (_fbp) munmap(_fbp, _screensize);
        if (_fb_fd != -1) close(_fb_fd);
    }

    bool init() {
        _fb_fd = open("/dev/graphics/fb0", O_RDWR);
        if (_fb_fd == -1) {
            _fb_fd = open("/dev/fb0", O_RDWR);
        }

        if (_fb_fd == -1) {
            LOG_ERROR("Cannot open framebuffer device. Root required?");
            return false;
        }

        if (ioctl(_fb_fd, FBIOGET_FSCREENINFO, &_finfo) == -1) {
            LOG_ERROR("Error reading fixed information.");
            return false;
        }

        if (ioctl(_fb_fd, FBIOGET_VSCREENINFO, &_vinfo) == -1) {
            LOG_ERROR("Error reading variable information.");
            return false;
        }

        _screensize = _vinfo.xres * _vinfo.yres * _vinfo.bits_per_pixel / 8;
        _fbp = (char*)mmap(0, _screensize, PROT_READ | PROT_WRITE, MAP_SHARED, _fb_fd, 0);

        if ((long)_fbp == -1) {
            LOG_ERROR("Failed to mmap framebuffer.");
            return false;
        }

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

    void clear() {
        if (_fbp) memset(_fbp, 0, _screensize);
    }
};

// ============================================================================
// 6. ENGINE BRIDGE (Shared Memory - من test.cpp)
// ============================================================================
class EngineBridge {
private:
    int _shm_fd;
    void* _shared_mem;
    const size_t SHM_SIZE = 1024 * 64;

public:
    EngineBridge() : _shm_fd(-1), _shared_mem(nullptr) {}

    bool create() {
        _shm_fd = shm_open("/looka_bridge", O_RDWR | O_CREAT, 0666);
        if (_shm_fd == -1) {
            LOG_ERROR("Failed to create Shared Memory Bridge.");
            return false;
        }

        if (ftruncate(_shm_fd, SHM_SIZE) == -1) return false;

        _shared_mem = mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, _shm_fd, 0);
        if (_shared_mem == MAP_FAILED) return false;

        LOG_INFO("Engine Bridge established at 0x%p", _shared_mem);
        return true;
    }

    template<typename T>
    void sendData(const T& data) {
        if (_shared_mem) {
            memcpy(_shared_mem, &data, sizeof(T));
        }
    }

    ~EngineBridge() {
        if (_shared_mem) munmap(_shared_mem, SHM_SIZE);
        if (_shm_fd != -1) {
            close(_shm_fd);
            shm_unlink("/looka_bridge");
        }
    }
};

// ============================================================================
// 7. SYSTEM MONITOR (من test.cpp)
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
// 8. CRYPTO PROVIDER (من test.cpp)
// ============================================================================
class CryptoProvider {
public:
    static std::string encrypt(const std::string& plain_text, const std::string& key) {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        unsigned char ciphertext[1024];
        int len, ciphertext_len;

        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, 
                          (unsigned char*)key.data(), (unsigned char*)key.data());

        EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plain_text.data(), plain_text.length());
        ciphertext_len = len;

        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);
        return std::string((char*)ciphertext, ciphertext_len);
    }

    static void handleErrors() {
        ERR_print_errors_fp(stderr);
    }
};

// ============================================================================
// 9. ROOT DETECTOR (من v11)
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
        
        const char* su_paths[] = {
            "/system/bin/su", "/system/xbin/su", "/sbin/su",
            "/data/local/bin/su", "/data/adb/magisk/magisk", nullptr
        };
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
// 10. HYBRID MODE SELECTOR (من v11)
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
// 11. GHOST PROCESS (من v11)
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
// 12. ADVANCED ANTI-DETECT (من v11)
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
// 13. FLOATING CONTROL PANEL (من v11)
// ============================================================================
class FloatingControlPanel {
private:
    bool visible;
    bool minimized;
    int width, height;
    std::mutex ui_mutex;
    std::atomic<bool> running{false};
    
public:
    FloatingControlPanel() : visible(true), minimized(false), width(FLOATING_WINDOW_WIDTH), 
                              height(FLOATING_WINDOW_HEIGHT) {}
    
    void start() { running = true; }
    void stop() { running = false; }
    void setVisible(bool vis) { visible = vis; }
    void setMinimized(bool min) { minimized = min; }
};

// ============================================================================
// 14. AIM FEATURES (من v11)
// ============================================================================
struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float px, float py) : x(px), y(py) {}
};

class DynamicAimAssist {
private:
    struct Config {
        bool enabled = true;
        float smoothness = 5.0f;
    } config;
    
public:
    void setEnabled(bool enabled) { config.enabled = enabled; }
    bool isEnabled() const { return config.enabled; }
    
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
    struct Config {
        bool enabled = false;
    } config;
    
public:
    void setEnabled(bool enabled) { config.enabled = enabled; }
    bool isEnabled() const { return config.enabled; }
};

class FlickDetector {
private:
    struct Config {
        bool enabled = true;
    } config;
    
public:
    void setEnabled(bool enabled) { config.enabled = enabled; }
    bool isEnabled() const { return config.enabled; }
};

// ============================================================================
// 15. MAIN ENGINE CLASS (دمج جميع الميزات)
// ============================================================================
class LookaEngineFinal {
private:
    std::unique_ptr<HybridModeSelector> mode_selector;
    ExecutionMode current_mode;
    
    // مكونات من test.cpp
    std::unique_ptr<MemoryManager> memory;
    std::unique_ptr<PatternScanner> scanner;
    std::unique_ptr<Overlay> overlay;
    std::unique_ptr<EngineBridge> bridge;
    
    // مكونات من v11
    std::unique_ptr<GhostProcess> ghost;
    std::unique_ptr<AdvancedAntiDetect> anti_detect;
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
    
    struct FreezeItem {
        uintptr_t address;
        std::vector<uint8_t> value;
        bool active;
    };
    std::vector<FreezeItem> _freeze_list;
    std::mutex _freeze_mutex;
    
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
        while (state.load() == EngineState::RUNNING && overlay && overlay->init()) {
            overlay->clear();
            // رسم ESP هنا
            overlay->drawPixel(540, 1170, {255, 0, 0, 255});
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    
public:
    LookaEngineFinal(pid_t pid, uintptr_t base, const std::string& module) 
        : target_pid(pid), base_address(base), module_name(module) {
        
        mode_selector = std::make_unique<HybridModeSelector>();
        current_mode = mode_selector->getEffectiveMode();
        
        memory = std::make_unique<MemoryManager>(target_pid);
        scanner = std::make_unique<PatternScanner>(*memory);
        overlay = std::make_unique<Overlay>();
        bridge = std::make_unique<EngineBridge>();
        
        ghost = std::make_unique<GhostProcess>();
        anti_detect = std::make_unique<AdvancedAntiDetect>();
        ui_panel = std::make_unique<FloatingControlPanel>();
        aim_assist = std::make_unique<DynamicAimAssist>();
        hardcore_aimbot = std::make_unique<HardcoreAimbot>();
        flick_detector = std::make_unique<FlickDetector>();
        
        LOG_INFO("Engine initialized - Mode: %s", 
                 current_mode == ExecutionMode::ROOT ? "ULTRA" : "STANDARD");
    }
    
    bool initialize() {
        try {
            if (!memory->attach()) {
                LOG_ERROR("Cannot attach to target process");
                return false;
            }
            
            if (!overlay->init()) {
                LOG_WARNING("Overlay initialization skipped");
            }
            
            if (!bridge->create()) {
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
            LOG_ERROR("Initialization failed: %s", e.what());
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
        
        _worker_threads.emplace_back(&LookaEngineFinal::freezeWorker, this);
        _worker_threads.emplace_back(&LookaEngineFinal::overlayWorker, this);
        
        while (state.load() == EngineState::RUNNING) {
            bridge->sendData(state.load());
            
            if (memory->getRegions().empty()) {
                LOG_ERROR("Target process lost");
                state.store(EngineState::ERROR);
                break;
            }
            
            frames++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        for (auto& t : _worker_threads) {
            if (t.joinable()) t.join();
        }
    }
    
    void stop() {
        LOG_INFO("Stopping engine...");
        state.store(EngineState::STOPPED);
        
        if (ui_panel) ui_panel->stop();
        if (memory) memory->detach();
        if (ghost) ghost->restore();
        if (anti_detect) anti_detect->deactivate();
    }
    
    int getCurrentFPS() const { return 60; }
    EngineState getState() const { return state.load(); }
    
    void printStatus() {
        std::cout << "\n╔══════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  Looka Engine v" << LOOKA_VERSION << " - Status Report                       ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  Mode: " << (current_mode == ExecutionMode::ROOT ? "ULTRA (Root)" : "STANDARD") << std::endl;
        std::cout << "║  Frames: " << frames.load() << std::endl;
        std::cout << "║  Ghost Mode: " << (ghost && ghost->isGhost() ? "Active" : "Inactive") << std::endl;
        std::cout << "║  Anti-Detect: " << (anti_detect && anti_detect->isActive() ? "Active" : "Inactive") << std::endl;
        std::cout << "╚══════════════════════════════════════════════════════════════════╝\n" << std::endl;
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
// MAIN
// ============================================================================
static std::atomic<bool> g_running{true};
static std::unique_ptr<LookaEngineFinal> g_engine;

void signalHandler(int sig) {
    LOG_INFO("Signal %d received", sig);
    g_running = false;
    if (g_engine) g_engine->stop();
}

int main() {
    try {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        
        std::cout << "\033[1;36m";
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
        std::cout << "  Looka Engine v" << LOOKA_VERSION << " - Merged Edition" << std::endl;
        std::cout << "\033[0m";
        
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
        
        g_engine = std::make_unique<LookaEngineFinal>(pid, base, module);
        
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
// END OF FILE - Looka Engine v11.0 Merged Edition
// ============================================================================
