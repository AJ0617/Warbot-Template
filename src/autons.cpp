#include "main.h"
#include "autons.h"
#include "auton_selector.hpp"

// To add a new auto: declare it in autons.h, add one line below, implement it.
void register_autons() {
    selector.autons_add({
        {"Red Auto",  redAuto},
        {"Blue Auto", blueAuto},
        {"Skills",    skills},
    });
}

void redAuto() {
    // red alliance autonomous routine
}

void blueAuto() {
    // blue alliance autonomous routine
}

void skills() {
    // skills autonomous routine
}
