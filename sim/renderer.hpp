#pragma once
#include <string>
#include <vector>
#include <utility>

struct RenderState {
    double robotX;
    double robotY;
    double robotAngle; // degrees, CW from forward
    std::vector<std::pair<double, double>> trail;
    std::string modeName;
    std::string autonName;
    bool paused;
};

class Renderer {
public:
    Renderer(int windowW, int windowH);

    // Draw one frame. Call from the main thread each iteration of the raylib loop.
    void drawFrame(const RenderState& state);

private:
    int w_, h_;         // window dimensions
    int fieldPx_;       // pixel size of the square field area
    double pxPerIn_;    // pixels per inch
    int fieldLeft_;     // left edge of field on screen
    int fieldTop_;      // top edge of field on screen
    int hudLeft_;       // left edge of HUD panel

    // Convert field coordinates (inches) to screen pixels.
    // Origin (0,0) = centre of the field area.
    // +X = right, +Y = up (flipped for screen Y-down convention).
    float toSX(double x) const;
    float toSY(double y) const;
};
