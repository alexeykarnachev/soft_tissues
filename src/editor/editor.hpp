#pragma once

#include "imgui/imgui.h"
#include "raylib/raylib.h"

namespace soft_tissues::editor {

void load();
void unload();

void update_and_draw();

// -----------------------------------------------------------------------
namespace room_editor {

void update_and_draw();

}  // namespace room_editor

namespace light_editor {

void update_and_draw();

}  // namespace light_editor

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

}  // namespace soft_tissues::editor
