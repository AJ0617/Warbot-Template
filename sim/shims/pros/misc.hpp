#pragma once
#include <cstdint>
#include <array>

namespace pros {

enum controller_analog_e_t {
    E_CONTROLLER_ANALOG_LEFT_X  = 0,
    E_CONTROLLER_ANALOG_LEFT_Y  = 1,
    E_CONTROLLER_ANALOG_RIGHT_X = 2,
    E_CONTROLLER_ANALOG_RIGHT_Y = 3,
};

enum controller_digital_e_t {
    E_CONTROLLER_DIGITAL_L1    = 0,
    E_CONTROLLER_DIGITAL_L2    = 1,
    E_CONTROLLER_DIGITAL_R1    = 2,
    E_CONTROLLER_DIGITAL_R2    = 3,
    E_CONTROLLER_DIGITAL_UP    = 4,
    E_CONTROLLER_DIGITAL_DOWN  = 5,
    E_CONTROLLER_DIGITAL_LEFT  = 6,
    E_CONTROLLER_DIGITAL_RIGHT = 7,
    E_CONTROLLER_DIGITAL_X     = 8,
    E_CONTROLLER_DIGITAL_B     = 9,
    E_CONTROLLER_DIGITAL_Y     = 10,
    E_CONTROLLER_DIGITAL_A     = 11,
};

enum controller_id_e_t {
    E_CONTROLLER_MASTER  = 0,
    E_CONTROLLER_PARTNER = 1,
};

struct ControllerState {
    std::array<int,  4>  axes    = {};  // -127..127
    std::array<bool, 12> buttons = {};
};

inline ControllerState g_controller_state;

class Controller {
    controller_id_e_t id_;
public:
    explicit Controller(controller_id_e_t id = E_CONTROLLER_MASTER) : id_(id) {}

    int  get_analog (controller_analog_e_t  ch)  const { return g_controller_state.axes[ch]; }
    bool get_digital(controller_digital_e_t btn) const { return g_controller_state.buttons[btn]; }
    int  set_text(std::uint8_t, std::uint8_t, const char*) { return 0; }
};

} // namespace pros

// Short names (PROS_USE_SIMPLE_NAMES)
#define ANALOG_LEFT_X    pros::E_CONTROLLER_ANALOG_LEFT_X
#define ANALOG_LEFT_Y    pros::E_CONTROLLER_ANALOG_LEFT_Y
#define ANALOG_RIGHT_X   pros::E_CONTROLLER_ANALOG_RIGHT_X
#define ANALOG_RIGHT_Y   pros::E_CONTROLLER_ANALOG_RIGHT_Y

#define DIGITAL_L1       pros::E_CONTROLLER_DIGITAL_L1
#define DIGITAL_L2       pros::E_CONTROLLER_DIGITAL_L2
#define DIGITAL_R1       pros::E_CONTROLLER_DIGITAL_R1
#define DIGITAL_R2       pros::E_CONTROLLER_DIGITAL_R2
#define DIGITAL_UP       pros::E_CONTROLLER_DIGITAL_UP
#define DIGITAL_DOWN     pros::E_CONTROLLER_DIGITAL_DOWN
#define DIGITAL_LEFT     pros::E_CONTROLLER_DIGITAL_LEFT
#define DIGITAL_RIGHT    pros::E_CONTROLLER_DIGITAL_RIGHT
#define DIGITAL_X        pros::E_CONTROLLER_DIGITAL_X
#define DIGITAL_B        pros::E_CONTROLLER_DIGITAL_B
#define DIGITAL_Y        pros::E_CONTROLLER_DIGITAL_Y
#define DIGITAL_A        pros::E_CONTROLLER_DIGITAL_A

#define CONTROLLER_MASTER   pros::E_CONTROLLER_MASTER
#define CONTROLLER_PARTNER  pros::E_CONTROLLER_PARTNER
