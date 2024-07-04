#pragma once

namespace soft_tissues::mode {

enum class Mode {
    EDITOR,
    PLAY,
};

Mode get_mode();
bool is_play();

void update();

}  // namespace soft_tissues::mode
