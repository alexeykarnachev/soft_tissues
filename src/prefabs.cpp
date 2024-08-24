#include "prefabs.hpp"

#include "component/component.hpp"
#include "component/light.hpp"
#include "entt/entity/fwd.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::prefabs {

entt::entity spawn_entity() {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, {0.0, 0.0, 0.0});

    globals::registry.emplace<component::Transform>(entity, transform);

    return entity;
}

entt::entity spawn_mesh(
    Vector3 position, std::string mesh_key, std::string material_pbr_key
) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, position);
    auto my_mesh = component::MyMesh(entity, mesh_key, material_pbr_key);

    globals::registry.emplace<component::Transform>(entity, transform);
    globals::registry.emplace<component::MyMesh>(entity, my_mesh);

    return entity;
}

entt::entity spawn_light(
    Vector3 position,
    light::LightType light_type,
    Color color,
    float intensity,
    light::Params params
) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, position);

    light::Light light(entity, light_type, color, intensity, params);

    globals::registry.emplace<component::Transform>(entity, transform);
    globals::registry.emplace<component::Light>(entity, light);

    return entity;
}

entt::entity spawn_spot_light(
    Vector3 position,
    Vector3 direction,
    Color color,
    float intensity,
    Vector3 attenuation,
    float inner_cutoff,
    float outer_cutoff
) {
    direction = Vector3Normalize(direction);

    light::Params params
        = {.spot = {
               .attenuation = attenuation,
               .inner_cutoff = inner_cutoff,
               .outer_cutoff = outer_cutoff,
           }};

    auto entity = spawn_light(position, light::LightType::SPOT, color, intensity, params);
    auto &tr = globals::registry.get<component::Transform>(entity);
    tr.set_forward(direction);

    return entity;
}

entt::entity spawn_ambient_light(Color color, float intensity) {
    return spawn_light(Vector3Zero(), light::LightType::AMBIENT, color, intensity, {});
}

entt::entity spawn_player(Vector2 position) {
    entt::entity player;
    entt::entity flashlight;

    // player
    {
        auto entity = globals::registry.create();
        auto transform = component::Transform(entity, {position.x, 0.0, position.y});

        globals::registry.emplace<component::Transform>(entity, transform);
        globals::registry.emplace<component::Player>(entity);

        player = entity;
    }

    // flashlight
    {
        static const Vector3 position = {0.0, globals::PLAYER_HEIGHT, 0.0};

        static const light::LightType type = light::LightType::SPOT;
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

        flashlight = entity;
    }

    // attach flashlight to player
    globals::registry.emplace<component::Parent>(flashlight, player);

    return player;
}

}  // namespace soft_tissues::prefabs
