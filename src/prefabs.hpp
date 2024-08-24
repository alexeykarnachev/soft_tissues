#pragma once

#include "component/light.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::prefabs {

entt::entity spawn_entity();
entt::entity spawn_mesh(
    Vector3 position, std::string mesh_key, std::string material_pbr_key
);
entt::entity spawn_light(
    Vector3 position,
    light::LightType light_type,
    Color color,
    float intensity,
    light::Params params
);
entt::entity spawn_spot_light(
    Vector3 position,
    Vector3 direction,
    Color color,
    float intensity,
    Vector3 attenuation,
    float inner_cutoff,
    float outer_cutoff
);
entt::entity spawn_flashlight(Vector3 position);
entt::entity spawn_ambient_light(Color color, float intenstiy);
entt::entity spawn_player(Vector2 position);

}  // namespace soft_tissues::prefabs
