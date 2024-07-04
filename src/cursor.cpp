#include "mode.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::cursor {

static const float RADIUS = 5.0;
static const Color COLOR = WHITE;

void draw() {
    if (!mode::is_play()) return;

    float x = 0.5 * GetScreenWidth();
    float y = 0.5 * GetScreenHeight();
    DrawCircle(x, y, RADIUS, COLOR);
}

}  // namespace soft_tissues::cursor
