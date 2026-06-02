#pragma once
#include <vector>
#include <cmath>

namespace simcfg {
    // Drivetrain ports mirrored from src/main.cpp (negative = reversed)
    static const std::vector<int> LEFT_PORTS  = {10};
    static const std::vector<int> RIGHT_PORTS = {-4};

    static const double WHEEL_DIA   = 4.125; // inches
    static const double GEAR_RATIO  = 0.4;   // driving/driven (motor teeth / wheel teeth)
    static const double TRACK_WIDTH = 12.0;  // inches, left to right wheel center
    static const double FREE_RPM    = 200.0; // V5 motor cartridge RPM

    static const int    IMU_PORT          = 20;
    static const int    VERT_TRACKER_PORT = -1; // abs = 1
    static const double VERT_TRACKER_DIA  = 2.0; // inches

    static const double PHYSICS_DT   = 0.005;  // 5 ms fixed step
    static const double FIELD_SIZE_IN = 144.0; // 144" x 144"

    static const double ROBOT_WIDTH_IN  = 14.0;
    static const double ROBOT_LENGTH_IN = 14.0;
}
