#include "../component/component.hpp"
#include "../globals.hpp"
#include "../prefabs.hpp"
#include "../resources.hpp"
#include "editor.hpp"
#include "entt/entity/entity.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include <cassert>
#include <cstdio>
#include <stdio.h>
#include <string>

namespace soft_tissues::editor::entities_editor {

entt::entity ENTITY = entt::null;

static void update_and_draw_transformation() {
    auto &tr = globals::registry.get<component::Transform>(ENTITY);

    ImGui::SeparatorText("Transformation");

    {
        static float speed = 0.1;
        float *v = (float *)&tr._position;

        gui::push_id();
        ImGui::DragFloat3("Position", v, speed);
        gui::pop_id();
    }

    {
        static float speed = PI / 16.0;
        static float min = -2.0 * PI;
        static float max = 2.0 * PI;
        float *v = (float *)&tr._rotation;

        gui::push_id();
        ImGui::DragFloat3("Rotation", v, speed, min, max);
        gui::pop_id();
    }
}

static void update_and_draw_mesh() {
    auto mesh = globals::registry.try_get<component::MyMesh>(ENTITY);

    gui::push_id();
    ImGui::SeparatorText("Mesh");

    if (mesh == NULL) {
        if (gui::button("Add [M]esh") || IsKeyPressed(KEY_M)) {
            mesh::MyMesh mesh(ENTITY, resources::CUBE_MESH, resources::MATERIALS_PBR[0]);
            globals::registry.emplace<component::MyMesh>(ENTITY, mesh);
        }
    } else {
        gui::material_picker(&mesh->material);
    }

    gui::pop_id();
}

static void update_and_draw_light() {
    auto light = globals::registry.try_get<component::Light>(ENTITY);
    auto &tr = globals::registry.get<component::Transform>(ENTITY);

    gui::push_id();
    ImGui::SeparatorText("Light");

    if (light == NULL) {
        if (gui::button("Add [L]ight") || IsKeyPressed(KEY_L)) {
            light::Params params = {.point = {.attenuation = {1.0, 1.5, 0.75}}};
            light::Light light(ENTITY, light::LightType::POINT, GREEN, 20.0, params);
            globals::registry.emplace<component::Light>(ENTITY, light);
        }
    } else {
        // ---------------------------------------------------------------
        // common light params

        ImGui::Checkbox("is on", &light->is_on);
        ImGui::Checkbox("casts shadows", &light->casts_shadows);

        // color
        auto color = ColorNormalize(light->color);
        float *color_p = (float *)&color;
        if (ImGui::ColorEdit3("Color", color_p)) {
            light->color = ColorFromNormalized(color);
        }

        // intensity
        float *v = &light->intensity;
        ImGui::SliderFloat("Intensity", v, 0.0, 100.0);

        // ---------------------------------------------------------------
        // shadow type selection
        gui::push_id();
        auto selected_type_name = light::get_shadow_type_name(light->shadow_type);

        if (ImGui::BeginCombo("Shadow type", selected_type_name.c_str())) {

            for (auto type : light::SHADOW_TYPES) {
                auto type_name = light::get_shadow_type_name(type);
                if (ImGui::Selectable(type_name.c_str(), type == light->shadow_type)) {
                    light->shadow_type = type;
                }
            }

            ImGui::EndCombo();
        }
        gui::pop_id();

        // ---------------------------------------------------------------
        // light type selection
        selected_type_name = light::get_light_type_name(light->light_type);
        if (ImGui::BeginCombo("Light type", selected_type_name.c_str())) {
            for (auto type : light::LIGHT_TYPES) {
                auto type_name = light::get_light_type_name(type);
                if (ImGui::Selectable(type_name.c_str(), type == light->light_type)) {
                    light->light_type = type;

                    // after selecting the light type, initialize it with some
                    // meaningful default light settings
                    switch (type) {
                        case light::LightType::POINT: {
                            light->params.point.attenuation = {1.0, 1.5, 0.75};

                            light->color = GREEN;
                            light->intensity = 20.0;
                        } break;

                        case light::LightType::DIRECTIONAL: {
                            light->color = YELLOW;
                            light->intensity = 5.0;

                            tr.set_forward({0.0, -1.0, 0.0});
                        } break;

                        case light::LightType::SPOT: {
                            light->params.spot.attenuation = {1.0, 1.2, 0.2};
                            light->params.spot.inner_cutoff = 0.95;
                            light->params.spot.outer_cutoff = 0.80;

                            light->color = {255, 255, 220, 255};
                            light->intensity = 50.0;

                            tr.set_forward({0.0, -1.0, 0.0});
                        } break;

                        case light::LightType::AMBIENT: {
                            light->params.ambient = {};

                            light->color = WHITE;
                            light->intensity = 0.1;
                        } break;

                        default: break;
                    }
                }
            }

            ImGui::EndCombo();
        }

        if (light->shadow_map != NULL) {
            gui::image(light->shadow_map->texture, 150.0, 150.0);
        }

        // ---------------------------------------------------------------
        // specific light type params
        switch (light->light_type) {
            case light::LightType::POINT: {
                gui::point_light_params(light);
            } break;

            case light::LightType::SPOT: {
                gui::spot_light_params(light);
            }

            default: break;
        }
    }

    gui::pop_id();
}

void update_and_draw() {
    if (!globals::registry.valid(ENTITY)) ENTITY = entt::null;

    // ---------------------------------------------------------------
    // entity picking
    bool is_hovered_picked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
                             && gizmo::STATE == gizmo::COLD && !IS_GUI_INTERACTED;

    if (gui::button("[N]ew Entity") || IsKeyPressed(KEY_N)) {
        ENTITY = prefabs::spawn_entity();
    } else if (is_hovered_picked) {
        ENTITY = editor::HOVERED_ENTITY;
    } else if (IsKeyPressed(KEY_ESCAPE)) {
        ENTITY = entt::null;
    }

    gizmo::attach(ENTITY);

    // ---------------------------------------------------------------
    // components
    if (ENTITY != entt::null) {
        update_and_draw_transformation();
        update_and_draw_mesh();
        update_and_draw_light();
    }
}

}  // namespace soft_tissues::editor::entities_editor
