#pragma once

#include "../pbr.hpp"

namespace soft_tissues::system::lighting {

void draw_shadow_maps();
void set_light_uniforms(pbr::PBRShader &pbr_shader);

}  // namespace soft_tissues::system::lighting
