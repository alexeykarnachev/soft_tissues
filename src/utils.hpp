#pragma once

#include "raylib/raylib.h"
#include <string>

namespace soft_tissues::utils {
// -----------------------------------------------------------------------
// shaders
std::string get_shader_file_path(const std::string &file_name);
std::string load_shader_src(const std::string &file_name);
Shader load_shader(const std::string &vs_file_name, const std::string &fs_file_name);

int get_attribute_loc(Shader shader, std::string name, bool is_fail_allowed = false);
int get_uniform_loc(Shader shader, std::string name, bool is_fail_allowed = false);

// -----------------------------------------------------------------------
// math and geometry
RayCollision get_cursor_floor_rect_collision(Rectangle rect, Camera camera);

}  // namespace soft_tissues::utils
