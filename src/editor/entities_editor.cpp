#include "component/component.hpp"
#include "globals.hpp"
#include "core/prefabs.hpp"
#include "system/transform.hpp"
#include "editor.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include <string>

namespace soft_tissues::editor::entities_editor {

entt::entity ENTITY = entt::null;

static void update_and_draw_transformation() {
    auto &tr = globals::registry.get<component::Transform>(ENTITY);

    ImGui::SeparatorText("Transformation");

    {
        static float speed = 0.1;
        float *v = (float *)&tr.position;

        gui::push_id();
        ImGui::DragFloat3("Position", v, speed);
        gui::pop_id();
    }

    {
        static float speed = PI / 16.0;
        static float min = -2.0 * PI;
        static float max = 2.0 * PI;
        float *v = (float *)&tr.rotation;

        gui::push_id();
        ImGui::DragFloat3("Rotation", v, speed, min, max);
        gui::pop_id();
    }
}

static void update_and_draw_mesh() {
    auto mesh = globals::registry.try_get<component::MyMesh>(ENTITY);

    gui::push_id();
    ImGui::SeparatorText("Mesh");

    if (mesh == nullptr) {
        if (gui::button("Add [M]esh") || IsKeyPressed(KEY_M)) {
            component::MyMesh my_mesh("cube", "brick_wall");
            globals::registry.emplace<component::MyMesh>(ENTITY, my_mesh);
        }
    } else {
        gui::material_picker(&mesh->material_pbr_key);
    }

    gui::pop_id();
}

static void update_and_draw_light() {
    auto light = globals::registry.try_get<component::Light>(ENTITY);

    gui::push_id();
    ImGui::SeparatorText("Light");

    if (light == nullptr) {
        if (gui::button("Add [L]ight") || IsKeyPressed(KEY_L)) {
            component::LightParams params = component::PointParams{{1.0, 1.5, 0.75}};
            component::Light light(component::LightType::POINT, GREEN, 20.0, params);
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
        auto selected_type_name = component::shadow_type_to_str(light->shadow_type);

        if (ImGui::BeginCombo("Shadow type", selected_type_name.c_str())) {

            for (auto type : component::SHADOW_TYPES) {
                auto type_name = component::shadow_type_to_str(type);
                if (ImGui::Selectable(type_name.c_str(), type == light->shadow_type)) {
                    light->shadow_type = type;
                }
            }

            ImGui::EndCombo();
        }
        gui::pop_id();

        // ---------------------------------------------------------------
        // light type selection
        selected_type_name = component::light_type_to_str(light->light_type);
        if (ImGui::BeginCombo("Light type", selected_type_name.c_str())) {
            for (auto type : component::LIGHT_TYPES) {
                auto type_name = component::light_type_to_str(type);
                if (ImGui::Selectable(type_name.c_str(), type == light->light_type)) {
                    light->light_type = type;

                    // after selecting the light type, initialize it with some
                    // meaningful default light settings
                    switch (type) {
                        case component::LightType::POINT: {
                            light->params = component::PointParams{{1.0, 1.5, 0.75}};
                            light->color = GREEN;
                            light->intensity = 20.0;
                        } break;

                        case component::LightType::DIRECTIONAL: {
                            light->params = component::DirectionalParams{};
                            light->color = YELLOW;
                            light->intensity = 5.0;
                            system::transform::set_forward(ENTITY,{0.0, -1.0, 0.0});
                        } break;

                        case component::LightType::SPOT: {
                            light->params = component::SpotParams{
                                {1.0, 1.2, 0.2}, 0.95f, 0.80f
                            };
                            light->color = {255, 255, 220, 255};
                            light->intensity = 50.0;
                            system::transform::set_forward(ENTITY,{0.0, -1.0, 0.0});
                        } break;

                        case component::LightType::AMBIENT: {
                            light->params = component::AmbientParams{};
                            light->color = WHITE;
                            light->intensity = 0.1;
                        } break;

                        default: break;
                    }
                }
            }

            ImGui::EndCombo();
        }

        auto *sd = globals::registry.try_get<component::ShadowData>(ENTITY);
        if (sd != nullptr && sd->shadow_map != nullptr) {
            gui::image(sd->shadow_map->texture, 150.0, 150.0);
        }

        // ---------------------------------------------------------------
        // specific light type params
        switch (light->light_type) {
            case component::LightType::POINT: {
                gui::point_light_params(light);
            } break;

            case component::LightType::SPOT: {
                gui::spot_light_params(light);
            } break;

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
