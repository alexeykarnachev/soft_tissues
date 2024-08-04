#include "../component/component.hpp"
#include "../globals.hpp"
#include "../prefabs.hpp"
#include "editor.hpp"
#include "entt/entity/entity.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include <cassert>
#include <cstdio>
#include <stdio.h>

namespace soft_tissues::editor::entities_editor {

entt::entity ENTITY = entt::null;

static void update_and_draw_transformation() {
    assert(ENTITY != entt::null);

    auto &tr = globals::registry.get<component::Transform>(ENTITY);

    ImGui::SeparatorText("Transformation");

    {
        static float speed = 0.1;
        float *v = (float *)&tr._position;

        ImGui::PushID(&tr._position);
        ImGui::DragFloat3("Position", v, speed);
        ImGui::PopID();
    }

    {
        static float speed = PI / 16.0;
        static float min = -2.0 * PI;
        static float max = 2.0 * PI;
        float *v = (float *)&tr._rotation;

        ImGui::PushID(v);
        ImGui::DragFloat3("Rotation", v, speed, min, max);
        ImGui::PopID();
    }
}

static void update_and_draw_light() {
    assert(ENTITY != entt::null);

    auto light = globals::registry.try_get<component::Light>(ENTITY);
    ImGui::PushID(light);

    ImGui::SeparatorText("Light");

    if (light == NULL) {
        if (ImGui::Button("Add [L]ight")) {
            light::Params params = {.point = {.attenuation = {1.0, 1.5, 0.75}}};
            light::Light light(ENTITY, light::Type::POINT, GREEN, 20.0, params);
            globals::registry.emplace<component::Light>(ENTITY, light);
        }
    } else {
        // color
        auto color = ColorNormalize(light->color);
        float *color_p = (float *)&color;
        if (ImGui::ColorEdit3("Color", color_p)) {
            light->color = ColorFromNormalized(color);
        }

        // intensity
        float *v = &light->intensity;
        ImGui::SliderFloat("Intensity", v, 0.0, 100.0);

        // type
        auto type_name = light::get_type_name(light->type);
        ImGui::Text("Type: %s", type_name.c_str());

        switch (light->type) {
            case light::Type::POINT: {
                // attenuation
                float *attenuation = (float *)&light->params.point.attenuation;
                ImGui::SliderFloat2("Attenuation", &attenuation[1], 0.0, 5.0);
            } break;
            default: {
                ImGui::TextColored({1.0, 1.0, 0.0, 1.0}, "TODO: Not implemented");
            } break;
        }
    }
    ImGui::PopID();
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
