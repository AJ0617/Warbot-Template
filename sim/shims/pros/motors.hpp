#pragma once
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <mutex>
#include "../../world.hpp"

namespace pros {

enum motor_brake_mode_e {
    E_MOTOR_BRAKE_COAST = 0,
    E_MOTOR_BRAKE_BRAKE = 1,
    E_MOTOR_BRAKE_HOLD  = 2,
};
using motor_brake_mode_e_t = motor_brake_mode_e;

class Motor {
    int absPort_;
    bool reversed_;
public:
    explicit Motor(int port)
        : absPort_(std::abs(port)), reversed_(port < 0)
    {
        // Ensure entry exists with default-zero state
        (void)SimWorld::get().motors[absPort_];
    }

    void move(int power) const {
        int clamped = power > 127 ? 127 : power < -127 ? -127 : power;
        auto& w = SimWorld::get();
        std::lock_guard<std::mutex> lk(w.worldMutex);
        w.motors[absPort_].rawPower = static_cast<double>(clamped);
    }

    // Returns encoder degrees; negated for reversed motors (matches real PROS behaviour)
    double get_position() const {
        auto& w = SimWorld::get();
        std::lock_guard<std::mutex> lk(w.worldMutex);
        double enc = w.motors[absPort_].physEncoderDeg;
        return reversed_ ? -enc : enc;
    }

    void tare_position() {
        auto& w = SimWorld::get();
        std::lock_guard<std::mutex> lk(w.worldMutex);
        w.motors[absPort_].physEncoderDeg = 0;
    }

    void set_brake_mode(motor_brake_mode_e_t) {}
    void brake() { move(0); }
};

class MotorGroup {
    std::vector<Motor> motors_;
public:
    explicit MotorGroup(std::vector<int> ports) {
        for (int p : ports) motors_.emplace_back(p);
    }

    void move(int power) {
        for (auto& m : motors_) m.move(power);
    }

    double get_position() const {
        if (motors_.empty()) return 0.0;
        double sum = 0;
        for (const auto& m : motors_) sum += m.get_position();
        return sum / static_cast<double>(motors_.size());
    }
};

} // namespace pros
