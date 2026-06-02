#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>
#include <mutex>

namespace pros {

enum text_format_e_t {
    E_TEXT_SMALL         = 0,
    E_TEXT_MEDIUM        = 1,
    E_TEXT_LARGE         = 2,
    E_TEXT_MEDIUM_CENTER = 3,
    E_TEXT_LARGE_CENTER  = 4,
    E_TEXT_SMALL_CENTER  = 5,
};

struct HudLine { int line; std::string text; };

inline std::vector<HudLine> g_hud_lines;
inline std::mutex            g_hud_mutex;

namespace screen {

inline void set_eraser(std::uint32_t) {}
inline void set_pen(std::uint32_t) {}
inline void erase() {}
inline void copy_area(int, int, int, int, std::uint32_t*, int) {}

inline void print(text_format_e_t /*fmt*/, int line, const char* format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    std::lock_guard<std::mutex> lk(g_hud_mutex);
    for (auto& h : g_hud_lines) {
        if (h.line == line) { h.text = buf; return; }
    }
    g_hud_lines.push_back({line, std::string(buf)});
}

} // namespace screen
} // namespace pros
