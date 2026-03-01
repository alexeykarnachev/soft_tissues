#include "pbr.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "utils.hpp"
#include <filesystem>

namespace soft_tissues::pbr {

using namespace utils;

// -----------------------------------------------------------------------
// PBRShader
PBRShader::PBRShader() = default;

PBRShader::PBRShader(const std::string &vs_file, const std::string &fs_file) {
    shader = load_shader(vs_file, fs_file);

    // vertex attributes
    shader.locs[SHADER_LOC_VERTEX_POSITION] = get_attribute_loc(shader, "a_position");
    shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = get_attribute_loc(shader, "a_tex_coord");
    shader.locs[SHADER_LOC_VERTEX_NORMAL] = get_attribute_loc(shader, "a_normal");
    shader.locs[SHADER_LOC_VERTEX_TANGENT] = get_attribute_loc(shader, "a_tangent");

    // matrix uniforms (used by raylib's DrawMesh)
    shader.locs[SHADER_LOC_MATRIX_MVP] = get_uniform_loc(shader, "u_mvp_mat");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = get_uniform_loc(shader, "u_model_mat");
    shader.locs[SHADER_LOC_MATRIX_NORMAL] = get_uniform_loc(shader, "u_normal_mat");

    // texture map uniforms (used by raylib's DrawMesh)
    shader.locs[SHADER_LOC_MAP_ALBEDO] = get_uniform_loc(shader, "u_albedo_map");
    shader.locs[SHADER_LOC_MAP_METALNESS] = get_uniform_loc(shader, "u_metalness_map");
    shader.locs[SHADER_LOC_MAP_NORMAL] = get_uniform_loc(shader, "u_normal_map");
    shader.locs[SHADER_LOC_MAP_ROUGHNESS] = get_uniform_loc(shader, "u_roughness_map");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = get_uniform_loc(shader, "u_occlusion_map");
    shader.locs[SHADER_LOC_MAP_HEIGHT] = get_uniform_loc(shader, "u_height_map");

    // per-draw uniforms
    is_shadow_map_pass_loc = get_uniform_loc(shader, "u_is_shadow_map_pass");
    camera_pos_loc = get_uniform_loc(shader, "u_camera_pos");
    is_light_enabled_loc = get_uniform_loc(shader, "u_is_light_enabled");
    constant_color_loc = get_uniform_loc(shader, "u_constant_color");
    shadow_map_bias_loc = get_uniform_loc(shader, "u_shadow_map_bias");
    shadow_map_max_dist_loc = get_uniform_loc(shader, "u_shadow_map_max_dist");
    n_lights_loc = get_uniform_loc(shader, "u_n_lights");
    tiling_loc = get_uniform_loc(shader, "u_tiling");
    displacement_scale_loc = get_uniform_loc(shader, "u_displacement_scale");

    // per-light uniforms (resolve for all array indices)
    for (int i = 0; i < globals::MAX_N_LIGHTS; ++i) {
        std::string prefix = "u_lights[" + std::to_string(i) + "].";
        auto &ll = light_locs[i];

        ll.position = get_uniform_loc(shader, prefix + "position");
        ll.type = get_uniform_loc(shader, prefix + "type");
        ll.color = get_uniform_loc(shader, prefix + "color");
        ll.intensity = get_uniform_loc(shader, prefix + "intensity");
        ll.casts_shadows = get_uniform_loc(shader, prefix + "casts_shadows");
        ll.vp_mat = get_uniform_loc(shader, prefix + "vp_mat");
        ll.direction = get_uniform_loc(shader, prefix + "direction");
        ll.attenuation = get_uniform_loc(shader, prefix + "attenuation");
        ll.inner_cutoff = get_uniform_loc(shader, prefix + "inner_cutoff");
        ll.outer_cutoff = get_uniform_loc(shader, prefix + "outer_cutoff");

        ll.shadow_map = GetShaderLocation(
            shader, TextFormat("u_shadow_maps[%d]", i)
        );
    }
}

Shader PBRShader::get_shader() {
    return shader;
}

void PBRShader::unload() {
    UnloadShader(shader);
}

void PBRShader::set_shadow_map_pass(bool value) {
    int v = (int)value;
    SetShaderValue(shader, is_shadow_map_pass_loc, &v, SHADER_UNIFORM_INT);
}

void PBRShader::set_camera_pos(Vector3 pos) {
    SetShaderValue(shader, camera_pos_loc, &pos, SHADER_UNIFORM_VEC3);
}

void PBRShader::set_light_enabled(bool value) {
    int v = (int)value;
    SetShaderValue(shader, is_light_enabled_loc, &v, SHADER_UNIFORM_INT);
}

void PBRShader::set_constant_color(Color color) {
    Vector4 v = ColorNormalize(color);
    SetShaderValue(shader, constant_color_loc, &v, SHADER_UNIFORM_VEC4);
}

void PBRShader::set_shadow_map_bias(float bias) {
    SetShaderValue(shader, shadow_map_bias_loc, &bias, SHADER_UNIFORM_FLOAT);
}

void PBRShader::set_shadow_map_max_dist(float dist) {
    SetShaderValue(shader, shadow_map_max_dist_loc, &dist, SHADER_UNIFORM_FLOAT);
}

void PBRShader::set_n_lights(int n) {
    SetShaderValue(shader, n_lights_loc, &n, SHADER_UNIFORM_INT);
}

void PBRShader::set_tiling(Vector2 tiling) {
    SetShaderValue(shader, tiling_loc, &tiling, SHADER_UNIFORM_VEC2);
}

void PBRShader::set_displacement_scale(float scale) {
    SetShaderValue(shader, displacement_scale_loc, &scale, SHADER_UNIFORM_FLOAT);
}

const PBRShader::LightLocs &PBRShader::get_light_locs(int idx) {
    return light_locs[idx];
}

// -----------------------------------------------------------------------
// MaterialPBR
MaterialPBR::MaterialPBR() = default;

MaterialPBR::MaterialPBR(PBRShader &pbr_shader, std::string dir_path, Vector2 tiling, float displacement_scale)
    : pbr_shader(&pbr_shader)
    , dir_path(dir_path) {
    Material material = LoadMaterialDefault();
    material.shader = pbr_shader.get_shader();

    // texture maps
    material.maps[MATERIAL_MAP_ALBEDO].texture = load_texture(dir_path, "albedo.png");
    material.maps[MATERIAL_MAP_METALNESS].texture = load_texture(dir_path, "metalness.png");
    material.maps[MATERIAL_MAP_NORMAL].texture = load_texture(dir_path, "normal.png");
    material.maps[MATERIAL_MAP_ROUGHNESS].texture = load_texture(dir_path, "roughness.png");
    material.maps[MATERIAL_MAP_OCCLUSION].texture = load_texture(dir_path, "occlusion.png");
    material.maps[MATERIAL_MAP_HEIGHT].texture = load_texture(dir_path, "height.png");

    // per-material uniforms
    pbr_shader.set_tiling(tiling);
    pbr_shader.set_displacement_scale(displacement_scale);

    this->material = material;
}

Texture MaterialPBR::get_texture() {
    return this->material.maps[0].texture;
}

Material MaterialPBR::get_material() {
    return this->material;
}

PBRShader &MaterialPBR::get_pbr_shader() {
    return *this->pbr_shader;
}

std::string MaterialPBR::get_name() {
    std::filesystem::path path(this->dir_path);
    auto parent_name = path.parent_path().filename().string();
    return parent_name;
}

void MaterialPBR::unload() {
    // Unload textures only; the shared PBR shader is managed by resources
    UnloadTexture(this->material.maps[MATERIAL_MAP_ALBEDO].texture);
    UnloadTexture(this->material.maps[MATERIAL_MAP_METALNESS].texture);
    UnloadTexture(this->material.maps[MATERIAL_MAP_NORMAL].texture);
    UnloadTexture(this->material.maps[MATERIAL_MAP_ROUGHNESS].texture);
    UnloadTexture(this->material.maps[MATERIAL_MAP_OCCLUSION].texture);
    UnloadTexture(this->material.maps[MATERIAL_MAP_HEIGHT].texture);
    RL_FREE(this->material.maps);
}

// -----------------------------------------------------------------------
// draw_mesh
void draw_mesh(Mesh mesh, MaterialPBR material_pbr, Color constant_color, Matrix matrix) {
    Material material = material_pbr.get_material();
    PBRShader &pbr_shader = material_pbr.get_pbr_shader();

    pbr_shader.set_shadow_map_pass(globals::IS_SHADOW_MAP_PASS);

    Matrix mat = MatrixInvert(rlGetMatrixModelview());
    pbr_shader.set_camera_pos({mat.m12, mat.m13, mat.m14});

    if (!globals::IS_SHADOW_MAP_PASS) {
        pbr_shader.set_light_enabled(globals::IS_LIGHT_ENABLED);
        pbr_shader.set_constant_color(constant_color);
        pbr_shader.set_shadow_map_bias(globals::SHADOW_MAP_BIAS);
        pbr_shader.set_shadow_map_max_dist(globals::SHADOW_MAP_MAX_DIST);

        if (globals::IS_LIGHT_ENABLED) {
            int light_idx = 0;

            for (auto entity : globals::registry.view<component::Light>()) {
                auto light = globals::registry.get<component::Light>(entity);
                if (!light.is_on) continue;

                light.set_shader_uniform(pbr_shader, light_idx++);
            }

            pbr_shader.set_n_lights(light_idx);
        }
    }

    DrawMesh(mesh, material, matrix);
}

}  // namespace soft_tissues::pbr
