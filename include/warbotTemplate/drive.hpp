#include "main.h"


using namespace warbots;
namespace warbots{
class Drive {
public:

//Constructor
Drive(std::vector<int> leftMotorPorts, std::vector<int> rightMotorPorts, double wheelDiameter, double ratio = 1.0, bool useImu = false, int imuPort = 0)
    : usingImu(useImu), storedImuPort(imuPort), storedWheelDiameter(wheelDiameter), storedGearRatio(ratio)
{
    for (int port : leftMotorPorts) {
        leftMotors.emplace_back(port);
    }
    for (int port : rightMotorPorts) {
        rightMotors.emplace_back(port);
    }
}
//This is to hold the Robot Pose
struct robotPose{
    double x;
    double y;
    double angle;
};

// Read the robot's current position (x = right inches, y = forward inches, angle = degrees CW from forward)
const robotPose& getPose() const { 
    return pose; 
}

//This function prints out the robots pose on the Brain Screen
void testingPose(){
std::string x = warbots::doubleToString(pose.x, 2);
std::string y = warbots::doubleToString(pose.y, 2);
std::string rotation = warbots::doubleToString(pose.angle, 2);
warbots::screenPrint("x:"+ x, 2);
warbots::screenPrint("y:"+ y, 2);
warbots::screenPrint("R:"+ rotation, 2);
}
//TrackWidth is the distance between the center of the left wheel to the center of the right wheel
void setTrackWidth(double inches) { 
    trackWidth = inches; 
}
//This is to configure the IMU if you are using one
void initImu(){
    if (usingImu) {
        imu.emplace(storedImuPort);
    }
}
//This is to configure the vertical Tracking Wheel if you have one
void addVerticalTrackingWheel(int port, double wheelDiameter){
    verticalTracker.emplace(port);
    verticalTracker->reset_position();
    verticalTrackerDiameter = wheelDiameter;
}

//This is to configure the vertical Tracking Wheel if you have one
void addHorizontalTrackingWheel(int port, double wheelDiameter){
    horizontalTracker.emplace(port);
    horizontalTracker->reset_position();
    horizontalTrackerDiameter = wheelDiameter;
}

/* ODOMETRY CONFIG---------------------------------------
Selects which sensors are used to track the robot's position.
Set with setOdomConfig(). Defaults to MOTOR_ENCODERS.
*/
enum odomConfig {
    MOTOR_ENCODERS     = 0,
    IMU_ONLY           = 1,
    VERTICAL_TRACKER   = 2,
    HORIZONTAL_TRACKER = 3,
    BOTH_TRACKERS      = 4,
    IMU_VERTICAL       = 5,
    IMU_HORIZONTAL     = 6,
    IMU_BOTH_TRACKERS  = 7
};

void setOdomConfig(odomConfig config){
    currentOdomConfig = config;
}

// Reset pose and sync all previous-value state so the first updatePose() call produces zero deltas.
void resetPose(double x = 0, double y = 0, double angle = 0) {
    pose = {x, y, angle};
    prevImuHeading = imu.has_value() ? imu->get_heading() : 0;

    double encToInches = (M_PI * storedWheelDiameter * storedGearRatio) / 360.0;
    double leftEnc = 0;
    double rightEnc = 0;
    for (auto& m : leftMotors)  {
        leftEnc  += m.get_position();
    }
    for (auto& m : rightMotors) {
        rightEnc += m.get_position();
    }
    prevLeftInches  = (leftEnc  / leftMotors.size())  * encToInches;
    prevRightInches = (rightEnc / rightMotors.size()) * encToInches;

    if (verticalTracker.has_value()){
        prevVerticalInches = (verticalTracker->get_position() / 36000.0) * M_PI * verticalTrackerDiameter;
    }
    if (horizontalTracker.has_value()){
        prevHorizontalInches = (horizontalTracker->get_position() / 36000.0) * M_PI * horizontalTrackerDiameter;
    }
}

/* updatePose() — call every 10 ms from a pros::Task.
   Coordinate system: +X = right, +Y = forward, angle 0 = facing forward, clockwise positive.
   Each case reads only the sensors required by the active odomConfig.
*/
void updatePose() {
    double deltaTheta_deg = 0;
    double deltaForward = 0;
    double deltaLateral = 0;

    switch (currentOdomConfig) {
        case MOTOR_ENCODERS:
            auto [dL, dR] = motorDeltas();
            deltaForward = (dL + dR) / 2.0;
            if (trackWidth > 0) {
                deltaTheta_deg = (dR - dL) / trackWidth * (180.0 / M_PI);
            }
            break;

        case IMU_ONLY:
            auto [dL, dR] = motorDeltas();
            deltaForward = (dL + dR) / 2.0;
            deltaTheta_deg = imuDelta();
            break;

        case VERTICAL_TRACKER:
            deltaForward = verticalDelta();
            auto [dL, dR] = motorDeltas();
            if (trackWidth > 0) deltaTheta_deg = (dR - dL) / trackWidth * (180.0 / M_PI);
            break;

        case HORIZONTAL_TRACKER:
            auto [dL, dR] = motorDeltas();
            deltaForward = (dL + dR) / 2.0;
            if (trackWidth > 0) deltaTheta_deg = (dR - dL) / trackWidth * (180.0 / M_PI);
            deltaLateral = horizontalDelta();
            break;

        case BOTH_TRACKERS:
            deltaForward = verticalDelta();
            deltaLateral = horizontalDelta();
            auto [dL, dR] = motorDeltas();
            if (trackWidth > 0) deltaTheta_deg = (dR - dL) / trackWidth * (180.0 / M_PI);
            break;

        case IMU_VERTICAL:
            deltaForward = verticalDelta();
            deltaTheta_deg = imuDelta();
            break;

        case IMU_HORIZONTAL:
            auto [dL, dR] = motorDeltas();
            deltaForward = (dL + dR) / 2.0;
            deltaLateral = horizontalDelta();
            deltaTheta_deg = imuDelta();
            break;

        case IMU_BOTH_TRACKERS:
            deltaForward = verticalDelta();
            deltaLateral = horizontalDelta();
            deltaTheta_deg = imuDelta();
            break;

    }

    double avgAngle_rad = (pose.angle + deltaTheta_deg / 2.0) * (M_PI / 180.0);
    pose.x += (deltaForward * std::sin(avgAngle_rad)) + (deltaLateral * std::cos(avgAngle_rad));
    pose.y += (deltaForward * std::cos(avgAngle_rad)) - (deltaLateral * std::sin(avgAngle_rad));
    pose.angle += deltaTheta_deg;

    while (pose.angle >= 360.0) {
        pose.angle -= 360.0;
    }
    while (pose.angle <    0.0) {
        pose.angle += 360.0;
    }
}

/* DRIVER CONTROL---------------------------------------
This code is used to let the user pick the type of driving they would like to use
The user can choose from 5 different configurations:

Tank drive: left joystick controls the left side of the drivetrain and the right joystick controls the right side of the drivetrain
Single Arcade: The left joystick controls both turning and driving
Flipped Single Arcade: The right joystick controls both turning and driving
Split Arcade: The left joystick controls forward and backwards, the right joystick controls turing left and right
Flipped Split Arcade: the left joystick controls turning left and right, the right joystick controls driving forward and back

*/
enum driveControlType{
    TANK = 0,
    SINGLE_ARCADE = 1,
    FLIPPED_SINGLE_ARCADE = 2,
    SPLIT_ARCADE = 3,
    FLIPPED_SPLIT_ARCADE = 4
};

void setDriveType(driveControlType type){
    currentControlType = type;
}


void control(pros::Controller& controller){
    int leftx = controller.get_analog(ANALOG_LEFT_X);
    int lefty = controller.get_analog(ANALOG_LEFT_Y);
    int rightx = controller.get_analog(ANALOG_RIGHT_X);
    int righty = controller.get_analog(ANALOG_RIGHT_Y);

    switch (currentControlType) {
        case TANK:
            moveLeftSide(lefty);
            moveRightSide(righty);
        break;

        case SINGLE_ARCADE:
            moveLeftSide(lefty + leftx);
            moveRightSide(lefty - leftx);
        break;

        case FLIPPED_SINGLE_ARCADE:
            moveLeftSide(righty + rightx);
            moveRightSide(righty - rightx);
        break;

        case SPLIT_ARCADE:
            moveLeftSide(lefty + rightx);
            moveRightSide(lefty - rightx);
        break;

        case FLIPPED_SPLIT_ARCADE:
            moveLeftSide(righty + leftx);
            moveRightSide(righty - leftx);
        break;
    }
}
/* Autonomous Functions
This code is used to control robots driving and turning in Auton
There is PID, Continuous Movement and more!!
*/

void PID_driveInches(double inches){

}

private:

// --- Configuration (set at construction) ---
double storedWheelDiameter = 0;
double storedGearRatio = 1.0;
double trackWidth = 0;
bool usingImu = false;
int storedImuPort = 0;

// --- Hardware ---
std::vector<pros::Motor> leftMotors;
std::vector<pros::Motor> rightMotors;
std::optional<pros::Imu> imu;
std::optional<pros::Rotation> verticalTracker;
double verticalTrackerDiameter = 0;
std::optional<pros::Rotation> horizontalTracker;
double horizontalTrackerDiameter = 0;

// --- State ---
robotPose pose = {0.0, 0.0, 0.0};
pros::motor_brake_mode_e_t currentBrakeMode = pros::E_MOTOR_BRAKE_COAST;
int currentMA = 2500;
odomConfig currentOdomConfig = odomConfig::MOTOR_ENCODERS;
driveControlType currentControlType = SPLIT_ARCADE;

// --- Odometry tracking (previous values for delta calculations) ---
double prevLeftInches = 0;
double prevRightInches = 0;
double prevVerticalInches = 0;
double prevHorizontalInches = 0;
double prevImuHeading = 0;

// Returns {deltaLeft, deltaRight} in inches and advances the stored prev-values.
std::pair<double,double> motorDeltas() {
    double encToInches = (M_PI * storedWheelDiameter * storedGearRatio) / 360.0;
    double lEnc = 0, rEnc = 0;
    for (auto& m : leftMotors)  lEnc += m.get_position();
    for (auto& m : rightMotors) rEnc += m.get_position();
    double lIn = (lEnc / leftMotors.size())  * encToInches;
    double rIn = (rEnc / rightMotors.size()) * encToInches;
    double dL = lIn - prevLeftInches, dR = rIn - prevRightInches;
    prevLeftInches = lIn; prevRightInches = rIn;
    return {dL, dR};
}

// Returns heading delta in degrees (clockwise positive).
double imuDelta() {
    if (!imu.has_value()) {
        return 0;
    }
    double h = imu->get_heading();
    double raw = h - prevImuHeading;
    if (raw >  180) {
        raw -= 360;
    }
    if (raw < -180) {
        raw += 360;
    }
    prevImuHeading = h;
    return raw;
}

// Returns forward distance delta in inches from the vertical tracking wheel.
double verticalDelta() {
    if (!verticalTracker.has_value()) {
        return 0;
    }
    double vIn = (verticalTracker->get_position() / 36000.0) * M_PI * verticalTrackerDiameter;
    double d = vIn - prevVerticalInches;
    prevVerticalInches = vIn;
    return d;
}

// Returns lateral distance delta in inches from the horizontal tracking wheel (positive = rightward).
double horizontalDelta() {
    if (!horizontalTracker.has_value()) {
        return 0;
    }
    double hIn = (horizontalTracker->get_position() / 36000.0) * M_PI * horizontalTrackerDiameter;
    double d = hIn - prevHorizontalInches;
    prevHorizontalInches = hIn;
    return d;
}
//These make the robot sides drive.
void moveLeftSide(int power)  {
    for (auto& m : leftMotors) {
        m.move(power);
    }
}
void moveRightSide(int power) {
    for (auto& m : rightMotors) {
        m.move(power);
    }
}


}; // class Drive
} // namespace warbots
