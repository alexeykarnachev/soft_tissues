#pragma once

#include "component/light.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::prefabs {

entt::entity spawn_player(Vector3 position);
entt::entity spawn_light(
    Vector3 position, light::Type type, Color color, float intensity, light::Params params
);

}  // namespace soft_tissues::prefabs
