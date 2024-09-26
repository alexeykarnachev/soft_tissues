#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace soft_tissues::globals {

enum class GameState {
    EDITOR,
    PLAY,
};

static const int SCREEN_WIDTH = 0;
static const int SCREEN_HEIGHT = 0;

static const int MAX_N_LIGHTS = 8;

static const int MAX_N_SHADOW_MAPS = MAX_N_LIGHTS;
static const int SHADOW_MAP_SIZE = 1024;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_SPEED = 4.0;
static constexpr float PLAYER_CAMERA_SENSETIVITY = 0.0015;

extern entt::registry registry;

extern bool IS_CULL_FACES;

extern bool IS_LIGHT_ENABLED;
extern bool IS_SHADOW_MAP_PASS;

extern float SHADOW_MAP_BIAS;
extern float SHADOW_MAP_MAX_DIST;

extern float WALL_THICKNESS;

extern float TOTAL_TIME;
extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;
extern GameState GAME_STATE;

void update();

}  // namespace soft_tissues::globals
