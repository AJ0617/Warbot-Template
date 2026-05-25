#include "main.h"

void examplePIDFunction(double goal) {
    double current = arm.get_position();
    double output  = warbots::calculatePID(current, goal, armPID);
    arm.move((int32_t)output);
}
