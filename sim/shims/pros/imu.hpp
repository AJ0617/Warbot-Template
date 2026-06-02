#pragma once
#include <cstdlib>
#include <mutex>
#include "../../world.hpp"

namespace pros {

class Imu {
    int port_;
public:
    explicit Imu(int port) : port_(std::abs(port)) {}

    double get_heading() const {
        auto& w = SimWorld::get();
        std::lock_guard<std::mutex> lk(w.worldMutex);
        return w.imuHeading;
    }

    // Calibration is instant in the sim
    int reset()            { return 0; }
    bool is_calibrating()  { return false; }
    int set_data_rate(std::uint32_t) { return 0; }
};

} // namespace pros
