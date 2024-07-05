#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace soft_tissues::globals {

static constexpr int SCREEN_WIDTH = 2560;
static constexpr int SCREEN_HEIGHT = 1440;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_MOVEMENT_SPEED = 2.0;
static constexpr float PLAYER_CAMERA_SENSETIVITY = 0.0015;

extern entt::registry registry;

extern float FRAME_DT;
extern bool WINDOW_SHOULD_CLOSE;

void update();

}  // namespace soft_tissues::globals
