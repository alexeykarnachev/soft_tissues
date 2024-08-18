#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace soft_tissues::globals {

enum class GameState {
    EDITOR,
    PLAY,
};

struct RenderOptions {
    bool is_light_enabled = true;
    bool is_shadow_map_pass = false;

    float shadow_map_bias = -0.1;
    float shadow_map_max_dist = 100.0;
};

static constexpr int SCREEN_WIDTH = 0;
static constexpr int SCREEN_HEIGHT = 0;

static constexpr int SHADOW_MAP_SIZE = 2048;

// TODO: Refactor shadow maps management (e.g 6 maps for point light, etc)
static constexpr int MAX_N_SHADOW_MAPS = 64;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_MOVEMENT_SPEED = 2.0;
static constexpr float PLAYER_CAMERA_SENSETIVITY = 0.0015;

extern float TOTAL_TIME;
extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;
extern GameState GAME_STATE;
extern RenderOptions RENDER_OPTIONS;

extern entt::registry registry;

void update();

}  // namespace soft_tissues::globals
