#include "utils.hpp"

#include "raylib/raylib.h"
#include <fstream>
#include <sstream>
#include <string>

namespace soft_tissues::utils {

std::string get_shader_file_path(const std::string &file_name) {
    auto file_path = "resources/shaders/" + file_name;
    return file_path;
}

std::string load_shader_src(const std::string &file_name) {
    const std::string version_src = "#version 460 core";
    std::ifstream common_file(get_shader_file_path("common.glsl"));
    std::ifstream shader_file(get_shader_file_path(file_name));

    std::stringstream common_stream, shader_stream;
    common_stream << common_file.rdbuf();
    shader_stream << shader_file.rdbuf();

    std::string common_src = common_stream.str();
    std::string shader_src = shader_stream.str();

    std::string full_src = version_src + "\n" + common_src + "\n" + shader_src;

    return full_src;
}

Shader load_shader(const std::string &vs_file_name, const std::string &fs_file_name) {
    auto vs = load_shader_src(vs_file_name);
    auto fs = load_shader_src(fs_file_name);
    Shader shader = LoadShaderFromMemory(vs.c_str(), fs.c_str());

    if (!IsShaderReady(shader)) {
        throw std::runtime_error(
            "Failed to load the shader: " + vs_file_name + ", " + fs_file_name
        );
    }

    return shader;
}

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

}  // namespace soft_tissues::utils
