#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace soft_tissues::resources {

pbr::PBRShader &get_pbr_shader();
Material get_material_color(Color color);
const pbr::MaterialPBR &get_material_pbr(const std::string &key);
const Mesh &get_mesh(const std::string &key);

std::vector<std::string> get_material_pbr_keys();

RenderTexture2D *get_shadow_map();
void free_shadow_map(RenderTexture2D *shadow_map);

const std::unordered_map<std::string, Mesh> &get_wall_meshes();
void set_wall_meshes(std::unordered_map<std::string, Mesh> meshes);
void unload_wall_meshes();

void load();
void unload();

}  // namespace soft_tissues::resources
