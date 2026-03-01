#pragma once

#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::component {

struct ShadowData {
    RenderTexture2D *shadow_map = nullptr;
    Matrix vp_mat = MatrixIdentity();
    bool needs_update = true;
};

}  // namespace soft_tissues::component
