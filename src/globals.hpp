#pragma once

#include "entt/entt.hpp"
#include "render_state.hpp"

namespace soft_tissues::globals {

enum class GameState {
    EDITOR,
    PLAY,
};

extern entt::registry registry;
extern RenderState RENDER_STATE;

extern float TOTAL_TIME;
extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;
extern GameState GAME_STATE;

void update();

}  // namespace soft_tissues::globals
