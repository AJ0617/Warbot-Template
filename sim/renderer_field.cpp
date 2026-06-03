#include "field_renderer.hpp"
#include "config.hpp"
#include <raylib.h>
#include <cmath>
#include <cstdio>
#include <algorithm>

// ── AdvantageScope palette ────────────────────────────────────────────────────
static const Color BG_WINDOW    = { 13,  17,  23, 255}; // #0d1117
static const Color COL_BORDER   = { 48,  54,  61, 255}; // #30363d
static const Color COL_TEXT     = {230, 237, 243, 255}; // #e6edf3
static const Color COL_MUTED    = {139, 148, 158, 255}; // #8b949e
static const Color COL_FIELD_BG = { 22,  38,  22, 255}; // #162616
static const Color COL_GRID     = { 29,  58,  29, 255}; // #1d3a1d
static const Color COL_TRAIL    = {255, 220,  80, 255};
static const Color COL_ROBOT    = {200,  50,  50, 255};
static const Color COL_NOSE     = {255, 255, 255, 255};

static const int STATUS_H = 36;

FieldRenderer::FieldRenderer(int windowW, int windowH) : w_(windowW), h_(windowH) {
    int areaW = windowW;
    int areaH = windowH - STATUS_H;
    fieldPx_   = std::min(areaW, areaH) - 20;
    pxPerIn_   = static_cast<double>(fieldPx_) / simcfg::FIELD_SIZE_IN;
    fieldLeft_ = (areaW - fieldPx_) / 2;
    fieldTop_  = (areaH - fieldPx_) / 2;
}

float FieldRenderer::toSX(double x) const {
    return static_cast<float>(fieldLeft_ + fieldPx_ / 2.0 + x * pxPerIn_);
}
float FieldRenderer::toSY(double y) const {
    return static_cast<float>(fieldTop_ + fieldPx_ / 2.0 - y * pxPerIn_);
}

void FieldRenderer::drawFrame(const RenderState& s, bool connected) {
    BeginDrawing();
    ClearBackground(BG_WINDOW);

    // ── Field background ──────────────────────────────────────────
    DrawRectangle(fieldLeft_, fieldTop_, fieldPx_, fieldPx_, COL_FIELD_BG);

    const double gridIn = 24.0;
    for (double x = -simcfg::FIELD_SIZE_IN / 2.0; x <= simcfg::FIELD_SIZE_IN / 2.0; x += gridIn) {
        float sx = toSX(x);
        DrawLineV({sx, (float)fieldTop_}, {sx, (float)(fieldTop_ + fieldPx_)}, COL_GRID);
    }
    for (double y = -simcfg::FIELD_SIZE_IN / 2.0; y <= simcfg::FIELD_SIZE_IN / 2.0; y += gridIn) {
        float sy = toSY(y);
        DrawLineV({(float)fieldLeft_, sy}, {(float)(fieldLeft_ + fieldPx_), sy}, COL_GRID);
    }
    DrawRectangleLines(fieldLeft_, fieldTop_, fieldPx_, fieldPx_, COL_BORDER);

    // ── Trail ─────────────────────────────────────────────────────
    for (const auto& pt : s.trail)
        DrawCircleV({toSX(pt.first), toSY(pt.second)}, 2.5f, COL_TRAIL);

    // ── Robot ─────────────────────────────────────────────────────
    float rW = (float)(simcfg::ROBOT_WIDTH_IN  * pxPerIn_);
    float rH = (float)(simcfg::ROBOT_LENGTH_IN * pxPerIn_);
    float cx = toSX(s.robotX);
    float cy = toSY(s.robotY);
    Rectangle rec    = {cx - rW / 2.0f, cy - rH / 2.0f, rW, rH};
    Vector2   origin = {rW / 2.0f, rH / 2.0f};
    DrawRectanglePro(rec, origin, (float)s.robotAngle, COL_ROBOT);

    float ang_rad = (float)(s.robotAngle * M_PI / 180.0);
    float noseLen = rH * 0.55f;
    Vector2 tip = {cx + noseLen * std::sin(ang_rad), cy - noseLen * std::cos(ang_rad)};
    DrawLineEx({cx, cy}, tip, 3.0f, COL_NOSE);
    DrawCircleV(tip, 4.0f, COL_NOSE);

    // Origin cross
    DrawLineV({toSX(0) - 6, toSY(0)}, {toSX(0) + 6, toSY(0)}, YELLOW);
    DrawLineV({toSX(0), toSY(0) - 6}, {toSX(0), toSY(0) + 6}, YELLOW);

    // ── Status bar ────────────────────────────────────────────────
    int barY = h_ - STATUS_H;
    DrawRectangle(0, barY, w_, STATUS_H, BG_WINDOW);
    DrawLine(0, barY, w_, barY, COL_BORDER);

    const char* mode = s.modeName.empty() ? "—" : s.modeName.c_str();
    DrawText(mode, 10, barY + (STATUS_H - 13) / 2, 13, COL_TEXT);

    char poseBuf[96];
    snprintf(poseBuf, sizeof(poseBuf), "X %.2f in   Y %.2f in   theta %.1f deg",
             s.robotX, s.robotY, s.robotAngle);
    int poseW = MeasureText(poseBuf, 13);
    DrawText(poseBuf, w_ - poseW - 10, barY + (STATUS_H - 13) / 2, 13, COL_MUTED);

    // ── Connection overlay ────────────────────────────────────────
    if (!connected) {
        DrawRectangle(0, 0, w_, h_, {13, 17, 23, 180});
        const char* msg = "Connecting to host  127.0.0.1 ...";
        int mw = MeasureText(msg, 20);
        DrawText(msg, (w_ - mw) / 2, h_ / 2 - 10, 20, COL_TEXT);
    }

    EndDrawing();
}
