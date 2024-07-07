#pragma once

#include "raylib/raylib.h"
#include <string>
namespace soft_tissues::drawing {

int get_attribute_loc(Shader shader, std::string name, bool is_fail_allowed = false);
int get_uniform_loc(Shader shader, std::string name, bool is_fail_allowed = false);

}  // namespace soft_tissues::drawing
