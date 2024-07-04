#include "prefabs.hpp"

#include "component/component.hpp"
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

}  // namespace soft_tissues::prefabs
