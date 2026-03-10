#pragma once

#include "raylib/raylib.h"
#include <cstdint>
#include <string>
#include <vector>

namespace soft_tissues::utils {

// -----------------------------------------------------------------------
// enums
enum Direction : uint8_t {
    NORTH = 0,
    SOUTH,
    WEST,
    EAST,
};

Direction flip_direction(Direction direction);

// -----------------------------------------------------------------------
// texture
Texture load_texture(const std::string &dir_path, const std::string &file_name);

// -----------------------------------------------------------------------
// shader
std::string get_shader_file_path(const std::string &file_name);
std::string load_shader_src(const std::string &file_name);
Shader load_shader(const std::string &vs_file_name, const std::string &fs_file_name);

// -----------------------------------------------------------------------
// shader attributes and uniforms
int get_attribute_loc(Shader shader, const std::string &name, bool is_fail_allowed = false);
int get_uniform_loc(Shader shader, const std::string &name, bool is_fail_allowed = false);

// -----------------------------------------------------------------------
// math and geometry
RayCollision get_cursor_floor_rect_collision(Rectangle rect, Camera camera);

// -----------------------------------------------------------------------
// mesh
void gen_mesh_tangents(Mesh *mesh);
Mesh gen_mesh_plane(int resolution);
Mesh gen_mesh_cube();
Mesh gen_mesh_sphere(int n_rings, int n_slices);

// -----------------------------------------------------------------------
// mesh builder
struct MeshBuilder {
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texcoords;
    std::vector<unsigned short> indices;

    void push_quad(
        Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3,
        Vector3 normal,
        float u0, float v_0, float u1, float v_1
    );

    Mesh build();
};

}  // namespace soft_tissues::utils
