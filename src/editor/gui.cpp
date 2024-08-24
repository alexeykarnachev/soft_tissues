#include "../resources.hpp"
#include "../tile.hpp"
#include "editor.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace soft_tissues::editor::gui {

ImVec4 COLOR_RED = {0.8, 0.2, 0.1, 1.0};
ImVec4 COLOR_GREEN = {0.2, 0.8, 0.1, 1.0};

void push_id() {
    static int id = 0;
    static int prev_frame = 0;
    int curr_frame = ImGui::GetFrameCount();

    if (curr_frame != prev_frame) {
        id = 0;
        prev_frame = curr_frame;
    }

    ImGui::PushID(id++);
}

void pop_id() {
    ImGui::PopID();
}

bool collapsing_header(const char *name, bool is_opened) {
    int flags = is_opened ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    return ImGui::CollapsingHeader(name, flags);
}

bool collapsing_header(const char *name) {
    return collapsing_header(name, true);
}

bool button(const char *name, bool is_enabled) {
    push_id();
    if (!is_enabled) ImGui::BeginDisabled();
    bool is_clicked = ImGui::Button(name);
    if (!is_enabled) ImGui::EndDisabled();
    pop_id();

    return is_clicked;
}

bool button_color(const char *name, ImVec4 color, bool is_enabled) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    bool is_clicked = button(name, is_enabled);
    ImGui::PopStyleColor();

    return is_clicked;
}

void image(unsigned int texture, float width, float height) {
    ImGui::Image((ImTextureID)(long)texture, {width, height}, {0, 1}, {1, 0});
}

void image(Texture texture, float width, float height) {
    if (height <= 0.0) {
        float aspect = (float)texture.width / texture.height;
        height = width / aspect;
    }
    return image(texture.id, width, height);
}

void material_picker(std::string *material_pbr_key) {
    if (ImGui::BeginMenu(material_pbr_key->c_str())) {
        ImGui::Separator();

        std::vector<std::string> keys = resources::get_material_pbr_keys();
        for (auto another_material_pbr_key : keys) {
            bool is_selected = another_material_pbr_key == *material_pbr_key;

            if (ImGui::MenuItem(another_material_pbr_key.c_str(), NULL, is_selected)) {
                *material_pbr_key = another_material_pbr_key;
            }

            auto another_material_pbr = resources::get_material_pbr(
                another_material_pbr_key
            );
            gui::image(another_material_pbr.get_texture(), 30.0);
            ImGui::Separator();
        }
        ImGui::EndMenu();
    }

    auto material_pbr = resources::get_material_pbr(*material_pbr_key);
    gui::image(material_pbr.get_texture(), 150.0);
}

void tile_material_picker(
    std::string *target_material_pbr_key, tile::TileMaterials *tile_materials
) {
    material_picker(target_material_pbr_key);
    if (gui::button("[A]pply to all") || IsKeyPressed(KEY_A)) {
        *tile_materials = tile::TileMaterials(*target_material_pbr_key);
    }
}

void spot_light_params(component::Light *light) {
    float *attenuation = (float *)&light->params.spot.attenuation;
    ImGui::SliderFloat2("Attenuation", &attenuation[1], 0.0, 5.0);

    float *inner_cutoff = &light->params.spot.inner_cutoff;
    ImGui::SliderFloat("Inner cutoff", inner_cutoff, 0.0, 1.0);

    float *outer_cutoff = &light->params.spot.outer_cutoff;
    ImGui::SliderFloat("Outer cutoff", outer_cutoff, 0.0, 1.0);
}

void point_light_params(component::Light *light) {
    float *attenuation = (float *)&light->params.point.attenuation;
    ImGui::SliderFloat2("Attenuation", &attenuation[1], 0.0, 5.0);
}

}  // namespace soft_tissues::editor::gui
