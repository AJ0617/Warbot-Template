# Warbot Simulator — Development Log

## Goal
Build a desktop simulation GUI for the Warbot PROS/VEX project, similar to FRC's **simulateJava** and **AdvantageScope** tools. The sim should let you:
- Visualize the robot driving on a 144"×144" VEX field
- Choose and run autonomous routines from a GUI list
- Drive the robot in opcontrol using keyboard or a real gamepad/joystick
- Stop running routines mid-execution
- See live pose (X, Y, angle) and brain screen output

---

## What Was Built

### Phase 1 — Base Simulation (pre-existing)
The project already had a basic Raylib simulation under `sim/` with:
- Physics engine (`world.cpp`) — differential drive kinematics, IMU/encoder shims
- Renderer (`renderer.cpp`) — field, robot, trail drawn with Raylib
- PROS shims (`sim/shims/pros/`) — fake motors, IMU, rotation sensor, controller, LCD
- Keyboard-only controls (WASD drive, A/D to start modes, LEFT/RIGHT to cycle autons)
- Text-based HUD panel on the right

### Phase 2 — GUI Upgrade (raygui)
**Files changed:** `sim/CMakeLists.txt`, `sim/config.hpp`, `sim/renderer.hpp`, `sim/renderer.cpp`, `sim/main_sim.cpp`, `sim/shims/pros/rtos.hpp`  
**New file:** `sim/gui_impl.cpp`

Added [raygui](https://github.com/raysan5/raygui) (Raylib's official single-header GUI library) via CMake `FetchContent`.

New GUI panel (right side of window) replaced the text HUD with:
- **AUTONOMOUS ROUTINE** — scrollable `ListView` of all registered autons, `RUN AUTONOMOUS` button, `STOP` button, `START OPCONTROL` button
- **INPUT** — toggle group: `KEYBOARD` / `GAMEPAD 1` / `GAMEPAD 2` (auto-detects connected gamepads)
- **POSE** — live X / Y / Angle readout
- **CONTROLS** — `RESET POSE` and `PAUSE/RESUME` buttons
- **OUTPUT** — LCD selector lines + brain `screenPrint` output

**Gamepad support** — Raylib's gamepad API wired to the PROS `g_controller_state` shim:
- Left/right sticks → analog axes (with 0.08 deadzone)
- All 12 buttons (A/B/X/Y, L1/L2/R1/R2, D-pad) → digital buttons

**Stop mechanism** — `pros::g_stop_requested` atomic flag added to `rtos.hpp`. `pros::delay()` now checks the flag every 1ms and throws `SimStopException` when set, cleanly stopping the running auton or opcontrol thread.

### Phase 3 — AdvantageScope Layout
**Files changed:** `sim/config.hpp`, `sim/renderer.hpp`, `sim/renderer.cpp`

Resized to **1440×900** and reorganized to match AdvantageScope's field-dominant layout:

```
┌─────────────────────────────────────────┬──────────────┐
│                                         │ WARBOT SIM   │
│         FIELD (~840px square)           │ AUTON LIST   │
│                                         │ RUN / STOP   │
│                                         │ START OPCTRL │
│                                         │ INPUT        │
│                                         │ OUTPUT       │
├─────────────────────────────────────────┴──────────────┤
│  [MODE PILL]   X  0.00 in   Y  0.00 in   θ  0.0°   [RESET] [PAUSE] │
└─────────────────────────────────────────────────────────────────────┘
```

**AdvantageScope color palette:**
| Role | Hex |
|------|-----|
| Window background | `#0d1117` |
| Panel background | `#161b22` |
| Panel border | `#30363d` |
| Accent / focus | `#58a6ff` (blue) |
| Running / active | `#3fb950` (green) |
| Paused | `#d29922` (orange) |
| Stop / error | `#f85149` (red) |
| Text primary | `#e6edf3` |
| Text secondary | `#8b949e` |

**Bottom status bar** (36px, full width):
- Color-coded mode pill (READY / AUTONOMOUS / OPCONTROL / PAUSED)
- Live pose readout centered
- `RESET POSE` and `PAUSE/RESUME` buttons on the right

---

## How to Build & Run

```bash
# From the sim directory
cd Warbot-Template/sim

# First time (or after moving the folder)
cmake -B build

# Every subsequent build
cmake --build build --config Release

# Run
build/Release/warbot_sim.exe
```

> **Note:** If blocked by Windows Application Control policy, move the project out of the Downloads folder (e.g. to `Documents`) and rebuild.

---

## Key Files

| File | Purpose |
|------|---------|
| `sim/main_sim.cpp` | Main loop, mode management, input handling, GUI action dispatch |
| `sim/renderer.cpp` | All drawing: field, robot, raygui panel, bottom bar |
| `sim/renderer.hpp` | `RenderState`, `GUIAction`/`GUIResult` types |
| `sim/world.cpp` | Physics simulation (differential drive, encoder/IMU update) |
| `sim/config.hpp` | Window size, field size, port assignments |
| `sim/gui_impl.cpp` | `RAYGUI_IMPLEMENTATION` translation unit |
| `sim/shims/pros/` | PROS API shims (motors, IMU, controller, delay, LCD) |
| `src/main.cpp` | Robot code: `initialize()`, `opcontrol()`, `autonomous()` |
| `src/autons.cpp` | Autonomous routine definitions (register via `register_autons()`) |
| `include/auton_selector.hpp` | `AutonSelector` + global `selector` object |
