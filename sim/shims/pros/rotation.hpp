#pragma once
#include <cstdlib>
#include <mutex>
#include "../../world.hpp"

namespace pros {

class Rotation {
    int absPort_;
public:
    explicit Rotation(int port) : absPort_(std::abs(port)) {
        (void)SimWorld::get().rotSensors[absPort_];
    }

    // Returns centidegrees (100 centideg = 1 deg; 36000 centideg = 1 full rotation)
    double get_position() const {
        auto& w = SimWorld::get();
        std::lock_guard<std::mutex> lk(w.worldMutex);
        return w.rotSensors[absPort_].centideg;
    }

    int reset_position() {
        auto& w = SimWorld::get();
        std::lock_guard<std::mutex> lk(w.worldMutex);
        w.rotSensors[absPort_].centideg = 0;
        return 0;
    }

    int set_reversed(bool) { return 0; }
};

} // namespace pros
