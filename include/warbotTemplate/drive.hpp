#include "main.h"


using namespace warbots;
namespace warbots{
class Drive {
public:

//Constructors

Drive(std::vector<int> leftMotorPorts, std::vector<int> rightMotorPorts, int ImuPort, double wheelDiameter, double ratio = 1.0)
    : imu(ImuPort)
{
    for (int port : leftMotorPorts) {
        leftMotors.emplace_back(port);
    }
    for (int port : rightMotorPorts) {
        rightMotors.emplace_back(port);
    }
}



pros::motor_brake_mode_e_t currentBrakeMode = pros::E_MOTOR_BRAKE_COAST;

//Current Milliamps
int currentMA = 2500;

//Motors for left side
std::vector<pros::Motor> leftMotors;

//Motors for Right side
std::vector<pros::Motor> rightMotors;

//Imu Port
pros::Imu imu;

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

driveControlType currentControlType = SPLIT_ARCADE;

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

private:
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