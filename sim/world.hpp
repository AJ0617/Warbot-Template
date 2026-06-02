#pragma once
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include <utility>
#include <cstdint>

struct MotorState {
    double rawPower      = 0;  // value passed to move(), -127..127
    double physEncoderDeg = 0; // raw physical encoder degrees (not reversed)
};

struct RotState {
    double centideg = 0;
};

class SimWorld {
public:
    // True robot pose (inches / degrees, CW from forward, 0-360)
    double trueX     = 0;
    double trueY     = 0;
    double trueAngle = 0;

    std::unordered_map<int, MotorState> motors;     // keyed by abs port
    std::unordered_map<int, RotState>   rotSensors; // keyed by abs port
    double imuHeading = 0;

    // Breadcrumb trail (x, y inches)
    std::vector<std::pair<double, double>> trail;

    mutable std::mutex worldMutex;
    std::atomic<bool>  paused {false};
    std::atomic<bool>  running{true};

    void physicsStep(double dt);
    void resetPose(double x = 0, double y = 0, double angle = 0);

    static SimWorld& get() {
        static SimWorld inst;
        return inst;
    }

private:
    SimWorld() = default;
    int trailTick_ = 0;
};
