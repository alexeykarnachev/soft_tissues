#include "resources.hpp"

#include "pbr.hpp"
#include "gameplay_config.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "utils.hpp"
#include <array>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace soft_tissues::resources {

using namespace utils;

static Material DEFAULT_MATERIAL;

static pbr::PBRShader PBR_SHADER;
static std::unordered_map<std::string, pbr::MaterialPBR> MATERIALS_PBR;
static std::unordered_map<std::string, Mesh> MESHES;

static std::unordered_set<int> FREE_SHADOW_MAP_IDXS;
static std::array<RenderTexture2D, render_config::MAX_N_SHADOW_MAPS> SHADOW_MAPS;

static std::string get_material_pbr_dir_path(const std::string &key) {
    return "resources/pbr/" + key + "/";
}

void load() {
    DEFAULT_MATERIAL = LoadMaterialDefault();

    // -------------------------------------------------------------------
    // pbr shader
    PBR_SHADER = pbr::PBRShader("pbr.vert.glsl", "pbr.frag.glsl");

    // -------------------------------------------------------------------
    // materials pbr
    static const std::array<std::string, 5> material_keys = {
        "brick_wall",
        "tiled_stone",
        "modern_shattered_wallpaper",
        "hungarian_point_flooring",
        "muddy_scattered_brickwork",
    };

    for (const auto &key : material_keys) {
        auto dir_path = get_material_pbr_dir_path(key);
        auto material_pbr = pbr::MaterialPBR(PBR_SHADER, dir_path, {1.0, 1.0}, 0.0);
        MATERIALS_PBR[key] = material_pbr;
    }

    // -------------------------------------------------------------------
    // meshes
    MESHES["plane"] = gen_mesh_plane(2);
    MESHES["cube"] = gen_mesh_cube();
    MESHES["sphere"] = gen_mesh_sphere(64, 64);
    MESHES["player_cylinder"] = GenMeshCylinder(0.25, gameplay_config::PLAYER_HEIGHT, 16);

    // -------------------------------------------------------------------
    // shadow maps
    for (size_t i = 0; i < SHADOW_MAPS.size(); ++i) {
        SHADOW_MAPS[i] = LoadRenderTexture(
            render_config::SHADOW_MAP_SIZE, render_config::SHADOW_MAP_SIZE
        );
        FREE_SHADOW_MAP_IDXS.insert(i);
    }
}

void unload() {
    UnloadMaterial(DEFAULT_MATERIAL);

    // -------------------------------------------------------------------
    // materials pbr (unload before shader since they reference it)
    for (auto &[_, material] : MATERIALS_PBR) {
        material.unload();
    }

    // -------------------------------------------------------------------
    // pbr shader
    PBR_SHADER.unload();

    // -------------------------------------------------------------------
    // meshes
    for (auto &[_, mesh] : MESHES) {
        UnloadMesh(mesh);
    }

    // -------------------------------------------------------------------
    // shadow maps
    for (auto &shadow_map : SHADOW_MAPS) {
        UnloadRenderTexture(shadow_map);
    }
}

// Returns a shallow copy of DEFAULT_MATERIAL with the given color.
// The copy shares DEFAULT_MATERIAL's maps pointer — only safe for single-use-per-frame
// (e.g. pass directly to DrawMesh). Do not store or call UnloadMaterial on the copy.
Material get_material_color(Color color) {
    Material material = DEFAULT_MATERIAL;
    material.maps[0].color = color;
    return material;
}

pbr::PBRShader &get_pbr_shader() {
    return PBR_SHADER;
}

const pbr::MaterialPBR &get_material_pbr(const std::string &key) {
    return MATERIALS_PBR.at(key);
}

const Mesh &get_mesh(const std::string &key) {
    return MESHES.at(key);
}

std::vector<std::string> get_material_pbr_keys() {
    std::vector<std::string> keys;
    for (auto &[key, _] : MATERIALS_PBR) {
        keys.push_back(key);
    }

    return keys;
}

RenderTexture2D *get_shadow_map() {
    if (FREE_SHADOW_MAP_IDXS.empty()) {
        TraceLog(LOG_WARNING, "Shadow map pool exhausted");
        return nullptr;
    }

    auto idx = *FREE_SHADOW_MAP_IDXS.begin();
    FREE_SHADOW_MAP_IDXS.erase(FREE_SHADOW_MAP_IDXS.begin());

    return &SHADOW_MAPS[idx];
}

void free_shadow_map(RenderTexture2D *shadow_map) {
    if (shadow_map == nullptr) {
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
