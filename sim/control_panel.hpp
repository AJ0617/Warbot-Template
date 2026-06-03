#pragma once
// Host-side control window: auton selector, run/stop/teleop, input mode, brain
// output, and pose/reset/pause bar. raygui-based; reads pros LCD/HUD shims.
#include "renderer.hpp"

class ControlPanel {
public:
    ControlPanel(int windowW, int windowH);

    // Draw one frame and return any GUI action triggered this frame.
    GUIResult drawFrame(const RenderState& state);

private:
    int w_, h_;
    int guiListScroll_ = 0;

    void applyDarkTheme();
};
