#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include <vector>

namespace soft_tissues::resources {

extern Material DEFAULT_MATERIAL;

Material get_material_color(Color color);
pbr::MaterialPBR get_material_pbr(std::string key);
Mesh get_mesh(std::string key);

std::vector<std::string> get_material_pbr_keys();

RenderTexture2D *get_shadow_map();
void free_shadow_map(RenderTexture2D *shadow_map);

void load();
void unload();

}  // namespace soft_tissues::resources
