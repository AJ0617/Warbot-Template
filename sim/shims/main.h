#pragma once

#ifndef _PROS_MAIN_H_
#define _PROS_MAIN_H_

#define PROS_USE_SIMPLE_NAMES
#define PROS_USE_LITERALS

#include "api.h"

// Real warbots headers (found on the include/ search path)
#include "autons.h"
#include "auton_selector.hpp"
#include "warbotTemplate/util.hpp"
#include "warbotTemplate/pid.hpp"
#include "warbotTemplate/drive.hpp"
// Example subsystem (found on the src/ search path)
#include "exampleSubsystem/example.hpp"

#ifdef __cplusplus
extern "C" {
#endif
void autonomous(void);
void initialize(void);
void disabled(void);
void competition_initialize(void);
void opcontrol(void);
#ifdef __cplusplus
}
#endif

#endif  // _PROS_MAIN_H_
