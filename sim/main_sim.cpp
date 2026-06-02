#include "main.h"       // resolves to shims/main.h — gets drive, selector, pros shims
#include "world.hpp"
#include "renderer.hpp"
#include "config.hpp"
#include "pros/misc.hpp"   // g_controller_state
#include "pros/llemu.hpp"  // g_btn0_cb, g_btn2_cb
#include <raylib.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <string>
#include <cmath>

// ── Physics thread ───────────────────────────────────────────────────────────

static void physicsThreadFn() {
    using Clock = std::chrono::steady_clock;
    using Dur   = std::chrono::duration<double>;

    auto& world = SimWorld::get();
    auto  next  = Clock::now();
    const auto dt_dur = std::chrono::duration_cast<Clock::duration>(Dur(simcfg::PHYSICS_DT));

    while (world.running) {
        next += dt_dur;
        std::this_thread::sleep_until(next);
        if (!world.paused) {
            std::lock_guard<std::mutex> lk(world.worldMutex);
            world.physicsStep(simcfg::PHYSICS_DT);
        }
    }
}

// ── Mode management ──────────────────────────────────────────────────────────

enum SimMode { SIM_INIT, SIM_READY, SIM_AUTON, SIM_OPCTRL };
static std::atomic<SimMode>  g_mode     {SIM_INIT};
static std::atomic<bool>     g_auton_running {false};
static std::atomic<bool>     g_opctrl_running{false};

static const char* modeString(SimMode m) {
    switch (m) {
        case SIM_INIT:   return "INIT";
        case SIM_READY:  return "READY";
        case SIM_AUTON:  return "AUTONOMOUS";
        case SIM_OPCTRL: return "OPCONTROL";
    }
    return "";
}

// ── Entry point ──────────────────────────────────────────────────────────────

int main() {
    const int WIN_W = 980, WIN_H = 720;
    InitWindow(WIN_W, WIN_H, "Warbot Sim");
    SetTargetFPS(60);

    // Start physics thread
    {
        std::thread pt(physicsThreadFn);
        pt.detach();
    }

    // Run initialize() in a background thread so the render loop can start
    {
        std::thread it([]() {
            initialize();
            g_mode = SIM_READY;
        });
        it.detach();
    }

    Renderer renderer(WIN_W, WIN_H);

    while (!WindowShouldClose()) {
        auto& world = SimWorld::get();

        // ── Keyboard: auton selector ─────────────────────────────
        if (IsKeyPressed(KEY_LEFT)  && pros::lcd::g_btn0_cb) pros::lcd::g_btn0_cb();
        if (IsKeyPressed(KEY_RIGHT) && pros::lcd::g_btn2_cb) pros::lcd::g_btn2_cb();

        // ── Keyboard: run modes ───────────────────────────────────
        if (IsKeyPressed(KEY_A) && !g_auton_running && !g_opctrl_running) {
            g_auton_running = true;
            g_mode = SIM_AUTON;
            std::thread([]() {
                autonomous();
                g_auton_running = false;
                g_mode = SIM_READY;
            }).detach();
        }

        if (IsKeyPressed(KEY_D) && !g_auton_running && !g_opctrl_running) {
            g_opctrl_running = true;
            g_mode = SIM_OPCTRL;
            std::thread([]() {
                opcontrol();
                g_opctrl_running = false;
                g_mode = SIM_READY;
            }).detach();
        }

        // ── Keyboard: reset / pause ───────────────────────────────
        if (IsKeyPressed(KEY_R)) {
            std::lock_guard<std::mutex> lk(world.worldMutex);
            world.resetPose();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            world.paused = !world.paused;
        }

        // ── Controller input (WASD for driving during opcontrol) ──
        // W/S → left Y and right Y (straight drive)
        // A/D → differential turn (added to left, subtracted from right)
        {
            int fwd  = IsKeyDown(KEY_W) ? 127 : IsKeyDown(KEY_S) ? -127 : 0;
            int turn = IsKeyDown(KEY_D) && g_opctrl_running.load() ? 80
                     : IsKeyDown(KEY_A) && g_opctrl_running.load() ? -80
                     : 0;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_LEFT_Y]  = fwd + turn;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_RIGHT_Y] = fwd - turn;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_LEFT_X]  = turn;
            pros::g_controller_state.axes[pros::E_CONTROLLER_ANALOG_RIGHT_X] = turn;
        }

        // ── Gather state for rendering ────────────────────────────
        RenderState rs;
        {
            std::lock_guard<std::mutex> lk(world.worldMutex);
            rs.robotX     = world.trueX;
            rs.robotY     = world.trueY;
            rs.robotAngle = world.trueAngle;
            rs.trail      = world.trail;
        }
        rs.modeName  = modeString(g_mode.load());
        rs.paused    = world.paused.load();

        {
            std::lock_guard<std::mutex> lk(pros::lcd::g_lcd_mutex);
            rs.autonName = pros::lcd::g_lcd_lines.empty()
                         ? "(none)"
                         : pros::lcd::g_lcd_lines.back().text;
        }
        // Prefer the auton selector's current name if available
        if (!selector.autons.empty()) {
            rs.autonName = selector.autons[selector.auton_page_current].name;
        }

        renderer.drawFrame(rs);
    }

    SimWorld::get().running = false;
    CloseWindow();
    return 0;
}
