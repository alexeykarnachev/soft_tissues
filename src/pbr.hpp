#pragma once

#include "raylib/raylib.h"
#include <string>

namespace soft_tissues::pbr {

class MaterialPBR {
private:
    Material material;

public:
    MaterialPBR();
    MaterialPBR(std::string dir_path, Vector2 tiling, float displacement_scale);

    Material get_material();

    void unload();
};

}  // namespace soft_tissues::pbr
