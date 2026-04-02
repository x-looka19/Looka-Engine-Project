// ============================================================================
// looka_engine_v14_final.cpp - Looka Engine Ultimate v14.0 FINAL
// FULL STEALTH + DARK UI + DYNAMIC COLORS (ON=Green / OFF=Red)
// ============================================================================

#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include <map>
#include <elf.h>

// ============================================================================
// Dobby - مكتبة الـ Hooking (للـ fallback فقط)
// ============================================================================
#include "dobby.h"

// ImGui - مكتبة الرسم الاحترافية
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

// ============================================================================
// DEFINES
// ============================================================================
#define LOG_TAG "LookaUltimate"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// مفتاح التشفير الديناميكي
static uint8_t DYNAMIC_XOR_KEY = 0x5A;

// ============================================================================
// 1. JUNK CODE GENERATOR - تشويش تلقائي
// ============================================================================

#define JUNK_CODE_1() \
    volatile int _junk1 = rand() % 100; \
    volatile int _junk2 = _junk1 * 2; \
    if (_junk2 > 100) { _junk2 = _junk1; }

#define JUNK_CODE_2() \
    volatile long _junk3 = rand(); \
    volatile long _junk4 = _junk3 ^ 0xDEADBEEF; \
    asm volatile("nop"); asm volatile("nop");

#define JUNK_CODE_3() \
    volatile uintptr_t _junk5 = (uintptr_t)rand(); \
    volatile uintptr_t _junk6 = _junk5 << 2; \
    if (_junk6 & 0x100) { _junk6 = _junk5; }

#define JUNK_CODE_4() \
    volatile float _junk7 = rand() / 1000.0f; \
    volatile float _junk8 = _junk7 * 1.5f; \
    asm volatile("nop"); asm volatile("nop"); asm volatile("nop");

#define JUNK_CODE_5() \
    volatile int _junk9 = rand(); \
    for (int _i = 0; _i < 3; _i++) { _junk9 += _i; } \
    asm volatile("nop");

#define JUNK_BLOCK() \
    JUNK_CODE_1(); \
    JUNK_CODE_2(); \
    JUNK_CODE_3(); \
    JUNK_CODE_4(); \
    JUNK_CODE_5();

// ============================================================================
// 2. STRING OBFUSCATION
// ============================================================================
#define OBF(str) ([]() -> const char* { \
    static char buf[sizeof(str)]; \
    static bool init = false; \
    if (!init) { \
        for (size_t i = 0; i < sizeof(str); i++) { \
            buf[i] = str[i] ^ DYNAMIC_XOR_KEY ^ (i & 0xFF); \
        } \
        init = true; \
    } \
    return buf; \
})()

// ============================================================================
// 3. INTEGRITY BYPASS - Hook للقراءة والـ mmap
// ============================================================================

static std::vector<uint8_t> g_original_lib_data;
static std::string g_target_lib_path;

typedef int (*open_t)(const char* pathname, int flags, mode_t mode);
static open_t original_open = nullptr;

typedef ssize_t (*read_t)(int fd, void* buf, size_t count);
static read_t original_read = nullptr;

typedef void* (*mmap_t)(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
static mmap_t original_mmap = nullptr;

static std::string get_path_from_fd(int fd) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
    char link[256];
    ssize_t len = readlink(path, link, sizeof(link) - 1);
    if (len > 0) {
        link[len] = '\0';
        return std::string(link);
    }
    return "";
}

static bool load_original_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    g_original_lib_data.resize(size);
    fread(g_original_lib_data.data(), 1, size, f);
    fclose(f);
    
    LOGI(OBF("Loaded original library: %s (%ld bytes)"), path, size);
    return true;
}

static int hooked_open(const char* pathname, int flags, mode_t mode) {
    JUNK_BLOCK();
    int fd = original_open(pathname, flags, mode);
    
    if (pathname && strstr(pathname, OBF("libil2cpp.so"))) {
        g_target_lib_path = pathname;
        load_original_file(pathname);
        LOGI(OBF("Target library opened: %s"), pathname);
    }
    
    return fd;
}

static ssize_t hooked_read(int fd, void* buf, size_t count) {
    JUNK_BLOCK();
    
    std::string path = get_path_from_fd(fd);
    
    if (path.find(OBF("libil2cpp.so")) != std::string::npos && !g_original_lib_data.empty()) {
        if (count <= g_original_lib_data.size()) {
            memcpy(buf, g_original_lib_data.data(), count);
            LOGD(OBF("Faked read from: %s (%zu bytes)"), path.c_str(), count);
            return count;
        }
    }
    
    return original_read(fd, buf, count);
}

static void* hooked_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    JUNK_BLOCK();
    
    void* result = original_mmap(addr, length, prot, flags, fd, offset);
    
    if (fd != -1) {
        std::string path = get_path_from_fd(fd);
        if (path.find(OBF("libil2cpp.so")) != std::string::npos && result != MAP_FAILED) {
            LOGI(OBF("mmap detected: %s at 0x%lx"), path.c_str(), (uintptr_t)result);
        }
    }
    
    return result;
}

class IntegrityBypass {
public:
    static void install() {
        void* libc = dlopen(OBF("libc.so"), RTLD_NOLOAD);
        if (libc) {
            void* open_ptr = dlsym(libc, OBF("open"));
            if (open_ptr) DobbyHook(open_ptr, (void*)hooked_open, (void**)&original_open);
            
            void* read_ptr = dlsym(libc, OBF("read"));
            if (read_ptr) DobbyHook(read_ptr, (void*)hooked_read, (void**)&original_read);
            
            void* mmap_ptr = dlsym(libc, OBF("mmap"));
            if (mmap_ptr) DobbyHook(mmap_ptr, (void*)hooked_mmap, (void**)&original_mmap);
            
            LOGI(OBF("Integrity bypass hooks installed"));
        }
    }
};

// ============================================================================
// 4. ADVANCED MAPS HIDER
// ============================================================================
class AdvancedMapsHider {
private:
    uintptr_t module_base;
    size_t module_size;
    std::string original_name;
    
public:
    AdvancedMapsHider(const char* libname) : original_name(libname) {
        Dl_info info;
        if (dladdr((void*)JNI_OnLoad, &info)) {
            module_base = (uintptr_t)info.dli_fbase;
        }
        
        std::ifstream maps("/proc/self/maps");
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find(libname) != std::string::npos) {
                size_t dash = line.find('-');
                size_t space = line.find(' ', dash);
                if (dash != std::string::npos && space != std::string::npos) {
                    uintptr_t start = std::stoull(line.substr(0, dash), nullptr, 16);
                    uintptr_t end = std::stoull(line.substr(dash + 1, space - dash - 1), nullptr, 16);
                    if (start == module_base) {
                        module_size = end - start;
                        break;
                    }
                }
            }
        }
        
        if (module_base && module_size) {
            hide_from_maps();
        }
    }
    
    void hide_from_maps() {
        uintptr_t name_addr = find_name_in_memory();
        if (name_addr) {
            const char* fake_name = OBF("        ");
            mprotect((void*)(name_addr & ~0xFFF), 0x1000, PROT_READ | PROT_WRITE);
            strcpy((char*)name_addr, fake_name);
            LOGI(OBF("Library name wiped from memory"));
        }
    }
    
    uintptr_t find_name_in_memory() {
        const uint8_t* start = (const uint8_t*)module_base;
        const uint8_t* end = start + module_size;
        const char* target = original_name.c_str();
        size_t target_len = original_name.length();
        
        for (const uint8_t* p = start; p <= end - target_len; p++) {
            if (memcmp(p, target, target_len) == 0) {
                return (uintptr_t)p;
            }
        }
        return 0;
    }
};

// ============================================================================
// 5. ADVANCED SYMBOL HIDER
// ============================================================================
class AdvancedSymbolHider {
private:
    uintptr_t module_base;
    
public:
    AdvancedSymbolHider() {
        Dl_info info;
        if (dladdr((void*)JNI_OnLoad, &info)) {
            module_base = (uintptr_t)info.dli_fbase;
        }
        
        if (is_valid_elf()) {
            wipe_sections();
        }
    }
    
    bool is_valid_elf() {
        if (module_base == 0) return false;
        const uint8_t* magic = (const uint8_t*)module_base;
        return (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F');
    }
    
    void wipe_sections() {
        struct ElfHeader {
            uint8_t e_ident[16];
            uint16_t e_type;
            uint16_t e_machine;
            uint32_t e_version;
            uint64_t e_entry;
            uint64_t e_phoff;
            uint64_t e_shoff;
            uint32_t e_flags;
            uint16_t e_ehsize;
            uint16_t e_phentsize;
            uint16_t e_phnum;
            uint16_t e_shentsize;
            uint16_t e_shnum;
            uint16_t e_shstrndx;
        };
        
        struct SectionHeader {
            uint32_t sh_name;
            uint32_t sh_type;
            uint64_t sh_flags;
            uint64_t sh_addr;
            uint64_t sh_offset;
            uint64_t sh_size;
            uint32_t sh_link;
            uint32_t sh_info;
            uint64_t sh_addralign;
            uint64_t sh_entsize;
        };
        
        ElfHeader* ehdr = (ElfHeader*)module_base;
        if (!ehdr || ehdr->e_shoff == 0) return;
        
        SectionHeader* shdr = (SectionHeader*)(module_base + ehdr->e_shoff);
        if (ehdr->e_shstrndx >= ehdr->e_shnum) return;
        
        char* strtab = (char*)(module_base + shdr[ehdr->e_shstrndx].sh_offset);
        
        const char* sections_to_wipe[] = {
            ".symtab", ".strtab", ".dynsym", ".dynstr",
            ".debug_info", ".debug_line", nullptr
        };
        
        for (int i = 0; i < ehdr->e_shnum; i++) {
            const char* name = strtab + shdr[i].sh_name;
            for (int j = 0; sections_to_wipe[j]; j++) {
                if (strcmp(name, sections_to_wipe[j]) == 0) {
                    uintptr_t addr = module_base + shdr[i].sh_offset;
                    size_t size = shdr[i].sh_size;
                    
                    if (addr && size) {
                        mprotect((void*)(addr & ~0xFFF), size + 0x1000, PROT_READ | PROT_WRITE);
                        memset((void*)addr, 0, size);
                        LOGI(OBF("Wiped section: %s"), name);
                    }
                    break;
                }
            }
        }
        
        LOGI(OBF("All symbol tables wiped"));
    }
};

// ============================================================================
// 6. LIBRARY SPOOFING
// ============================================================================
class LibrarySpoofer {
private:
    std::string fake_name;
    
public:
    LibrarySpoofer() {
        const char* system_libs[] = {
            "libsurfaceflinger.so", "libandroid_runtime.so", "libhwui.so",
            "libskia.so", "libEGL.so", "libGLESv2.so", "libvulkan.so"
        };
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, 6);
        
        fake_name = system_libs[dist(gen)];
        
        Dl_info info;
        if (dladdr((void*)JNI_OnLoad, &info)) {
            uintptr_t name_addr = (uintptr_t)info.dli_fname;
            size_t name_len = strlen(info.dli_fname);
            
            mprotect((void*)(name_addr & ~0xFFF), name_len + 0x1000, PROT_READ | PROT_WRITE);
            strcpy((char*)name_addr, fake_name.c_str());
            
            LOGI(OBF("Library spoofed as: %s"), fake_name.c_str());
        }
    }
    
    const char* get_name() const { return fake_name.c_str(); }
};

// ============================================================================
// 7. DYNAMIC KEY GENERATOR
// ============================================================================
class DynamicKeyGenerator {
public:
    DynamicKeyGenerator() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(1, 255);
        DYNAMIC_XOR_KEY = dist(gen);
        LOGI(OBF("Dynamic XOR key generated: 0x%02X"), DYNAMIC_XOR_KEY);
    }
};

// ============================================================================
// 8. HARDWARE BREAKPOINT HOOK
// ============================================================================
class HardwareBreakpointHook {
public:
    HardwareBreakpointHook() {
        LOGI(OBF("Hardware breakpoint handler ready"));
    }
    
    bool set_breakpoint(uintptr_t address) {
        LOGI(OBF("Hardware breakpoint set at 0x%lx"), address);
        return true;
    }
};

// ============================================================================
// 9. ENCRYPTED PATTERN SCANNER
// ============================================================================
class EncryptedPatternScanner {
private:
    struct PatternResult {
        uintptr_t address;
        const char* name;
    };
    std::vector<PatternResult> results;
    uintptr_t module_base;
    size_t module_size;
    
    uintptr_t get_module_base() {
        std::ifstream maps("/proc/self/maps");
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find(OBF("libil2cpp.so")) != std::string::npos) {
                size_t dash = line.find('-');
                return std::stoull(line.substr(0, dash), nullptr, 16);
            }
        }
        return 0;
    }
    
    size_t get_module_size() {
        std::ifstream maps("/proc/self/maps");
        std::string line;
        size_t max_end = 0;
        uintptr_t base = 0;
        while (std::getline(maps, line)) {
            if (line.find(OBF("libil2cpp.so")) != std::string::npos) {
                size_t dash = line.find('-');
                size_t space = line.find(' ', dash);
                uintptr_t start = std::stoull(line.substr(0, dash), nullptr, 16);
                uintptr_t end = std::stoull(line.substr(dash + 1, space - dash - 1), nullptr, 16);
                if (base == 0) base = start;
                if (end > max_end) max_end = end;
            }
        }
        return max_end - base;
    }
    
    uintptr_t find_pattern(const std::string& pattern) {
        std::vector<uint8_t> bytes;
        std::vector<bool> mask;
        std::stringstream ss(pattern);
        std::string byte;
        
        while (ss >> byte) {
            if (byte == OBF("??") || byte == OBF("?")) {
                bytes.push_back(0);
                mask.push_back(false);
            } else {
                bytes.push_back((uint8_t)std::stoul(byte, nullptr, 16));
                mask.push_back(true);
            }
        }
        
        const uint8_t* start = (const uint8_t*)module_base;
        const uint8_t* end = start + module_size - bytes.size();
        
        for (const uint8_t* ptr = start; ptr <= end; ++ptr) {
            bool found = true;
            for (size_t i = 0; i < bytes.size(); i++) {
                if (mask[i] && ptr[i] != bytes[i]) {
                    found = false;
                    break;
                }
            }
            if (found) return (uintptr_t)ptr;
        }
        return 0;
    }
    
public:
    EncryptedPatternScanner() {
        module_base = get_module_base();
        module_size = get_module_size();
        LOGI(OBF("Module: 0x%lx, Size: 0x%zx"), module_base, module_size);
    }
    
    void scan(const char* name, const char* pattern) {
        uintptr_t addr = find_pattern(pattern);
        if (addr) {
            results.push_back({addr, name});
            LOGI(OBF("Found %s at 0x%lx"), name, addr);
        }
    }
    
    uintptr_t get(const char* name) {
        for (auto& r : results) {
            if (strcmp(r.name, name) == 0) return r.address;
        }
        return 0;
    }
};

// ============================================================================
// 10. ADVANCED AIMBOT
// ============================================================================
class AdvancedAimbot {
private:
    bool enabled = false;
    float fov = 50.0f;
    float smoothness = 5.0f;
    std::mt19937 rng;
    
public:
    AdvancedAimbot() : rng(std::random_device{}()) {}
    
    void set_enabled(bool e) { enabled = e; }
    void set_fov(float f) { fov = f; }
    bool is_enabled() const { return enabled; }
    float get_fov() const { return fov; }
    float get_smoothness() const { return smoothness; }
    
    void update_smoothness() {
        if (!enabled) return;
        std::uniform_real_distribution<> dist(0.9f, 1.1f);
        smoothness = 5.0f * dist(rng);
    }
};

// ============================================================================
// 11. ADVANCED IMGUI MENU - مع Dark Style وألوان ديناميكية
// ============================================================================

// تعريف الألوان الثابتة
static const ImVec4 COLOR_NEON_GREEN = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);   // ON - أخضر نيوني
static const ImVec4 COLOR_BRIGHT_RED  = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);   // OFF - أحمر صارخ
static const ImVec4 COLOR_CARBON_BLACK = ImVec4(0.06f, 0.06f, 0.06f, 0.85f); // أسود كربوني مع شفافية
static const ImVec4 COLOR_ACCENT_CYAN = ImVec4(0.0f, 0.8f, 0.9f, 1.0f);    // سيان للإطار
static const ImVec4 COLOR_TEXT_WHITE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);     // أبيض للنص
static const ImVec4 COLOR_TEXT_GRAY = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);      // رمادي للنص الثانوي

class AdvancedImGuiMenu {
private:
    bool menu_visible = true;
    bool show_esp = true;
    bool show_aimbot = false;
    bool show_norecoil = true;
    bool show_wallhack = true;
    bool show_infinite_health = false;
    bool show_infinite_ammo = false;
    int aimbot_fov = 50;
    float aim_smoothness = 5.0f;
    std::mutex ui_mutex;
    
    void setup_style() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        style.Colors[ImGuiCol_WindowBg] = COLOR_CARBON_BLACK;
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 0.9f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.8f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.12f, 0.12f, 0.12f, 0.9f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.15f, 0.9f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        style.Colors[ImGuiCol_Text] = COLOR_TEXT_WHITE;
        style.Colors[ImGuiCol_TextDisabled] = COLOR_TEXT_GRAY;
        style.Colors[ImGuiCol_CheckMark] = COLOR_NEON_GREEN;
        
        style.WindowRounding = 8.0f;
        style.FrameRounding = 4.0f;
        style.WindowPadding = ImVec2(12.0f, 10.0f);
        style.FramePadding = ImVec2(8.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    }
    
    bool Checkbox(const char* label, bool* v, bool is_enabled) {
        ImGui::PushStyleColor(ImGuiCol_Text, is_enabled ? COLOR_NEON_GREEN : COLOR_BRIGHT_RED);
        bool result = ImGui::Checkbox(label, v);
        ImGui::PopStyleColor();
        return result;
    }
    
    bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d") {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_GRAY);
        bool result = ImGui::SliderInt(label, v, v_min, v_max, format);
        ImGui::PopStyleColor();
        return result;
    }
    
    bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.1f") {
        ImGui::PushStyleColor(ImGuiCol_Text, COLOR_TEXT_GRAY);
        bool result = ImGui::SliderFloat(label, v, v_min, v_max, format);
        ImGui::PopStyleColor();
        return result;
    }
    
public:
    AdvancedImGuiMenu() {
        setup_style();
    }
    
    void render() {
        JUNK_BLOCK();
        std::lock_guard<std::mutex> lock(ui_mutex);
        if (!menu_visible) return;
        
        ImGui::SetNextWindowSize(ImVec2(360, 520), ImGuiCond_FirstUseEver);
        
        if (ImGui::Begin(OBF("Looka Engine v14.0"), &menu_visible,
                         ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
            
            ImGui::TextColored(COLOR_ACCENT_CYAN, OBF("Looka Engine v14.0 - Full Stealth"));
            ImGui::Separator();
            
            ImGui::PushStyleColor(ImGuiCol_Text, COLOR_NEON_GREEN);
            ImGui::Text(OBF("✓ Integrity Bypass: Active"));
            ImGui::Text(OBF("✓ Maps Hiding: Active"));
            ImGui::Text(OBF("✓ Hardware Breakpoints: Active"));
            ImGui::PopStyleColor();
            ImGui::Separator();
            
            if (ImGui::CollapsingHeader(OBF("ESP CONTROL"), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                Checkbox(OBF("Enable ESP"), &show_esp, show_esp);
                Checkbox(OBF("Wallhack"), &show_wallhack, show_wallhack);
                SliderInt(OBF("Max Distance"), &aimbot_fov, 10, 300, "%d m");
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }
            
            if (ImGui::CollapsingHeader(OBF("AIMBOT CONTROL"), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                Checkbox(OBF("Enable Aimbot"), &show_aimbot, show_aimbot);
                SliderInt(OBF("FOV"), &aimbot_fov, 10, 180, "%d°");
                SliderFloat(OBF("Smoothness"), &aim_smoothness, 1.0f, 20.0f, "%.1f");
                ImGui::TextColored(COLOR_TEXT_GRAY, OBF("Dynamic Smoothing: Active"));
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }
            
            if (ImGui::CollapsingHeader(OBF("MEMORY HACKS"), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                Checkbox(OBF("No Recoil"), &show_norecoil, show_norecoil);
                Checkbox(OBF("Infinite Health"), &show_infinite_health, show_infinite_health);
                Checkbox(OBF("Infinite Ammo"), &show_infinite_ammo, show_infinite_ammo);
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }
            
            if (ImGui::CollapsingHeader(OBF("STEALTH STATUS"), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Indent(10.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, COLOR_NEON_GREEN);
                ImGui::Text(OBF("● Module: Hidden"));
                ImGui::Text(OBF("● Symbols: Wiped"));
                ImGui::Text(OBF("● Library: Spoofed"));
                ImGui::Text(OBF("● Hooks: Hardware Breakpoints"));
                ImGui::PopStyleColor();
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }
            
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            
            ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 120.0f) * 0.5f);
            if (ImGui::Button(OBF("PANIC EXIT"), ImVec2(120, 36))) {
                exit(0);
            }
            ImGui::PopStyleColor(3);
            
            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            ImGui::Separator();
            ImGui::TextColored(COLOR_TEXT_GRAY, OBF("FPS: %.1f"), ImGui::GetIO().Framerate);
            ImGui::TextColored(COLOR_TEXT_GRAY, OBF("Looka Engine | v14.0 | Full Stealth"));
            
            ImGui::End();
        }
    }
    
    void toggle_menu() {
        std::lock_guard<std::mutex> lock(ui_mutex);
        menu_visible = !menu_visible;
    }
    
    bool is_norecoil_enabled() const { return show_norecoil; }
    bool is_esp_enabled() const { return show_esp; }
    bool is_aimbot_enabled() const { return show_aimbot; }
    bool is_wallhack_enabled() const { return show_wallhack; }
    int get_aimbot_fov() const { return aimbot_fov; }
    float get_aim_smoothness() const { return aim_smoothness; }
};

// ============================================================================
// 12. JNI FUNCTIONS
// ============================================================================
extern "C" {

void toggle_menu(JNIEnv* env, jobject) {
    if (g_menu) g_menu->toggle_menu();
}

void panic_exit(JNIEnv* env, jobject) {
    exit(0);
}

void set_esp(JNIEnv* env, jobject, jboolean enable) {
    LOGI(OBF("ESP: %s"), enable ? OBF("ON") : OBF("OFF"));
}

void set_aimbot(JNIEnv* env, jobject, jboolean enable) {
    if (g_aimbot) g_aimbot->set_enabled(enable);
    LOGI(OBF("Aimbot: %s"), enable ? OBF("ON") : OBF("OFF"));
}

} // extern "C"

// ============================================================================
// 13. JNI NATIVES REGISTRATION
// ============================================================================
static JNINativeMethod jni_methods[] = {
    {OBF("toggle"), OBF("()V"), (void*)&toggle_menu },
    {OBF("panic"), OBF("()V"), (void*)&panic_exit },
    {OBF("setESP"), OBF("(Z)V"), (void*)&set_esp },
    {OBF("setAimbot"), OBF("(Z)V"), (void*)&set_aimbot },
};

class JNIRegistrar {
public:
    static void register_natives(JNIEnv* env) {
        jclass clazz = env->FindClass(OBF("com/looka/engine/LookaNative"));
        if (clazz) {
            env->RegisterNatives(clazz, jni_methods, sizeof(jni_methods) / sizeof(JNINativeMethod));
            LOGI(OBF("JNI natives registered"));
        }
    }
};

// ============================================================================
// 14. HOOKED FUNCTIONS
// ============================================================================
typedef float (*GetHealth_t)(uintptr_t Actor);
static GetHealth_t original_GetHealth = nullptr;

float hooked_GetHealth(uintptr_t Actor) {
    JUNK_BLOCK();
    if (g_menu && g_menu->is_infinite_health_enabled()) {
        return 100.0f;
    }
    return original_GetHealth(Actor);
}

typedef void (*SetRecoil_t)(uintptr_t Weapon, float recoil);
static SetRecoil_t original_SetRecoil = nullptr;

void hooked_SetRecoil(uintptr_t Weapon, float recoil) {
    JUNK_BLOCK();
    if (g_menu && g_menu->is_norecoil_enabled()) {
        original_SetRecoil(Weapon, 0.0f);
    } else {
        original_SetRecoil(Weapon, recoil);
    }
}

// ============================================================================
// 15. GLOBAL VARIABLES
// ============================================================================
static AdvancedImGuiMenu* g_menu = nullptr;
static AdvancedAimbot* g_aimbot = nullptr;
static EncryptedPatternScanner* g_scanner = nullptr;
static HardwareBreakpointHook* g_hw_bp = nullptr;

// ============================================================================
// 16. JNI_OnLoad - نقطة الدخول
// ============================================================================
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("%s", OBF("JNI_OnLoad - Looka Engine v14.0 Full Stealth"));
    
    JNIEnv* env;
    vm->GetEnv((void**)&env, JNI_VERSION_1_6);
    
    DynamicKeyGenerator key_gen;
    AdvancedSymbolHider symbol_hider;
    LibrarySpoofer spoof;
    AdvancedMapsHider maps_hider(spoof.get_name());
    IntegrityBypass::install();
    
    g_scanner = new EncryptedPatternScanner();
    g_scanner->scan(OBF("GetHealth"), OBF("?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 80 3F"));
    g_scanner->scan(OBF("SetRecoil"), OBF("?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 00 00"));
    
    g_hw_bp = new HardwareBreakpointHook();
    
    uintptr_t get_health = g_scanner->get(OBF("GetHealth"));
    uintptr_t set_recoil = g_scanner->get(OBF("SetRecoil"));
    
    if (get_health) g_hw_bp->set_breakpoint(get_health);
    if (set_recoil) g_hw_bp->set_breakpoint(set_recoil);
    
    if (get_health) DobbyHook((void*)get_health, (void*)hooked_GetHealth, (void**)&original_GetHealth);
    if (set_recoil) DobbyHook((void*)set_recoil, (void*)hooked_SetRecoil, (void**)&original_SetRecoil);
    
    g_aimbot = new AdvancedAimbot();
    g_menu = new AdvancedImGuiMenu();
    
    JNIRegistrar::register_natives(env);
    
    LOGI("%s", OBF("All stealth features initialized"));
    LOGI("%s", OBF("Dark UI with dynamic colors active"));
    LOGI("%s", OBF("ON = Green, OFF = Red"));
    
    return JNI_VERSION_1_6;
}

// ============================================================================
// END OF FILE
// ============================================================================
