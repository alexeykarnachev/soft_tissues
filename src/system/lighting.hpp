#pragma once

#include "../pbr.hpp"
#include "../render_state.hpp"
#include <functional>

namespace soft_tissues::system::lighting {

using SceneDrawFn = std::function<void()>;

void draw_shadow_maps(
    pbr::PBRShader &pbr_shader,
    RenderState &render_state,
    SceneDrawFn draw_scene
);

void set_light_uniforms(pbr::PBRShader &pbr_shader);

}  // namespace soft_tissues::system::lighting
