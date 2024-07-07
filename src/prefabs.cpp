#include "prefabs.hpp"

#include "component/component.hpp"
#include "component/light.hpp"
#include "entt/entity/fwd.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::prefabs {

entt::entity spawn_player(Vector3 position) {
    auto entity = globals::registry.create();
    auto transform = component::Transform(entity, position);

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

}  // namespace soft_tissues::prefabs
