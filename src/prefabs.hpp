#pragma once

#include "component/light.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::prefabs {

entt::entity spawn_entity();
entt::entity spawn_player(Vector2 position);
entt::entity spawn_light(
    Vector3 position, light::Type type, Color color, float intensity, light::Params params
);
entt::entity spawn_ambient_light(Color color, float intenstiy);

}  // namespace soft_tissues::prefabs
