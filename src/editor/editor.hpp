#pragma once

#include "../tile.hpp"
#include "entt/entity/fwd.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"

namespace soft_tissues::editor {

void load();
void unload();

void update_and_draw();

// -----------------------------------------------------------------------
namespace rooms_editor {

void update_and_draw();

}  // namespace rooms_editor
//
// -----------------------------------------------------------------------
namespace entities_editor {

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

void draw_tile_perimiter(tile::Tile *tile, Color color, bool only_walls = false);
void draw_tile_perimiter_walls(tile::Tile *tile, Color color);
void draw_room_perimiter_walls(int room_id, Color color);

}  // namespace utils

// -----------------------------------------------------------------------
namespace gizmo {

void load();
void unload();

void attach_to_entity(entt::entity entity);
void detach();

void update_and_draw();

}  // namespace gizmo

}  // namespace soft_tissues::editor
