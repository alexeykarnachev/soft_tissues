#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::resources {

extern pbr::MaterialPBR BRICK_WALL_MATERIAL_PBR;
extern pbr::MaterialPBR TILED_STONE_MATERIAL_PBR;

extern Mesh PLANE_MESH;

Material get_color_material(Color color);

void load();
void unload();

}  // namespace soft_tissues::resources
