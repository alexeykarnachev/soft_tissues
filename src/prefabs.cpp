#include "prefabs.hpp"

#include "component/component.hpp"
#include "component/light.hpp"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "resources.hpp"

namespace soft_tissues::prefabs {

entt::entity spawn_entity() {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, {0.0, 0.0, 0.0});

    globals::registry.emplace<component::Transform>(entity, transform);

    return entity;
}

entt::entity spawn_player(Vector2 position) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, {position.x, 0.0, position.y});

    globals::registry.emplace<component::Transform>(entity, transform);
    globals::registry.emplace<component::Player>(entity);

    return entity;
}

entt::entity spawn_mesh(Vector3 position, Mesh mesh, pbr::MaterialPBR material) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, position);
    auto my_mesh = component::MyMesh(entity, mesh, material);

    globals::registry.emplace<component::Transform>(entity, transform);
    globals::registry.emplace<component::MyMesh>(entity, my_mesh);

    return entity;
}

entt::entity spawn_cube(Vector3 position, pbr::MaterialPBR material) {
    return spawn_mesh(position, resources::CUBE_MESH, material);
}

entt::entity spawn_sphere(Vector3 position, pbr::MaterialPBR material) {
    return spawn_mesh(position, resources::SPHERE_MESH, material);
}

entt::entity spawn_light(
    Vector3 position, light::Type type, Color color, float intensity, light::Params params
) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, position);

    light::Light light(entity, type, color, intensity, params);

    globals::registry.emplace<component::Transform>(entity, transform);
    globals::registry.emplace<component::Light>(entity, light);

    return entity;
}

entt::entity spawn_flashlight(Vector3 position) {
    static const light::Type type = light::Type::SPOT;
    static const Color color = {255, 255, 220, 255};
    static const auto intensity = 50.0;

    static const Vector3 attenuation = {1.0, 1.2, 0.2};
    static const float inner_cutoff = 0.95;
    static const float outer_cutoff = 0.80;

    light::Params params
        = {.spot = {
               .attenuation = attenuation,
               .inner_cutoff = inner_cutoff,
               .outer_cutoff = outer_cutoff,
           }};
    auto entity = spawn_light(position, type, color, intensity, params);
    globals::registry.emplace<component::Flashlight>(entity);

    return entity;
}

entt::entity spawn_ambient_light(Color color, float intensity) {
    return spawn_light(Vector3Zero(), light::Type::AMBIENT, color, intensity, {});
}

}  // namespace soft_tissues::prefabs
