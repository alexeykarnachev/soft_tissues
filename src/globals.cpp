#include "globals.hpp"

#include "raylib/raylib.h"

namespace soft_tissues::globals {

entt::registry registry;
RenderState RENDER_STATE;

float FRAME_DT = 0.0;
GameState GAME_STATE = GameState::PLAY;

bool update() {
    FRAME_DT = GetFrameTime();
    return WindowShouldClose();
}

}  // namespace soft_tissues::globals
