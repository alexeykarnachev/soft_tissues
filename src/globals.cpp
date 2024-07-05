#include "globals.hpp"

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::globals {

entt::registry registry;

float FRAME_DT = 0.0;
bool WINDOW_SHOULD_CLOSE = false;
GameState GAME_STATE = GameState::EDITOR;

void update() {
    // FRAME_DT
    FRAME_DT = GetFrameTime();

    // WINDOW_SHOULD_CLOSE
    bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    WINDOW_SHOULD_CLOSE = (WindowShouldClose() || is_alt_f4_pressed);
}

}  // namespace soft_tissues::globals
