#pragma once

#include "component/light.hpp"
#include "entt/entity/fwd.hpp"
#include "pbr.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::prefabs {

entt::entity spawn_entity();
entt::entity spawn_player(Vector2 position);
entt::entity spawn_mesh(Vector3 position, Mesh mesh, pbr::MaterialPBR material);
entt::entity spawn_cube(Vector3 position, pbr::MaterialPBR material);
entt::entity spawn_sphere(Vector3 position, pbr::MaterialPBR material);
entt::entity spawn_light(
    Vector3 position, light::Type type, Color color, float intensity, light::Params params
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

}  // namespace soft_tissues::prefabs
