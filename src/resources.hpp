#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include <vector>

namespace soft_tissues::resources {

extern std::vector<pbr::MaterialPBR> MATERIALS_PBR;

extern Mesh PLANE_MESH;
extern Mesh CUBE_MESH;

Material get_color_material(Color color);

void load();
void unload();

}  // namespace soft_tissues::resources
