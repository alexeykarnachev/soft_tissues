#include "globals.hpp"

#include "raylib/raylib.h"

namespace soft_tissues::globals {

entt::registry registry;
RenderState RENDER_STATE;

float FRAME_DT = 0.0;
bool WINDOW_SHOULD_CLOSE = false;
GameState GAME_STATE = GameState::PLAY;

void update() {
    FRAME_DT = GetFrameTime();

    bool is_alt_f4_pressed = IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4);
    WINDOW_SHOULD_CLOSE = (WindowShouldClose() || is_alt_f4_pressed);
}

}  // namespace soft_tissues::globals
