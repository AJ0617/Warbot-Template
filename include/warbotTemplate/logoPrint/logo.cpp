#include "main.h"
#include "logo_data.hpp"
using namespace warbots;

namespace warbots{


void drawLogo(int x_offset, int y_offset) {  // default: centered 100×100 on 480×272
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
