#pragma once

#include "../component/component.hpp"
#include "entt/entity/fwd.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"

namespace soft_tissues::editor {

extern entt::entity HOVERED_ENTITY;

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

bool collapsing_header(const char *name, bool is_opened);
bool collapsing_header(const char *name);
bool button(const char *name, bool is_enabled = true);
bool button_color(const char *name, ImVec4 color, bool is_enabled = true);
bool button_cancel(bool is_enabled = true);
bool button_accept(bool is_enabled = true);
void image(Texture texture, float width, float height = 0.0);

}  // namespace gui

// -----------------------------------------------------------------------
namespace utils {

void draw_room_perimiter(int room_id, Color color);
void draw_room_perimiter(int room_id, Color solid_color, Color door_color);
void update_and_draw_common_light_params(component::Light *light);
void update_and_draw_spot_light_params(component::Light *light);

}  // namespace utils

// -----------------------------------------------------------------------
namespace gizmo {

void load();
void unload();

void attach(entt::entity entity);
void detach();

void update_and_draw();

}  // namespace gizmo

}  // namespace soft_tissues::editor
