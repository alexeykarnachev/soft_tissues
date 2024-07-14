#include "tiling.hpp"

#include "camera.hpp"
#include "component/light.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "resources.hpp"
#include "utils.hpp"
#include <cstdint>

namespace soft_tissues::tiling {

using namespace utils;

TileMaterials::TileMaterials(Material floor, Material wall, Material ceil)
    : floor(floor)
    , wall(wall)
    , ceil(ceil) {}

Tile::Tile(uint32_t idx, uint16_t flags, TileMaterials materials)
    : idx(idx)
    , flags(flags)
    , materials(materials) {}

bool Tile::has_flags(uint16_t flags) {
    return (this->flags & flags) == flags;
}

Vector2 Tile::get_world_position() {
    uint32_t row = this->idx / globals::WORLD_N_COLS;
    uint32_t col = this->idx % globals::WORLD_N_COLS;

    float x = static_cast<float>(col) + 0.5;
    float y = static_cast<float>(row) + 0.5;

    return {x, y};
}

static void set_shader_uniforms(Shader shader) {
    static float tiling[2] = {1.0, 1.0};
    static float displacement_scale = 0.1;
    static float ambient_intensity = 0.01;
    static Vector3 ambient_color = {1.0, 1.0, 1.0};

    // -----------------------------------------------------------------------
    // textures
    int tiling_loc = get_uniform_loc(shader, "u_tiling");
    int displacement_scale_loc = get_uniform_loc(shader, "u_displacement_scale");

    SetShaderValue(shader, tiling_loc, tiling, SHADER_UNIFORM_VEC2);
    SetShaderValue(
        shader, displacement_scale_loc, &displacement_scale, SHADER_UNIFORM_FLOAT
    );

    // -----------------------------------------------------------------------
    // ambient light
    int ambient_color_loc = get_uniform_loc(shader, "u_ambient_color");
    int ambient_intensity_loc = get_uniform_loc(shader, "u_ambient_intensity");

    SetShaderValue(shader, ambient_color_loc, &ambient_color, SHADER_UNIFORM_VEC3);
    SetShaderValue(
        shader, ambient_intensity_loc, &ambient_intensity, SHADER_UNIFORM_FLOAT
    );

    // -----------------------------------------------------------------------
    // lighting
    int light_idx = 0;
    for (auto entity : globals::registry.view<light::Light>()) {
        auto light = globals::registry.get<light::Light>(entity);
        light.set_shader_uniform(shader, light_idx++);
    }

    int camera_pos_loc = get_uniform_loc(shader, "u_camera_pos");
    int n_lights_loc = get_uniform_loc(shader, "u_n_lights");

    SetShaderValue(shader, camera_pos_loc, &camera::CAMERA.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, n_lights_loc, &light_idx, SHADER_UNIFORM_INT);
}

void Tile::draw() {
    Mesh mesh = resources::PLANE_MESH;
    Vector2 position = this->get_world_position();

    if (this->has_flags(TILE_FLOOR)) {
        Material material = this->materials.floor;

        Matrix t = MatrixTranslate(position.x, 0.0, position.y);
        Matrix matrix = t;

        set_shader_uniforms(material.shader);
        DrawMesh(mesh, material, matrix);
    }

    if (this->has_flags(TILE_CEIL)) {
        Material material = this->materials.ceil;

        Matrix t = MatrixTranslate(position.x, this->height, position.y);
        Matrix r = MatrixRotateX(PI);
        Matrix matrix = MatrixMultiply(r, t);

        set_shader_uniforms(material.shader);
        DrawMesh(mesh, material, matrix);
    }

    if (this->has_flags(TILE_NORTH_WALL)) {
        Material material = this->materials.wall;

        set_shader_uniforms(material.shader);
        for (int i = 0; i < this->height; ++i) {
            Matrix t = MatrixTranslate(position.x, i + 0.5, position.y - 0.5);
            Matrix r = MatrixRotateX(0.5 * PI);
            Matrix matrix = MatrixMultiply(r, t);
            DrawMesh(mesh, material, matrix);
        }
    }

    if (this->has_flags(TILE_SOUTH_WALL)) {
        Material material = this->materials.wall;

        set_shader_uniforms(material.shader);
        for (int i = 0; i < this->height; ++i) {
            Matrix t = MatrixTranslate(position.x, i + 0.5, position.y + 0.5);
            Matrix r = MatrixRotateX(-0.5 * PI);
            Matrix matrix = MatrixMultiply(r, t);
            DrawMesh(mesh, material, matrix);
        }
    }

    if (this->has_flags(TILE_WEST_WALL)) {
        Material material = this->materials.wall;

        set_shader_uniforms(material.shader);
        for (int i = 0; i < this->height; ++i) {
            Matrix t = MatrixTranslate(position.x - 0.5, i + 0.5, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            Matrix matrix = MatrixMultiply(r, t);
            DrawMesh(mesh, material, matrix);
        }
    }

    if (this->has_flags(TILE_EAST_WALL)) {
        Material material = this->materials.wall;

        set_shader_uniforms(material.shader);
        for (int i = 0; i < this->height; ++i) {
            Matrix t = MatrixTranslate(position.x + 0.5, i + 0.5, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(-0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            Matrix matrix = MatrixMultiply(r, t);
            DrawMesh(mesh, material, matrix);
        }
    }
}

}  // namespace soft_tissues::tiling
