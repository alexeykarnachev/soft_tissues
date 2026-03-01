#pragma once

#include "render_config.hpp"
#include "raylib/raylib.h"
#include <array>
#include <string>

namespace soft_tissues::pbr {

class PBRShader {
public:
    struct LightLocs {
        int position, type, color, intensity, casts_shadows, vp_mat;
        int direction, attenuation, inner_cutoff, outer_cutoff;
        int shadow_map;
    };

private:
    Shader shader;

    int is_shadow_map_pass_loc;
    int camera_pos_loc;
    int is_light_enabled_loc;
    int constant_color_loc;
    int shadow_map_bias_loc;
    int shadow_map_max_dist_loc;
    int n_lights_loc;
    int tiling_loc;
    int displacement_scale_loc;

    std::array<LightLocs, render_config::MAX_N_LIGHTS> light_locs;

public:
    PBRShader();
    PBRShader(const std::string &vs_file, const std::string &fs_file);

    // Returns Shader by value (shallow copy sharing GPU handle). Do not call UnloadShader() on the copy.
    Shader get_shader() const;
    void unload();

    void set_shadow_map_pass(bool value);
    void set_camera_pos(Vector3 pos);
    void set_light_enabled(bool value);
    void set_constant_color(Color color);
    void set_shadow_map_bias(float bias);
    void set_shadow_map_max_dist(float dist);
    void set_n_lights(int n);
    void set_tiling(Vector2 tiling);
    void set_displacement_scale(float scale);

    const LightLocs &get_light_locs(int idx) const;
};

class MaterialPBR {
private:
    Material material = {};
    PBRShader *pbr_shader = nullptr;

    std::string dir_path;
    Vector2 tiling = {1.0, 1.0};
    float displacement_scale = 0.0;

public:
    MaterialPBR();
    MaterialPBR(PBRShader &pbr_shader, std::string dir_path, Vector2 tiling, float displacement_scale);

    Texture get_texture() const;
    // Returns Material by value (shallow copy sharing internal maps pointer). Do not call UnloadMaterial() on the copy.
    Material get_material() const;
    PBRShader &get_pbr_shader() const;
    Vector2 get_tiling() const;
    float get_displacement_scale() const;
    std::string get_name() const;

    void unload();
};

}  // namespace soft_tissues::pbr
