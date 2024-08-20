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

Mesh PLANE_MESH;
Mesh CUBE_MESH;
Mesh SPHERE_MESH;

// TODO: Make hashmap instead of the vector
std::vector<pbr::MaterialPBR> MATERIALS_PBR;

static std::unordered_set<int> FREE_SHADOW_MAP_IDXS;
static std::array<RenderTexture2D, globals::MAX_N_SHADOW_MAPS> SHADOW_MAPS;

void load() {
    DEFAULT_MATERIAL = LoadMaterialDefault();

    MATERIALS_PBR = {
        pbr::MaterialPBR("resources/pbr/brick_wall/", {1.0, 1.0}, 0.0),
        pbr::MaterialPBR("resources/pbr/tiled_stone/", {1.0, 1.0}, 0.0),
        pbr::MaterialPBR("resources/pbr/modern_shattered_wallpaper/", {1.0, 1.0}, 0.0),
        pbr::MaterialPBR("resources/pbr/hungarian_point_flooring/", {1.0, 1.0}, 0.0),
        pbr::MaterialPBR("resources/pbr/muddy_scattered_brickwork/", {1.0, 1.0}, 0.0),
    };

    PLANE_MESH = gen_mesh_plane(2);
    CUBE_MESH = gen_mesh_cube();
    SPHERE_MESH = gen_mesh_sphere(64, 64);

    for (size_t i = 0; i < SHADOW_MAPS.size(); ++i) {
        SHADOW_MAPS[i] = LoadRenderTexture(
            globals::SHADOW_MAP_SIZE, globals::SHADOW_MAP_SIZE
        );
        FREE_SHADOW_MAP_IDXS.insert(i);
    }
}

void unload() {
    UnloadMaterial(DEFAULT_MATERIAL);

    UnloadMesh(PLANE_MESH);
    UnloadMesh(CUBE_MESH);
    UnloadMesh(SPHERE_MESH);

    for (auto material : MATERIALS_PBR) {
        material.unload();
    }

    for (auto &shadow_map : SHADOW_MAPS) {
        UnloadRenderTexture(shadow_map);
    }
}

Material get_color_material(Color color) {
    Material material = DEFAULT_MATERIAL;
    material.maps[0].color = color;
    return material;
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
