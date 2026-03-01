#pragma once

#include "../pbr.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::system::render {

void begin_frame(pbr::PBRShader &pbr_shader);
void draw_mesh(Mesh mesh, pbr::MaterialPBR material_pbr, Color constant_color, Matrix matrix);

}  // namespace soft_tissues::system::render
