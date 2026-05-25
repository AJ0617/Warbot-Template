#pragma once
#include "main.h"
//ADD THIS FILE To main.h so you can access it in your .cpp file!!!

//Create and Configure your devices for your subsystem here

//Make sure to put inline at the front so it only creates once!!!!
inline pros::Motor arm(-8);
inline warbots::PIDconfigs armPID = {0.7, 0.001, 0.1, 2000.0};

//Create Functions down here, they will be accessible in your example.cpp file for you to define
void examplePIDFunction(double goal);