#pragma once
// Read-only field view used by the viewer process. Pure raylib — no raygui,
// no pros, no robot code. Draws the field, trail, robot, and a status overlay.
#include "renderer.hpp"

class FieldRenderer {
public:
    FieldRenderer(int windowW, int windowH);

    // Draw one frame. `connected` toggles the "waiting for host" overlay.
    void drawFrame(const RenderState& state, bool connected);

private:
    int    w_, h_;
    int    fieldPx_;
    double pxPerIn_;
    int    fieldLeft_;
    int    fieldTop_;

    float toSX(double x) const;
    float toSY(double y) const;
};
