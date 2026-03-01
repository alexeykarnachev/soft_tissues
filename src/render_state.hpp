#pragma once

namespace soft_tissues {

struct RenderState {
    bool is_shadow_map_pass = false;
    bool is_light_enabled = true;
    float shadow_map_bias = -0.4f;
    float shadow_map_max_dist = 100.0f;
};

}  // namespace soft_tissues
