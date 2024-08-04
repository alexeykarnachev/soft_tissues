#include "../component/component.hpp"
#include "../globals.hpp"
#include "../prefabs.hpp"
#include "editor.hpp"
#include "entt/entity/entity.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include <cassert>
#include <cstdio>

namespace soft_tissues::editor::entities_editor {

entt::entity ENTITY = entt::null;

static void update_and_draw_transformation() {
    assert(ENTITY != entt::null);

    auto &tr = globals::registry.get<component::Transform>(ENTITY);

    ImGui::SeparatorText("Transformation");

    {
        static float speed = 0.1;
        float *v = (float *)&tr.position;

        ImGui::PushID(&tr.position);
        ImGui::DragFloat3("Position", v, speed);
        ImGui::PopID();
    }

    {
        static float speed = PI / 16.0;
        static float min = -2.0 * PI;
        static float max = 2.0 * PI;
        float *v = (float *)&tr.rotation;

        ImGui::PushID(&tr.rotation);
        ImGui::DragFloat3("Rotation", v, speed, min, max);
        ImGui::PopID();
    }
}

static void update_and_draw_light() {
    assert(ENTITY != entt::null);

    ImGui::SeparatorText("Light");

    auto light = globals::registry.try_get<component::Light>(ENTITY);

    if (light == NULL) {
        if (ImGui::Button("Add [L]ight")) {
            light::Params params = {.point = {.attenuation = {1.0, 0.0, 0.0}}};
            light::Light light(ENTITY, light::Type::POINT, WHITE, 1.0, params);
            globals::registry.emplace<component::Light>(ENTITY, light);
        }
    } else {
        ImGui::Text("HERE ARE LIGHT PARAMS!");
    }
}

void update_and_draw() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        ENTITY = entt::null;
    }

    // ---------------------------------------------------------------
    bool is_remove_down = IsKeyDown(KEY_R);

    if (gui::button("[N]ew Entity") || IsKeyPressed(KEY_N)) {
        ENTITY = prefabs::spawn_entity();
        gizmo::attach(ENTITY);
    }

    // ---------------------------------------------------------------
    // components
    if (ENTITY != entt::null) {
        update_and_draw_transformation();
        update_and_draw_light();
    }
}

}  // namespace soft_tissues::editor::entities_editor
