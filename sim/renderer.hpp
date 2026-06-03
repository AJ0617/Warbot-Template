#pragma once
// Shared view/control types passed between the host control panel, the field
// viewer, and the network protocol. Plain structs only — no pros / raylib deps.
#include <string>
#include <vector>
#include <utility>

struct RenderState {
    // Robot pose
    double robotX     = 0;
    double robotY     = 0;
    double robotAngle = 0; // degrees, CW from forward

    std::vector<std::pair<double, double>> trail;

    // Mode / status
    std::string modeName;    // resolved display label (READY/AUTONOMOUS/OPCONTROL/PAUSED)
    std::string autonName;   // currently selected auton name (legacy)
    bool paused      = false;

    // Auton list for GUI
    std::vector<std::string> autonList;
    int  autonSelectedIdx = 0;

    // Running state
    bool autonRunning  = false;
    bool opctrlRunning = false;

    // Input mode: 0 = keyboard, 1 = gamepad 0, 2 = gamepad 1
    int inputMode         = 0;
    int availableGamepads = 0;
};

enum class GUIAction {
    NONE,
    RUN_AUTON,
    START_OPCTRL,
    STOP,
    RESET_POSE,
    TOGGLE_PAUSE,
    SELECT_AUTON,  // param = new auton index
    SET_INPUT,     // param = new input mode (0=kb, 1+=gamepad idx)
};

struct GUIResult {
    GUIAction action = GUIAction::NONE;
    int param        = 0;
};
