#pragma once

#include "core/pbr.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include <vector>

namespace soft_tissues::system::lighting {

struct ShadowPassJob {
    entt::entity entity;
    Camera3D camera;
    RenderTexture2D *shadow_map;
};

std::vector<ShadowPassJob> prepare_shadow_passes();
void finalize_shadow_pass(entt::entity entity, Matrix vp_mat);

void set_light_uniforms(pbr::PBRShader &pbr_shader);

}  // namespace soft_tissues::system::lighting
