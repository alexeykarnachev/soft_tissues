#pragma once

#include "../pbr.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <vector>

namespace soft_tissues::system::lighting {

struct ShadowData {
    RenderTexture2D *shadow_map = nullptr;
    Matrix vp_mat = MatrixIdentity();
    bool needs_update = true;
};

struct ShadowPassJob {
    entt::entity entity;
    Camera3D camera;
    RenderTexture2D *shadow_map;
};

ShadowData *get_shadow_data(entt::entity entity);

void cleanup_shadow_data(entt::entity entity);
void cleanup_all_shadow_data();

std::vector<ShadowPassJob> prepare_shadow_passes();
void finalize_shadow_pass(entt::entity entity, Matrix vp_mat);

void set_light_uniforms(pbr::PBRShader &pbr_shader);

}  // namespace soft_tissues::system::lighting
