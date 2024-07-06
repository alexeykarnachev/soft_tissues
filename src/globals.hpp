#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace soft_tissues::globals {

static constexpr int SCREEN_WIDTH = 0;
static constexpr int SCREEN_HEIGHT = 0;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_MOVEMENT_SPEED = 2.0;
static constexpr float PLAYER_CAMERA_SENSETIVITY = 0.0015;

extern entt::registry registry;

enum class GameState {
    EDITOR,
    PLAY,
};

extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;
extern GameState GAME_STATE;

void update();

}  // namespace soft_tissues::globals
