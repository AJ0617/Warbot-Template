#pragma once
#include "main.h"
#include "warbotTemplate/logo_data.hpp"

namespace warbots {

    inline void screenPrint(const std::string text, int line = 0,
                            pros::text_format_e_t fmt = pros::E_TEXT_MEDIUM) {
        pros::screen::print(fmt, line, "%s", text.c_str());
    }

    inline void drawBackground(uint32_t color = 0x1A1A2E) {
        pros::screen::set_eraser(color);
        pros::screen::erase();
    }

    inline void drawLogo(int x_offset = 190, int y_offset = 86, uint32_t bg_color = 555555) {
        static uint32_t modified_pixels[LOGO_HEIGHT][LOGO_WIDTH];
        static bool initialized = false;
        if (!initialized) {
            for (int y = 0; y < LOGO_HEIGHT; y++) {
                for (int x = 0; x < LOGO_WIDTH; x++) {
                    modified_pixels[y][x] = (logo_pixels[y][x] == 0x00FFFFFF) ? bg_color : logo_pixels[y][x];
                }
            }
            initialized = true;
        }
        pros::screen::copy_area(
            x_offset,
            y_offset,
            x_offset + LOGO_WIDTH,
            y_offset + LOGO_HEIGHT,
            (uint32_t*)modified_pixels,
            LOGO_WIDTH
        );
    }
}
