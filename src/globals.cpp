#include "globals.hpp"

#include "raylib/raylib.h"

namespace soft_tissues::globals {

entt::registry registry;
RenderState RENDER_STATE;

float FRAME_DT = 0.0;
GameState GAME_STATE = GameState::PLAY;

bool update() {
    FRAME_DT = GetFrameTime();

    bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    return WindowShouldClose() || is_alt_f4_pressed;
}

}  // namespace soft_tissues::globals
