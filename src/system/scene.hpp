#pragma once

#include "../render_state.hpp"

namespace soft_tissues::system::scene {

void draw_grid();
void draw_tiles(const RenderState &render_state);
void draw_meshes(const RenderState &render_state);

}  // namespace soft_tissues::system::scene
