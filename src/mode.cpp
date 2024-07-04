#include "mode.hpp"

#include "raylib/raylib.h"

namespace soft_tissues::mode {

static Mode MODE = Mode::PLAY;

Mode get_mode() {
    return MODE;
}

bool is_play() {
    return MODE == Mode::PLAY;
}

void update() {
    if (!IsKeyPressed(KEY_ESCAPE)) return;

    if (MODE == Mode::PLAY) {
        MODE = Mode::EDITOR;
        EnableCursor();
    } else if (MODE == Mode::EDITOR) {
        MODE = Mode::PLAY;
        DisableCursor();
    }
}

}  // namespace soft_tissues::mode
