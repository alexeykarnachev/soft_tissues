#pragma once

#include "raylib/raylib.h"
#include <string>

namespace soft_tissues::pbr {

class MaterialPBR {
private:
    Material material;

    std::string dir_path;

public:
    MaterialPBR();
    MaterialPBR(std::string dir_path, Vector2 tiling, float displacement_scale);

    Texture get_texture();
    Material get_material();
    std::string get_name();

    void unload();
};

void draw_mesh(Mesh mesh, MaterialPBR material_pbr, Matrix matrix);

}  // namespace soft_tissues::pbr
