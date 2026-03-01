#pragma once

#include "entt/entt.hpp"
#include "render_state.hpp"

namespace soft_tissues::globals {

enum class GameState {
    EDITOR,
    PLAY,
};

static constexpr int MAX_N_LIGHTS = 8;

static constexpr int MAX_N_SHADOW_MAPS = MAX_N_LIGHTS;
static constexpr int SHADOW_MAP_SIZE = 1024;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_SPEED = 1.0;
static constexpr float PLAYER_CAMERA_SENSITIVITY = 0.0015;

extern entt::registry registry;
extern RenderState RENDER_STATE;

extern float TOTAL_TIME;
extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;
extern GameState GAME_STATE;

void update();

}  // namespace soft_tissues::globals
