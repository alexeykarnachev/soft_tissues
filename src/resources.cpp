#include "resources.hpp"

#include "pbr.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "utils.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace soft_tissues::resources {

using namespace utils;

static Material DEFAULT_MATERIAL;

std::vector<pbr::MaterialPBR> MATERIALS_PBR;

Mesh PLANE_MESH;

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
}

void unload() {
    UnloadMaterial(DEFAULT_MATERIAL);

    for (auto material : MATERIALS_PBR) {
        material.unload();
    }

    UnloadMesh(PLANE_MESH);
}

Material get_color_material(Color color) {
    Material material = DEFAULT_MATERIAL;
    material.maps[0].color = color;
    return material;
}

}  // namespace soft_tissues::resources
