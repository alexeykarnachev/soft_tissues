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

entt::entity spawn_player(Vector2 position) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, {position.x, 0.0, position.y});

    globals::registry.emplace<component::Transform>(entity, transform);
    globals::registry.emplace<component::Player>(entity);

    return entity;
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

entt::entity spawn_ambient_light(Color color, float intensity) {
    return spawn_light(Vector3Zero(), light::Type::AMBIENT, color, intensity, {});
}

}  // namespace soft_tissues::prefabs
