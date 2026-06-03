#include "control_panel.hpp"
#include "config.hpp"
#include "pros/screen.hpp"
#include "pros/llemu.hpp"
#include <raylib.h>
#include "raygui.h"
#include <cmath>
#include <cstdio>
#include <string>
#include <algorithm>

// ── AdvantageScope palette ────────────────────────────────────────────────────
static const Color BG_WINDOW  = { 13,  17,  23, 255}; // #0d1117
static const Color BG_PANEL   = { 22,  27,  34, 255}; // #161b22
static const Color COL_BORDER = { 48,  54,  61, 255}; // #30363d
static const Color COL_TEXT   = {230, 237, 243, 255}; // #e6edf3
static const Color COL_MUTED  = {139, 148, 158, 255}; // #8b949e
static const Color COL_BLUE   = { 88, 166, 255, 255}; // #58a6ff
static const Color COL_GREEN  = { 63, 185,  80, 255}; // #3fb950
static const Color COL_ORANGE = {210, 153,  34, 255}; // #d29922

static const int BAR_H = simcfg::BOTTOM_BAR_H;

ControlPanel::ControlPanel(int windowW, int windowH) : w_(windowW), h_(windowH) {
    applyDarkTheme();
}

void ControlPanel::applyDarkTheme() {
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR,      0x161b22ff);
    GuiSetStyle(DEFAULT, LINE_COLOR,            0x30363dff);
    GuiSetStyle(DEFAULT, TEXT_SIZE,             14);

    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL,     0x21262dff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL,   0x30363dff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL,     0xe6edf3ff);

    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED,    0x30363dff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED,  0x58a6ffff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED,    0xffffffff);

    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED,    0x1f6febff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED,  0x58a6ffff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED,    0xffffffff);

    GuiSetStyle(DEFAULT, BASE_COLOR_DISABLED,   0x161b22ff);
    GuiSetStyle(DEFAULT, BORDER_COLOR_DISABLED, 0x21262dff);
    GuiSetStyle(DEFAULT, TEXT_COLOR_DISABLED,   0x484f58ff);

    GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED,   0x196c2eff);
    GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0x3fb950ff);
    GuiSetStyle(TOGGLE, TEXT_COLOR_PRESSED,   0xffffffff);
}

GUIResult ControlPanel::drawFrame(const RenderState& s) {
    GUIResult result;

    BeginDrawing();
    ClearBackground(BG_PANEL);

    const int panelH = h_ - BAR_H;
    const int px  = 0;
    const int pw  = w_;
    const int gcx = px + 12;
    const int gcw = pw - 24;

    DrawRectangle(px, 0, pw, panelH, BG_PANEL);

    // Title
    DrawText("WARBOT SIM  ·  HOST", gcx, 10, 17, COL_TEXT);
    DrawLine(gcx, 34, pw - 8, 34, COL_BORDER);

    // ── AUTONOMOUS ROUTINE ────────────────────────────────────────
    int ay = 38;
    GuiGroupBox({(float)(px + 4), (float)ay, (float)(pw - 8), 236.0f}, "AUTONOMOUS ROUTINE");

    std::string autonStr;
    for (size_t i = 0; i < s.autonList.size(); i++) {
        if (i) autonStr += ";";
        autonStr += s.autonList[i];
    }
    int listActive = s.autonSelectedIdx;
    GuiListView({(float)gcx, (float)(ay + 18), (float)gcw, 114.0f},
                autonStr.empty() ? "(no autons)" : autonStr.c_str(),
                &guiListScroll_, &listActive);
    if (listActive >= 0 && listActive != s.autonSelectedIdx && !s.autonList.empty()) {
        if (result.action == GUIAction::NONE)
            result = {GUIAction::SELECT_AUTON, listActive};
    }

    bool anyRunning = s.autonRunning || s.opctrlRunning;
    int btnY = ay + 138;

    if (anyRunning) GuiSetState(STATE_DISABLED);
    if (GuiButton({(float)gcx, (float)btnY, (float)gcw, 26.0f},
                  s.autonRunning ? "AUTONOMOUS RUNNING..." : "RUN AUTONOMOUS")
        && result.action == GUIAction::NONE && !anyRunning)
        result = {GUIAction::RUN_AUTON, 0};
    if (anyRunning) GuiSetState(STATE_NORMAL);

    int stopY = btnY + 32;
    if (!anyRunning) GuiSetState(STATE_DISABLED);
    if (GuiButton({(float)gcx, (float)stopY, (float)gcw, 26.0f}, "STOP")
        && result.action == GUIAction::NONE && anyRunning)
        result = {GUIAction::STOP, 0};
    if (!anyRunning) GuiSetState(STATE_NORMAL);

    int opcY = stopY + 32;
    if (anyRunning) GuiSetState(STATE_DISABLED);
    if (GuiButton({(float)gcx, (float)opcY, (float)gcw, 26.0f},
                  s.opctrlRunning ? "OPCONTROL RUNNING..." : "START OPCONTROL")
        && result.action == GUIAction::NONE && !anyRunning)
        result = {GUIAction::START_OPCTRL, 0};
    if (anyRunning) GuiSetState(STATE_NORMAL);

    // ── INPUT ─────────────────────────────────────────────────────
    int iy = ay + 240;
    GuiGroupBox({(float)(px + 4), (float)iy, (float)(pw - 8), 62.0f}, "INPUT");

    std::string inputStr = "KEYBOARD";
    for (int g = 0; g < s.availableGamepads; g++)
        inputStr += ";GAMEPAD " + std::to_string(g + 1);

    int maxInput    = 1 + s.availableGamepads;
    int inputActive = std::min(s.inputMode, maxInput - 1);
    int prevInput   = inputActive;
    GuiToggleGroup({(float)gcx, (float)(iy + 18), (float)gcw, 26.0f},
                   inputStr.c_str(), &inputActive);
    if (inputActive != prevInput && result.action == GUIAction::NONE)
        result = {GUIAction::SET_INPUT, inputActive};

    // ── OUTPUT (fills rest of panel) ──────────────────────────────
    int outy = iy + 66;
    int outH = panelH - outy - 4;
    GuiGroupBox({(float)(px + 4), (float)outy, (float)(pw - 8), (float)outH}, "OUTPUT");

    int outTextY = outy + 18;
    {
        std::lock_guard<std::mutex> lk(pros::lcd::g_lcd_mutex);
        for (auto& l : pros::lcd::g_lcd_lines) {
            if (outTextY >= outy + outH - 14) break;
            DrawText(l.text.c_str(), gcx, outTextY, 12, COL_GREEN);
            outTextY += 16;
        }
    }
    {
        std::lock_guard<std::mutex> lk(pros::g_hud_mutex);
        for (auto& hud : pros::g_hud_lines) {
            if (hud.text.empty()) continue;
            if (outTextY >= outy + outH - 14) break;
            DrawText(hud.text.c_str(), gcx, outTextY, 12, COL_TEXT);
            outTextY += 16;
        }
    }

    // ── Bottom bar ────────────────────────────────────────────────
    const int barY = h_ - BAR_H;
    DrawRectangle(0, barY, w_, BAR_H, BG_WINDOW);
    DrawLine(0, barY, w_, barY, COL_BORDER);

    Color pillColor; const char* modeLabel;
    if (s.paused)            { pillColor = COL_ORANGE; modeLabel = "PAUSED"; }
    else if (s.autonRunning) { pillColor = COL_GREEN;  modeLabel = "AUTONOMOUS"; }
    else if (s.opctrlRunning){ pillColor = COL_BLUE;   modeLabel = "OPCONTROL"; }
    else                     { pillColor = COL_MUTED;  modeLabel = s.modeName.c_str(); }

    const int pillX = 10, pillY = barY + 6, pillH = BAR_H - 12;
    int pillW = MeasureText(modeLabel, 13) + 16;
    DrawRectangleRounded({(float)pillX,(float)pillY,(float)pillW,(float)pillH}, 0.4f, 6,
                         {pillColor.r, pillColor.g, pillColor.b, 50});
    DrawRectangleRoundedLines({(float)pillX,(float)pillY,(float)pillW,(float)pillH}, 0.4f, 6, pillColor);
    DrawText(modeLabel, pillX + 8, pillY + (pillH - 13) / 2, 13, pillColor);

    // Pose readout under nothing — show compact on the bar right of the pill
    char poseBuf[80];
    snprintf(poseBuf, sizeof(poseBuf), "X %.1f  Y %.1f  th %.0f", s.robotX, s.robotY, s.robotAngle);
    DrawText(poseBuf, pillX + pillW + 12, barY + (BAR_H - 11) / 2, 11, COL_MUTED);

    // Reset / Pause buttons (right)
    const int btnW = 86, btnH = BAR_H - 10, btnY2 = barY + 5;
    int bx = w_ - btnW * 2 - 14;
    if (GuiButton({(float)bx, (float)btnY2, (float)btnW, (float)btnH}, "RESET POSE")
        && result.action == GUIAction::NONE)
        result = {GUIAction::RESET_POSE, 0};
    bx += btnW + 6;
    if (GuiButton({(float)bx, (float)btnY2, (float)btnW, (float)btnH}, s.paused ? "RESUME" : "PAUSE")
        && result.action == GUIAction::NONE)
        result = {GUIAction::TOGGLE_PAUSE, 0};

    EndDrawing();
    return result;
}
