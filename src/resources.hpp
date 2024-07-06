#pragma once

#include "raylib/raylib.h"

namespace soft_tissues::resources {

extern Material BRICK_WALL_MATERIAL;

extern Mesh PLANE_MESH;

extern Model PLANE_MODEL;

void load();
void unload();

}  // namespace soft_tissues::resources
