#include "main.h"


using namespace warbots;
namespace warbots{
class Drive {
public:

//Constructors

Drive(std::vector<int> leftMotorPorts, std::vector<int> rightMotorPorts, int ImuPort, double wheelDiameter, double ticks, double ratio = 1.0)
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

};




}; //Warbots namespace