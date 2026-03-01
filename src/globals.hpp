#pragma once

#include "entt/entity/registry.hpp"
#include "render_state.hpp"

namespace soft_tissues::globals {

enum class GameState {
    EDITOR,
    PLAY,
};

extern entt::registry registry;
extern RenderState RENDER_STATE;

extern float FRAME_DT;
extern GameState GAME_STATE;

// Returns true if the window should close.
bool update();

}  // namespace soft_tissues::globals
