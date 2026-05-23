#include "main.h"

namespace warbots {

    void warbotScreenPrint() {}

    void screenPrint(std::string text, int line, pros::text_format_e_t fmt) {
        pros::screen::print(fmt, line, "%s", text.c_str());
    }

}