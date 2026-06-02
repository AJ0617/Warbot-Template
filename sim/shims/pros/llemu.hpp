#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <mutex>
#include <functional>

namespace pros {
namespace lcd {

struct LcdLine { int line; std::string text; };

inline std::vector<LcdLine>       g_lcd_lines;
inline std::mutex                 g_lcd_mutex;
inline std::function<void()>      g_btn0_cb;
inline std::function<void()>      g_btn2_cb;

inline bool initialize() { return true; }

inline bool set_text(std::uint8_t line, const char* text) {
    std::lock_guard<std::mutex> lk(g_lcd_mutex);
    for (auto& l : g_lcd_lines) {
        if (l.line == static_cast<int>(line)) { l.text = text; return true; }
    }
    g_lcd_lines.push_back({static_cast<int>(line), std::string(text)});
    return true;
}

inline bool clear_line(std::uint8_t line) {
    std::lock_guard<std::mutex> lk(g_lcd_mutex);
    for (auto& l : g_lcd_lines) {
        if (l.line == static_cast<int>(line)) { l.text.clear(); return true; }
    }
    return true;
}

inline bool clear() {
    std::lock_guard<std::mutex> lk(g_lcd_mutex);
    g_lcd_lines.clear();
    return true;
}

inline void register_btn0_cb(std::function<void()> cb) { g_btn0_cb = std::move(cb); }
inline void register_btn2_cb(std::function<void()> cb) { g_btn2_cb = std::move(cb); }

} // namespace lcd
} // namespace pros
