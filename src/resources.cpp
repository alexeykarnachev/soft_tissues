#include "resources.hpp"

#include "globals.hpp"
#include "pbr.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "utils.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace soft_tissues::resources {

using namespace utils;

Material DEFAULT_MATERIAL;

static Shader PBR_SHADER;
static std::unordered_map<std::string, pbr::MaterialPBR> MATERIALS_PBR;
static std::unordered_map<std::string, Mesh> MESHES;

static std::unordered_set<int> FREE_SHADOW_MAP_IDXS;
static std::array<RenderTexture2D, globals::MAX_N_SHADOW_MAPS> SHADOW_MAPS;

static std::string get_material_pbr_dir_path(std::string key) {
    return "resources/pbr/" + key + "/";
}

void load() {
    DEFAULT_MATERIAL = LoadMaterialDefault();

    // -------------------------------------------------------------------
    // pbr shader
    PBR_SHADER = load_shader("pbr.vert.glsl", "pbr.frag.glsl");

    // vertex attributes
    PBR_SHADER.locs[SHADER_LOC_VERTEX_POSITION] = get_attribute_loc(PBR_SHADER, "a_position");
    PBR_SHADER.locs[SHADER_LOC_VERTEX_TEXCOORD01] = get_attribute_loc(PBR_SHADER, "a_tex_coord");
    PBR_SHADER.locs[SHADER_LOC_VERTEX_NORMAL] = get_attribute_loc(PBR_SHADER, "a_normal");
    PBR_SHADER.locs[SHADER_LOC_VERTEX_TANGENT] = get_attribute_loc(PBR_SHADER, "a_tangent");

    // uniforms
    PBR_SHADER.locs[SHADER_LOC_MATRIX_MVP] = get_uniform_loc(PBR_SHADER, "u_mvp_mat");
    PBR_SHADER.locs[SHADER_LOC_MATRIX_MODEL] = get_uniform_loc(PBR_SHADER, "u_model_mat");
    PBR_SHADER.locs[SHADER_LOC_MATRIX_NORMAL] = get_uniform_loc(PBR_SHADER, "u_normal_mat");

    PBR_SHADER.locs[SHADER_LOC_MAP_ALBEDO] = get_uniform_loc(PBR_SHADER, "u_albedo_map");
    PBR_SHADER.locs[SHADER_LOC_MAP_METALNESS] = get_uniform_loc(PBR_SHADER, "u_metalness_map");
    PBR_SHADER.locs[SHADER_LOC_MAP_NORMAL] = get_uniform_loc(PBR_SHADER, "u_normal_map");
    PBR_SHADER.locs[SHADER_LOC_MAP_ROUGHNESS] = get_uniform_loc(PBR_SHADER, "u_roughness_map");
    PBR_SHADER.locs[SHADER_LOC_MAP_OCCLUSION] = get_uniform_loc(PBR_SHADER, "u_occlusion_map");
    PBR_SHADER.locs[SHADER_LOC_MAP_HEIGHT] = get_uniform_loc(PBR_SHADER, "u_height_map");

    // -------------------------------------------------------------------
    // materials pbr
    static const std::array<std::string, 5> material_keys = {
        "brick_wall",
        "tiled_stone",
        "modern_shattered_wallpaper",
        "hungarian_point_flooring",
        "muddy_scattered_brickwork",
    };

    for (auto key : material_keys) {
        auto dir_path = get_material_pbr_dir_path(key);
        auto material_pbr = pbr::MaterialPBR(PBR_SHADER, dir_path, {1.0, 1.0}, 0.0);
        MATERIALS_PBR[key] = material_pbr;
    }

    // -------------------------------------------------------------------
    // meshes
    MESHES["plane"] = gen_mesh_plane(2);
    MESHES["cube"] = gen_mesh_cube();
    MESHES["sphere"] = gen_mesh_sphere(64, 64);

    // -------------------------------------------------------------------
    // shadow maps
    for (size_t i = 0; i < SHADOW_MAPS.size(); ++i) {
        SHADOW_MAPS[i] = LoadRenderTexture(
            globals::SHADOW_MAP_SIZE, globals::SHADOW_MAP_SIZE
        );
        FREE_SHADOW_MAP_IDXS.insert(i);
    }
}

void unload() {
    UnloadMaterial(DEFAULT_MATERIAL);

    // -------------------------------------------------------------------
    // materials pbr (unload before shader since they reference it)
    for (auto [_, material] : MATERIALS_PBR) {
        material.unload();
    }

    // -------------------------------------------------------------------
    // pbr shader
    UnloadShader(PBR_SHADER);

    // -------------------------------------------------------------------
    // meshes
    for (auto [_, mesh] : MESHES) {
        UnloadMesh(mesh);
    }

    // -------------------------------------------------------------------
    // shadow maps
    for (auto &shadow_map : SHADOW_MAPS) {
        UnloadRenderTexture(shadow_map);
    }
}

Material get_material_color(Color color) {
    Material material = DEFAULT_MATERIAL;
    material.maps[0].color = color;
    return material;
}

pbr::MaterialPBR get_material_pbr(std::string key) {
    return MATERIALS_PBR[key];
}

Mesh get_mesh(std::string key) {
    return MESHES[key];
}

std::vector<std::string> get_material_pbr_keys() {
    std::vector<std::string> keys;
    for (auto [key, _] : MATERIALS_PBR) {
        keys.push_back(key);
    }

    return keys;
}

RenderTexture2D *get_shadow_map() {
    if (FREE_SHADOW_MAP_IDXS.size() == 0) return NULL;

    auto idx = *FREE_SHADOW_MAP_IDXS.begin();
    FREE_SHADOW_MAP_IDXS.erase(FREE_SHADOW_MAP_IDXS.begin());

    return &SHADOW_MAPS[idx];
}

void free_shadow_map(RenderTexture2D *shadow_map) {
    if (shadow_map == NULL) {
        throw std::runtime_error("Can't free nullptr shadow map");
    }

    for (size_t i = 0; i < SHADOW_MAPS.size(); ++i) {
        if (&SHADOW_MAPS[i] != shadow_map) continue;

        if (FREE_SHADOW_MAP_IDXS.count(i) != 0) {
            throw std::runtime_error("Shadow map double free");
        }

        FREE_SHADOW_MAP_IDXS.insert(i);

        return;
    }

    throw std::runtime_error(
        "Can only free shadow maps which have been created by resource manager"
    );
}

}  // namespace soft_tissues::resources
