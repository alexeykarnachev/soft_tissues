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

pbr::MaterialPBR BRICK_WALL_MATERIAL_PBR;
pbr::MaterialPBR TILED_STONE_MATERIAL_PBR;

Mesh PLANE_MESH;

void load() {
    BRICK_WALL_MATERIAL_PBR = pbr::MaterialPBR(
        "resources/textures/brick_wall/", {1.0, 1.0}, 0.1
    );
    TILED_STONE_MATERIAL_PBR = pbr::MaterialPBR(
        "resources/textures/tiled_stone/", {1.0, 1.0}, 0.0
    );

    PLANE_MESH = gen_mesh_plane(16);
}

void unload() {
    BRICK_WALL_MATERIAL_PBR.unload();
    TILED_STONE_MATERIAL_PBR.unload();

    UnloadMesh(PLANE_MESH);
}

}  // namespace soft_tissues::resources
