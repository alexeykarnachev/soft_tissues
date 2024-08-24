#pragma once

#include "../component/component.hpp"
#include "../tile.hpp"
#include "entt/entity/fwd.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"

namespace soft_tissues::editor {

extern entt::entity HOVERED_ENTITY;
extern bool IS_GUI_INTERACTED;

void load();
void unload();

void update_and_draw();
void update_hovered_entity();

// -----------------------------------------------------------------------
namespace rooms_editor {

void update_and_draw();

}  // namespace rooms_editor

// -----------------------------------------------------------------------
namespace entities_editor {

void update_entity_picking();
void update_and_draw();

}  // namespace entities_editor

// -----------------------------------------------------------------------
namespace gui {

extern ImVec4 COLOR_RED;
extern ImVec4 COLOR_GREEN;

void push_id();
void pop_id();

bool collapsing_header(const char *name, bool is_opened);
bool collapsing_header(const char *name);
bool button(const char *name, bool is_enabled = true);
bool button_color(const char *name, ImVec4 color, bool is_enabled = true);
void image(unsigned int texture, float width, float height);
void image(Texture texture, float width, float height = 0.0);
void material_picker(std::string *material_pbr_key);
void tile_material_picker(
    std::string *target_material_pbr_key, tile::TileMaterials *tile_materials
);
void spot_light_params(component::Light *light);
void point_light_params(component::Light *light);

}  // namespace gui

// -----------------------------------------------------------------------
namespace utils {

void draw_room_perimiter(int room_id, Color color);
void draw_room_perimiter(int room_id, Color solid_color, Color door_color);

}  // namespace utils

// -----------------------------------------------------------------------
namespace gizmo {

// TODO: Rename options like "GIZMO_COLD, ..." to avoid enum names collisions,
// or refactor using "enum class"
enum State {
    COLD,

    HOT,

    HOT_ROT,
    HOT_AXIS,
    HOT_PLANE,

    ACTIVE,

    ACTIVE_ROT,
    ACTIVE_AXIS,
    ACTIVE_PLANE,
};

extern State STATE;

void load();
void unload();

void attach(entt::entity entity);
void detach();

void update_and_draw();

}  // namespace gizmo

}  // namespace soft_tissues::editor
