#pragma once
#include "main.h"

namespace warbots{

    void warbotScreenPrint();

    void screenPrint(std::string text, int line = 0,
                     pros::text_format_e_t fmt = pros::E_TEXT_MEDIUM);
                     
    void drawLogo(int x_offset = 190, int y_offset = 86);
}
