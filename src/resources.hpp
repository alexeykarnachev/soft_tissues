#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include <vector>

namespace soft_tissues::resources {

extern Material DEFAULT_MATERIAL;

pbr::PBRShader &get_pbr_shader();
Material get_material_color(Color color);
const pbr::MaterialPBR &get_material_pbr(const std::string &key);
const Mesh &get_mesh(const std::string &key);

std::vector<std::string> get_material_pbr_keys();

RenderTexture2D *get_shadow_map();
void free_shadow_map(RenderTexture2D *shadow_map);

void load();
void unload();

}  // namespace soft_tissues::resources
