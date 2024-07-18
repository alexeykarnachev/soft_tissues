#include "pbr.hpp"

#include "camera.hpp"
#include "component/light.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "utils.hpp"
#include <filesystem>

namespace soft_tissues::pbr {

using namespace utils;

MaterialPBR::MaterialPBR() = default;

MaterialPBR::MaterialPBR(std::string dir_path, Vector2 tiling, float displacement_scale)
    : dir_path(dir_path) {
    Material material = LoadMaterialDefault();
    Shader shader = load_shader("pbr.vert.glsl", "pbr.frag.glsl");

    // -------------------------------------------------------------------
    // set shader locations

    // vertex attributes
    shader.locs[SHADER_LOC_VERTEX_POSITION] = get_attribute_loc(shader, "a_position");
    shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = get_attribute_loc(shader, "a_tex_coord");
    shader.locs[SHADER_LOC_VERTEX_NORMAL] = get_attribute_loc(shader, "a_normal");
    shader.locs[SHADER_LOC_VERTEX_TANGENT] = get_attribute_loc(shader, "a_tangent");

    // uniforms
    shader.locs[SHADER_LOC_MATRIX_MVP] = get_uniform_loc(shader, "u_mvp_mat");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = get_uniform_loc(shader, "u_model_mat");
    shader.locs[SHADER_LOC_MATRIX_NORMAL] = get_uniform_loc(shader, "u_normal_mat");

    shader.locs[SHADER_LOC_MAP_ALBEDO] = get_uniform_loc(shader, "u_albedo_map");
    shader.locs[SHADER_LOC_MAP_METALNESS] = get_uniform_loc(shader, "u_metalness_map");
    shader.locs[SHADER_LOC_MAP_NORMAL] = get_uniform_loc(shader, "u_normal_map");
    shader.locs[SHADER_LOC_MAP_ROUGHNESS] = get_uniform_loc(shader, "u_roughness_map");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = get_uniform_loc(shader, "u_occlusion_map");
    shader.locs[SHADER_LOC_MAP_HEIGHT] = get_uniform_loc(shader, "u_height_map");

    // texture maps
    material.maps[MATERIAL_MAP_ALBEDO].texture = load_texture(dir_path, "albedo.png");
    material.maps[MATERIAL_MAP_METALNESS].texture = load_texture(
        dir_path, "metalness.png"
    );
    material.maps[MATERIAL_MAP_NORMAL].texture = load_texture(dir_path, "normal.png");
    material.maps[MATERIAL_MAP_ROUGHNESS].texture = load_texture(
        dir_path, "roughness.png"
    );
    material.maps[MATERIAL_MAP_OCCLUSION].texture = load_texture(
        dir_path, "occlusion.png"
    );
    material.maps[MATERIAL_MAP_HEIGHT].texture = load_texture(dir_path, "height.png");
    material.shader = shader;

    // -------------------------------------------------------------------
    // set constant uniforms values

    // textures
    int tiling_loc = get_uniform_loc(shader, "u_tiling");
    int displacement_scale_loc = get_uniform_loc(shader, "u_displacement_scale");

    SetShaderValue(shader, tiling_loc, &tiling, SHADER_UNIFORM_VEC2);
    SetShaderValue(
        shader, displacement_scale_loc, &displacement_scale, SHADER_UNIFORM_FLOAT
    );

    this->material = material;
}

Material MaterialPBR::get_material() {
    Shader shader = this->material.shader;

    // -----------------------------------------------------------------------
    // lighting
    int light_idx = 0;
    for (auto entity : globals::registry.view<light::Light>()) {
        auto light = globals::registry.get<light::Light>(entity);
        light.set_shader_uniform(shader, light_idx++);
    }

    int camera_pos_loc = get_uniform_loc(shader, "u_camera_pos");
    int n_lights_loc = get_uniform_loc(shader, "u_n_lights");

    SetShaderValue(shader, camera_pos_loc, &camera::CAMERA.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, n_lights_loc, &light_idx, SHADER_UNIFORM_INT);

    return this->material;
}

std::string MaterialPBR::get_name() {
    std::filesystem::path path(this->dir_path);
    auto parent_name = path.parent_path().filename().string();
    return parent_name;
}

void MaterialPBR::unload() {
    UnloadMaterial(this->material);
}

}  // namespace soft_tissues::pbr
