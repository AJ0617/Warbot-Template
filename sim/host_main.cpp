#include "main.h"       // resolves to shims/main.h — gets drive, selector, pros shims
#include "world.hpp"
#include "control_panel.hpp"
#include "config.hpp"
#include "net/table_server.hpp"
#include "net/protocol.hpp"
#include "pros/misc.hpp"    // g_controller_state
#include "pros/llemu.hpp"   // g_lcd_lines
#include "pros/rtos.hpp"    // g_stop_requested
#include <raylib.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <cmath>
#include <cstdio>

static const int TABLE_PORT = 5805;

// ── Physics thread ───────────────────────────────────────────────────────────

static void physicsThreadFn() {
    using Clock = std::chrono::steady_clock;
    using Dur   = std::chrono::duration<double>;

    auto& world   = SimWorld::get();
    auto  next    = Clock::now();
    const auto dt = std::chrono::duration_cast<Clock::duration>(Dur(simcfg::PHYSICS_DT));

    while (world.running) {
        next += dt;
        std::this_thread::sleep_until(next);
        if (!world.paused) {
            std::lock_guard<std::mutex> lk(world.worldMutex);
            world.physicsStep(simcfg::PHYSICS_DT);
        }
    }
}

// ── Mode state ───────────────────────────────────────────────────────────────

enum SimMode { SIM_INIT, SIM_READY, SIM_AUTON, SIM_OPCTRL };
static std::atomic<SimMode> g_mode         {SIM_INIT};
static std::atomic<bool>    g_auton_running {false};
static std::atomic<bool>    g_opctrl_running{false};
static int                  g_inputMode     = 0;  // 0=keyboard, 1=gamepad0, 2=gamepad1

static const char* modeString(SimMode m) {
    switch (m) {
        case SIM_INIT:   return "INIT";
        case SIM_READY:  return "READY";
        case SIM_AUTON:  return "AUTONOMOUS";
        case SIM_OPCTRL: return "OPCONTROL";
    }
    return "";
}

static void launchAuton() {
    pros::g_stop_requested = false;
    g_auton_running = true;
    g_mode = SIM_AUTON;
    std::thread([]() {
        try { autonomous(); } catch (...) {}
        g_auton_running = false;
        g_mode = SIM_READY;
        pros::g_stop_requested = false;
    }).detach();
}

static void launchOpctrl() {
    pros::g_stop_requested = false;
    g_opctrl_running = true;
    g_mode = SIM_OPCTRL;
    std::thread([]() {
        try { opcontrol(); } catch (...) {}
        g_opctrl_running = false;
        g_mode = SIM_READY;
        pros::g_stop_requested = false;
    }).detach();
}

// ── Gamepad → controller state ───────────────────────────────────────────────

static void updateGamepadInput(int gpIdx) {
    if (!IsGamepadAvailable(gpIdx)) return;

    const float DEAD = 0.08f;
    auto axis = [&](int a) -> int {
        float v = GetGamepadAxisMovement(gpIdx, a);
        return std::abs(v) < DEAD ? 0 : static_cast<int>(v * 127.0f);
    };

    pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_LEFT_X]  =  axis(GAMEPAD_AXIS_LEFT_X);
    pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_LEFT_Y]  = -axis(GAMEPAD_AXIS_LEFT_Y);
    pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_RIGHT_X] =  axis(GAMEPAD_AXIS_RIGHT_X);
    pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_RIGHT_Y] = -axis(GAMEPAD_AXIS_RIGHT_Y);

    auto btn = [&](int rb, pros::controller_digital_e_t pb) {
        pros::g_controller_state.buttons[pb] = IsGamepadButtonDown(gpIdx, rb);
    };
    btn(GAMEPAD_BUTTON_RIGHT_FACE_DOWN,  pros::E_CONTROLLER_DIGITAL_A);
    btn(GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, pros::E_CONTROLLER_DIGITAL_B);
    btn(GAMEPAD_BUTTON_RIGHT_FACE_UP,    pros::E_CONTROLLER_DIGITAL_Y);
    btn(GAMEPAD_BUTTON_RIGHT_FACE_LEFT,  pros::E_CONTROLLER_DIGITAL_X);
    btn(GAMEPAD_BUTTON_LEFT_TRIGGER_1,   pros::E_CONTROLLER_DIGITAL_L1);
    btn(GAMEPAD_BUTTON_LEFT_TRIGGER_2,   pros::E_CONTROLLER_DIGITAL_L2);
    btn(GAMEPAD_BUTTON_RIGHT_TRIGGER_1,  pros::E_CONTROLLER_DIGITAL_R1);
    btn(GAMEPAD_BUTTON_RIGHT_TRIGGER_2,  pros::E_CONTROLLER_DIGITAL_R2);
    btn(GAMEPAD_BUTTON_LEFT_FACE_UP,     pros::E_CONTROLLER_DIGITAL_UP);
    btn(GAMEPAD_BUTTON_LEFT_FACE_DOWN,   pros::E_CONTROLLER_DIGITAL_DOWN);
    btn(GAMEPAD_BUTTON_LEFT_FACE_LEFT,   pros::E_CONTROLLER_DIGITAL_LEFT);
    btn(GAMEPAD_BUTTON_LEFT_FACE_RIGHT,  pros::E_CONTROLLER_DIGITAL_RIGHT);
}

// resolved display label for the bottom pill / viewer overlay
static const char* displayMode(const RenderState& rs) {
    if (rs.paused)            return "PAUSED";
    if (rs.autonRunning)      return "AUTONOMOUS";
    if (rs.opctrlRunning)     return "OPCONTROL";
    return rs.modeName.c_str();
}

// ── Entry point ──────────────────────────────────────────────────────────────

int main() {
    const int WIN_W = 380;
    const int WIN_H = 820;
    InitWindow(WIN_W, WIN_H, "Warbot Sim — Host");
    SetTargetFPS(60);

    TableServer server;
    if (server.start(TABLE_PORT))
        std::printf("[host] table server listening on 127.0.0.1:%d\n", TABLE_PORT);
    else
        std::printf("[host] WARNING: failed to start table server on port %d\n", TABLE_PORT);

    std::thread(physicsThreadFn).detach();
    std::thread([]() {
        initialize();
        g_mode = SIM_READY;
    }).detach();

    ControlPanel panel(WIN_W, WIN_H);
    int broadcastTick = 0;

    while (!WindowShouldClose()) {
        auto& world = SimWorld::get();

        // ── Keyboard shortcuts (R = reset, SPACE = pause) ─────────
        if (IsKeyPressed(KEY_R)) {
            std::lock_guard<std::mutex> lk(world.worldMutex);
            world.resetPose();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            world.paused = !world.paused;
        }

        // ── Controller input ──────────────────────────────────────
        if (g_inputMode == 0) {
            bool opRunning = g_opctrl_running.load();
            int fwd  = IsKeyDown(KEY_W) ? 127 : IsKeyDown(KEY_S) ? -127 : 0;
            int turn = opRunning ? (IsKeyDown(KEY_D) ? 80 : IsKeyDown(KEY_A) ? -80 : 0) : 0;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_LEFT_Y]  = fwd + turn;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_RIGHT_Y] = fwd - turn;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_LEFT_X]  = turn;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_RIGHT_X] = turn;
        } else {
            updateGamepadInput(g_inputMode - 1);
        }

        // ── Count available gamepads ──────────────────────────────
        int availGP = 0;
        for (int i = 0; i < 4; i++)
            if (IsGamepadAvailable(i)) availGP = i + 1; else break;

        // ── Build state snapshot ──────────────────────────────────
        RenderState rs;
        {
            std::lock_guard<std::mutex> lk(world.worldMutex);
            rs.robotX     = world.trueX;
            rs.robotY     = world.trueY;
            rs.robotAngle = world.trueAngle;
            rs.trail      = world.trail;
        }
        rs.modeName      = modeString(g_mode.load());
        rs.paused        = world.paused.load();
        rs.autonRunning  = g_auton_running.load();
        rs.opctrlRunning = g_opctrl_running.load();
        rs.inputMode     = g_inputMode;
        rs.availableGamepads = availGP;

        if (!selector.autons.empty()) {
            rs.autonList.reserve(selector.autons.size());
            for (auto& a : selector.autons)
                rs.autonList.push_back(a.name);
            rs.autonSelectedIdx = selector.auton_page_current;
            rs.autonName        = selector.autons[selector.auton_page_current].name;
        }

        // ── Publish to viewers (~50 Hz: every 1.2 frames; throttle to ~30 Hz) ──
        if (++broadcastTick >= 2) {
            broadcastTick = 0;
            RenderState feed = rs;
            feed.modeName = displayMode(rs);  // resolved label for the viewer overlay
            server.broadcast(simnet::serialize(feed));
        }

        // ── Draw and handle GUI actions ───────────────────────────
        GUIResult res = panel.drawFrame(rs);

        switch (res.action) {
            case GUIAction::RUN_AUTON:
                if (!g_auton_running && !g_opctrl_running) launchAuton();
                break;
            case GUIAction::START_OPCTRL:
                if (!g_auton_running && !g_opctrl_running) launchOpctrl();
                break;
            case GUIAction::STOP:
                pros::g_stop_requested = true;
                break;
            case GUIAction::RESET_POSE: {
                std::lock_guard<std::mutex> lk(world.worldMutex);
                world.resetPose();
                break;
            }
            case GUIAction::TOGGLE_PAUSE:
                world.paused = !world.paused;
                break;
            case GUIAction::SELECT_AUTON:
                if (res.param >= 0 && res.param < (int)selector.autons.size()) {
                    selector.auton_page_current = res.param;
                    selector.selected_auton_print();
                }
                break;
            case GUIAction::SET_INPUT:
                g_inputMode = res.param;
                break;
            default:
                break;
        }
    }

    server.stop();
    SimWorld::get().running = false;
    CloseWindow();
    return 0;
}
