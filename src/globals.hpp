#pragma once

#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace soft_tissues::globals {

static constexpr int SCREEN_WIDTH = 2560;
static constexpr int SCREEN_HEIGHT = 1440;
static constexpr float DT = 1.0 / 60.0;

static constexpr float PLAYER_HEIGHT = 1.8;
static constexpr float PLAYER_MOVE_SPEED = 2.0;

extern entt::registry registry;

}  // namespace soft_tissues::globals
