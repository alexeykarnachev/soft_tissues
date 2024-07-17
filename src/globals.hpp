#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace soft_tissues::globals {

enum class GameState {
    EDITOR,
    PLAY,
};

static constexpr int SCREEN_WIDTH = 0;
static constexpr int SCREEN_HEIGHT = 0;

static constexpr int WORLD_HEIGHT = 3;
static constexpr int WORLD_N_ROWS = 15;
static constexpr int WORLD_N_COLS = 15;
static constexpr int WORLD_N_TILES = WORLD_N_ROWS * WORLD_N_COLS;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_MOVEMENT_SPEED = 2.0;
static constexpr float PLAYER_CAMERA_SENSETIVITY = 0.0015;

extern float TOTAL_TIME;
extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;
extern GameState GAME_STATE;

extern entt::registry registry;

void update();

}  // namespace soft_tissues::globals
