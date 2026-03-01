#pragma once

#include "core/pbr.hpp"
#include "render_state.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::system::render {

void begin_frame(pbr::PBRShader &pbr_shader, const RenderState &render_state);
void draw_mesh(const Mesh &mesh, const pbr::MaterialPBR &material_pbr, Color constant_color, Matrix matrix, const RenderState &render_state);

}  // namespace soft_tissues::system::render
