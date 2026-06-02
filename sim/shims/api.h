#pragma once

// Standard headers needed by the warbots framework
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <algorithm>
#include <utility>
#include <unordered_map>

// PROS shims — order matters: rtos before motors (motors needs mutex from std)
#include "pros/rtos.hpp"
#include "pros/motors.hpp"
#include "pros/imu.hpp"
#include "pros/rotation.hpp"
#include "pros/misc.hpp"
#include "pros/screen.hpp"
#include "pros/llemu.hpp"

#ifndef PROS_USE_SIMPLE_NAMES
#define PROS_USE_SIMPLE_NAMES
#endif
