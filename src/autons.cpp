#include "main.h"


template <typename Condition>
void waitUntil(Condition condition) {
  while (!condition()) {
    pros::delay(10);
  }
}

// To add a new auto: declare it in autons.h, add one line below, implement it.
void register_autons() {
    selector.autons_add({
        //The first part in brackets that is in quotes is what is shown on the brain
        //The second part of the brackets is the function name so whatever you named it in autos.cpp
        //{"Example Auto", exampleAuto},
        {"Red Auto",  redAuto},
        {"Blue Auto", blueAuto},
        {"Skills",    skills}
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
