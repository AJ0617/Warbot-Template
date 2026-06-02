#include "world.hpp"
#include "config.hpp"
#include <cmath>
#include <cstdlib>
#include <set>

void SimWorld::physicsStep(double dt) {
    // Average raw power for left and right wheel groups
    double leftPower = 0, rightPower = 0;
    int leftCount = 0, rightCount = 0;
    for (int port : simcfg::LEFT_PORTS) {
        leftPower += motors[std::abs(port)].rawPower;
        ++leftCount;
    }
    for (int port : simcfg::RIGHT_PORTS) {
        rightPower += motors[std::abs(port)].rawPower;
        ++rightCount;
    }
    if (leftCount  > 0) leftPower  /= leftCount;
    if (rightCount > 0) rightPower /= rightCount;

    // Wheel linear velocities (in/s) at commanded power
    const double wheelCirc   = M_PI * simcfg::WHEEL_DIA;
    const double maxWheelRPM = simcfg::FREE_RPM * simcfg::GEAR_RATIO;
    const double maxLinVel   = maxWheelRPM / 60.0 * wheelCirc;

    double leftVel  = (leftPower  / 127.0) * maxLinVel;
    double rightVel = (rightPower / 127.0) * maxLinVel;

    // Differential-drive kinematics
    double fwdVel    = (leftVel + rightVel) / 2.0;
    double omega_rps = (rightVel - leftVel) / simcfg::TRACK_WIDTH;  // rad/s

    double deltaFwd   = fwdVel    * dt;
    double dTheta_deg = omega_rps * dt * (180.0 / M_PI);

    // Integrate pose (midpoint heading for accuracy)
    double midAngle_rad = (trueAngle + dTheta_deg / 2.0) * (M_PI / 180.0);
    trueX     += deltaFwd * std::sin(midAngle_rad);
    trueY     += deltaFwd * std::cos(midAngle_rad);
    trueAngle += dTheta_deg;
    while (trueAngle >= 360.0) trueAngle -= 360.0;
    while (trueAngle <    0.0) trueAngle += 360.0;

    imuHeading = trueAngle;

    // Update drivetrain motor encoders
    // encPerIn = motor-shaft degrees per inch of wheel travel
    const double encPerIn = 360.0 / (wheelCirc * simcfg::GEAR_RATIO);
    double deltaLeft  = leftVel  * dt;
    double deltaRight = rightVel * dt;

    for (int port : simcfg::LEFT_PORTS) {
        int ap = std::abs(port);
        bool rev = (port < 0);
        // Non-reversed: physEncoderDeg increases when wheel goes forward
        // Reversed    : physEncoderDeg decreases (get_position() negates it → increases)
        motors[ap].physEncoderDeg += (rev ? -1.0 : 1.0) * deltaLeft * encPerIn;
    }
    for (int port : simcfg::RIGHT_PORTS) {
        int ap = std::abs(port);
        bool rev = (port < 0);
        motors[ap].physEncoderDeg += (rev ? -1.0 : 1.0) * deltaRight * encPerIn;
    }

    // Update vertical tracker
    if (simcfg::VERT_TRACKER_PORT != 0) {
        int ap = std::abs(simcfg::VERT_TRACKER_PORT);
        const double trackerCirc = M_PI * simcfg::VERT_TRACKER_DIA;
        rotSensors[ap].centideg += deltaFwd * (36000.0 / trackerCirc);
    }

    // Simulate other (non-drivetrain) motors so PID loops on mechanisms converge
    std::set<int> driveAbs;
    for (int p : simcfg::LEFT_PORTS)  driveAbs.insert(std::abs(p));
    for (int p : simcfg::RIGHT_PORTS) driveAbs.insert(std::abs(p));
    const double otherEncPerSec = simcfg::FREE_RPM * 360.0 / 60.0;
    for (auto& [ap, state] : motors) {
        if (driveAbs.find(ap) == driveAbs.end()) {
            state.physEncoderDeg += (state.rawPower / 127.0) * otherEncPerSec * dt;
        }
    }

    // Breadcrumb trail (every 20 ticks ≈ 100 ms)
    if (++trailTick_ >= 20) {
        trailTick_ = 0;
        trail.push_back({trueX, trueY});
        if (trail.size() > 500) trail.erase(trail.begin());
    }
}

void SimWorld::resetPose(double x, double y, double angle) {
    trueX      = x;
    trueY      = y;
    trueAngle  = angle;
    imuHeading = angle;
    for (auto& [ap, m] : motors)      m.physEncoderDeg = 0;
    for (auto& [ap, r] : rotSensors)  r.centideg = 0;
    trail.clear();
    trailTick_ = 0;
}
