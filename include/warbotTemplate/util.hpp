#pragma once
#include "main.h"
#include "warbotTemplate/logo_data.hpp"

namespace warbots {

    inline void screenPrint(const std::string text, int line = 0,
                            pros::text_format_e_t fmt = pros::E_TEXT_MEDIUM) {
        pros::screen::print(fmt, line, "%s", text.c_str());
    }

    inline void drawLogo(int x_offset = 190, int y_offset = 86) {
        pros::screen::copy_area(
            x_offset,
            y_offset,
            x_offset + LOGO_WIDTH,
            y_offset + LOGO_HEIGHT,
            (uint32_t*)logo_pixels,
            LOGO_WIDTH
        );
    }
}
