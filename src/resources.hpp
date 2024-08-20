#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include <vector>

namespace soft_tissues::resources {

extern Material DEFAULT_MATERIAL;

extern std::vector<pbr::MaterialPBR> MATERIALS_PBR;

extern Mesh PLANE_MESH;
extern Mesh CUBE_MESH;
extern Mesh SPHERE_MESH;

Material get_color_material(Color color);

RenderTexture2D *get_shadow_map();
void free_shadow_map(RenderTexture2D *shadow_map);

void load();
void unload();

}  // namespace soft_tissues::resources
