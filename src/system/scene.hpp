#pragma once

#include "render_state.hpp"

namespace soft_tissues::system::scene {

void draw_grid();
void draw_tiles(const RenderState &render_state);
void draw_meshes(const RenderState &render_state);
void draw_player();
void draw_light_shells();

void rebuild_wall_meshes();
void unload_wall_meshes();

}  // namespace soft_tissues::system::scene
