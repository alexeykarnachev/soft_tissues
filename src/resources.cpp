#include "resources.hpp"

#include "drawing.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace soft_tissues::resources {
using namespace soft_tissues::drawing;

Material BRICK_WALL_MATERIAL;

Mesh PLANE_MESH;

Model PLANE_MODEL;

static std::string get_shader_file_path(const std::string &file_name) {
    auto file_path = "resources/shaders/" + file_name;
    return file_path;
}

static std::string load_shader_src(const std::string &file_name) {
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

static Shader load_shader(
    const std::string &vs_file_name, const std::string &fs_file_name
) {
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

static Texture load_texture(std::string dir_path, std::string file_name) {
    auto file_path = dir_path + "/" + file_name;
    return LoadTexture(file_path.c_str());
}

static Material load_pbr_material(std::string textures_dir_path) {
    Shader shader = load_shader("base.vert.glsl", "pbr.frag.glsl");
    Material material = LoadMaterialDefault();

    // -------------------------------------------------------------------
    // shader locations

    // vertex attributes
    shader.locs[SHADER_LOC_VERTEX_POSITION] = get_attribute_loc(shader, "a_position");
    shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = get_attribute_loc(shader, "a_tex_coord");
    shader.locs[SHADER_LOC_VERTEX_NORMAL] = get_attribute_loc(shader, "a_normal");
    shader.locs[SHADER_LOC_VERTEX_TANGENT] = get_attribute_loc(shader, "a_tangent");

    // uniforms
    shader.locs[SHADER_LOC_MATRIX_MVP] = get_uniform_loc(shader, "u_mvp_mat");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = get_uniform_loc(shader, "u_model_mat");
    shader.locs[SHADER_LOC_MATRIX_NORMAL] = get_uniform_loc(shader, "u_normal_mat");

    shader.locs[SHADER_LOC_MAP_ALBEDO] = get_uniform_loc(shader, "u_albedo_map");
    shader.locs[SHADER_LOC_MAP_METALNESS] = get_uniform_loc(shader, "u_metalness_map");
    shader.locs[SHADER_LOC_MAP_NORMAL] = get_uniform_loc(shader, "u_normal_map");
    shader.locs[SHADER_LOC_MAP_ROUGHNESS] = get_uniform_loc(shader, "u_roughness_map");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = get_uniform_loc(shader, "u_occlusion_map");

    material.shader = shader;

    // -------------------------------------------------------------------
    // textures
    material.maps[MATERIAL_MAP_ALBEDO].texture = load_texture(
        textures_dir_path, "albedo.png"
    );

    material.maps[MATERIAL_MAP_METALNESS].texture = load_texture(
        textures_dir_path, "/metalness.png"
    );

    material.maps[MATERIAL_MAP_NORMAL].texture = load_texture(
        textures_dir_path, "/normal.png"
    );

    material.maps[MATERIAL_MAP_ROUGHNESS].texture = load_texture(
        textures_dir_path, "/roughness.png"
    );

    material.maps[MATERIAL_MAP_OCCLUSION].texture = load_texture(
        textures_dir_path, "/occlusion.png"
    );

    material.maps[MATERIAL_MAP_HEIGHT].texture = load_texture(
        textures_dir_path, "/height.png"
    );

    return material;
}

void load() {
    BRICK_WALL_MATERIAL = load_pbr_material("resources/textures/brick_wall/");

    PLANE_MESH = GenMeshPlane(1.0, 1.0, 2, 2);

    PLANE_MODEL = LoadModelFromMesh(PLANE_MESH);
    PLANE_MODEL.materials[0] = BRICK_WALL_MATERIAL;
    PLANE_MODEL.transform = MatrixIdentity();
    PLANE_MODEL.materialCount = 1;
    PLANE_MODEL.meshCount = 1;
}

void unload() {
    UnloadMaterial(BRICK_WALL_MATERIAL);

    UnloadMesh(PLANE_MESH);
}

}  // namespace soft_tissues::resources
