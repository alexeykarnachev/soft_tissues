#pragma once

#include "raylib/raylib.h"

namespace soft_tissues::resources {

extern Material DEFAULT_MATERIAL;
extern Material BRICK_WALL_MATERIAL;
extern Material TILED_STONE_MATERIAL;

extern Mesh PLANE_MESH;

void load();
void unload();

}  // namespace soft_tissues::resources
