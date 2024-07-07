#include "drawing.hpp"

#include "raylib/raylib.h"
#include <stdexcept>
#include <string>

namespace soft_tissues::drawing {

int get_attribute_loc(Shader shader, std::string name, bool is_fail_allowed) {
    int loc = GetShaderLocationAttrib(shader, name.c_str());
    if (!is_fail_allowed && loc == -1) {
        throw std::runtime_error("Failed to find vertex attribute: " + name);
    }

    return loc;
}

int get_uniform_loc(Shader shader, std::string name, bool is_fail_allowed) {
    int loc = GetShaderLocation(shader, name.c_str());
    if (!is_fail_allowed && loc == -1) {
        throw std::runtime_error("Failed to find uniform: " + name);
    }

    return loc;
}

}  // namespace soft_tissues::drawing
