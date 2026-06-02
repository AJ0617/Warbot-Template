#include "renderer.hpp"
#include "config.hpp"
#include "pros/screen.hpp"
#include "pros/llemu.hpp"
#include <raylib.h>
#include <cmath>
#include <cstdio>
#include <string>

// Colours
static const Color COL_FIELD_BG  = {20,  100,  20,  255}; // dark green
static const Color COL_GRID      = {50,  140,  50,  255};
static const Color COL_TRAIL     = {255, 220,  80,  255}; // yellow
static const Color COL_ROBOT     = {200, 50,   50,  255}; // red body
static const Color COL_NOSE      = {255, 255,  255, 255}; // white nose arrow
static const Color COL_HUD_BG    = {30,  30,   30,  255};
static const Color COL_HUD_TEXT  = {220, 220,  220, 255};
static const Color COL_AUTON_TXT = {100, 200, 100,  255};
static const Color COL_MODE_TXT  = {100, 180, 255,  255};
static const Color COL_PAUSED    = {255, 100,  50,  255};

Renderer::Renderer(int windowW, int windowH) : w_(windowW), h_(windowH) {
    const int HUD_W = 240;
    hudLeft_  = windowW - HUD_W;
    int fieldAreaW = hudLeft_;
    int fieldAreaH = windowH;
    fieldPx_  = std::min(fieldAreaW, fieldAreaH) - 20; // 10px margin each side
    pxPerIn_  = static_cast<double>(fieldPx_) / simcfg::FIELD_SIZE_IN;
    fieldLeft_ = (fieldAreaW - fieldPx_) / 2;
    fieldTop_  = (fieldAreaH - fieldPx_) / 2;
}

float Renderer::toSX(double x) const {
    return static_cast<float>(fieldLeft_ + fieldPx_ / 2.0 + x * pxPerIn_);
}

float Renderer::toSY(double y) const {
    // Y is flipped: positive field Y (forward) = upward on screen
    return static_cast<float>(fieldTop_ + fieldPx_ / 2.0 - y * pxPerIn_);
}

void Renderer::drawFrame(const RenderState& s) {
    BeginDrawing();
    ClearBackground(BLACK);

    // ── Field background ──────────────────────────────────────────
    DrawRectangle(fieldLeft_, fieldTop_, fieldPx_, fieldPx_, COL_FIELD_BG);

    // Grid lines every 24" (2 feet)
    const double gridIn = 24.0;
    for (double x = -simcfg::FIELD_SIZE_IN / 2.0; x <= simcfg::FIELD_SIZE_IN / 2.0; x += gridIn) {
        float sx = toSX(x);
        DrawLineV({sx, static_cast<float>(fieldTop_)},
                  {sx, static_cast<float>(fieldTop_ + fieldPx_)}, COL_GRID);
    }
    for (double y = -simcfg::FIELD_SIZE_IN / 2.0; y <= simcfg::FIELD_SIZE_IN / 2.0; y += gridIn) {
        float sy = toSY(y);
        DrawLineV({static_cast<float>(fieldLeft_), sy},
                  {static_cast<float>(fieldLeft_ + fieldPx_), sy}, COL_GRID);
    }

    // Field border
    DrawRectangleLines(fieldLeft_, fieldTop_, fieldPx_, fieldPx_, WHITE);

    // ── Trail ─────────────────────────────────────────────────────
    for (const auto& pt : s.trail) {
        DrawCircleV({toSX(pt.first), toSY(pt.second)}, 2.5f, COL_TRAIL);
    }

    // ── Robot ─────────────────────────────────────────────────────
    float rW = static_cast<float>(simcfg::ROBOT_WIDTH_IN  * pxPerIn_);
    float rH = static_cast<float>(simcfg::ROBOT_LENGTH_IN * pxPerIn_);
    float cx = toSX(s.robotX);
    float cy = toSY(s.robotY);

    // DrawRectanglePro rotates CW around the given origin in screen coords.
    // Our angle is CW from "forward" (up on screen). Screen Y is flipped, so
    // the rectangle's "up" aligns when we pass angle directly.
    Rectangle rec = {cx - rW / 2.0f, cy - rH / 2.0f, rW, rH};
    Vector2   origin = {rW / 2.0f, rH / 2.0f};
    DrawRectanglePro(rec, origin, static_cast<float>(s.robotAngle), COL_ROBOT);

    // Heading arrow: from centre toward "front" of robot
    float ang_rad = static_cast<float>(s.robotAngle * M_PI / 180.0);
    float noseLen = rH * 0.55f;
    Vector2 tip = {cx + noseLen * std::sin(ang_rad),
                   cy - noseLen * std::cos(ang_rad)};
    DrawLineEx({cx, cy}, tip, 3.0f, COL_NOSE);
    DrawCircleV(tip, 4.0f, COL_NOSE);

    // Origin cross
    DrawLineV({toSX(0) - 6, toSY(0)}, {toSX(0) + 6, toSY(0)}, YELLOW);
    DrawLineV({toSX(0), toSY(0) - 6}, {toSX(0), toSY(0) + 6}, YELLOW);

    // ── HUD panel ─────────────────────────────────────────────────
    DrawRectangle(hudLeft_, 0, w_ - hudLeft_, h_, COL_HUD_BG);
    DrawLineV({static_cast<float>(hudLeft_), 0},
              {static_cast<float>(hudLeft_), static_cast<float>(h_)}, GRAY);

    int hx = hudLeft_ + 10;
    int hy = 10;
    const int lineH = 22;

    DrawText("WARBOT SIM", hx, hy, 18, WHITE);  hy += 28;
    DrawLine(hx, hy, w_ - 8, hy, GRAY);          hy += 10;

    // Mode
    Color modeCol = s.paused ? COL_PAUSED : COL_MODE_TXT;
    std::string modeStr = s.paused ? ("PAUSED | " + s.modeName) : s.modeName;
    DrawText(modeStr.c_str(), hx, hy, 16, modeCol); hy += lineH;

    // Auton
    DrawText("Auton:", hx, hy, 14, GRAY); hy += 18;
    DrawText(s.autonName.c_str(), hx + 4, hy, 15, COL_AUTON_TXT); hy += lineH + 4;

    DrawLine(hx, hy, w_ - 8, hy, GRAY); hy += 8;

    // Pose
    DrawText("POSE", hx, hy, 14, GRAY); hy += 18;
    char buf[64];
    snprintf(buf, sizeof(buf), "X:     %.2f in", s.robotX);
    DrawText(buf, hx, hy, 14, COL_HUD_TEXT); hy += lineH;
    snprintf(buf, sizeof(buf), "Y:     %.2f in", s.robotY);
    DrawText(buf, hx, hy, 14, COL_HUD_TEXT); hy += lineH;
    snprintf(buf, sizeof(buf), "Angle: %.1f deg", s.robotAngle);
    DrawText(buf, hx, hy, 14, COL_HUD_TEXT); hy += lineH + 8;

    DrawLine(hx, hy, w_ - 8, hy, GRAY); hy += 8;

    // LCD lines (auton selector)
    {
        std::lock_guard<std::mutex> lk(pros::lcd::g_lcd_mutex);
        DrawText("LCD:", hx, hy, 14, GRAY); hy += 18;
        for (auto& l : pros::lcd::g_lcd_lines) {
            DrawText(l.text.c_str(), hx, hy, 13, COL_AUTON_TXT);
            hy += 18;
        }
    }
    hy += 6;
    DrawLine(hx, hy, w_ - 8, hy, GRAY); hy += 8;

    // Screen HUD lines (from screenPrint)
    {
        std::lock_guard<std::mutex> lk(pros::g_hud_mutex);
        DrawText("BRAIN:", hx, hy, 14, GRAY); hy += 18;
        for (auto& h : pros::g_hud_lines) {
            if (h.text.empty()) continue;
            DrawText(h.text.c_str(), hx, hy, 12, COL_HUD_TEXT);
            hy += 16;
            if (hy > h_ - 80) break;
        }
    }

    // Key legend at the bottom
    hy = h_ - 100;
    DrawLine(hx, hy, w_ - 8, hy, GRAY); hy += 6;
    DrawText("KEYS",                   hx, hy, 13, GRAY); hy += 16;
    DrawText("<-/-> Auton cycle",     hx, hy, 11, GRAY); hy += 14;
    DrawText("A  Run autonomous",     hx, hy, 11, GRAY); hy += 14;
    DrawText("D  Start opcontrol",    hx, hy, 11, GRAY); hy += 14;
    DrawText("WASD  Drive (opctrl)",  hx, hy, 11, GRAY); hy += 14;
    DrawText("R  Reset  SPC Pause",   hx, hy, 11, GRAY);

    EndDrawing();
}
