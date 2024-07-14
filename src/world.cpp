#include "world.hpp"

#include "camera.hpp"
#include "component/light.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "resources.hpp"
#include "utils.hpp"
#include <array>

namespace soft_tissues::world {

using namespace utils;

const int WORLD_HEIGHT = 3;
const int WORLD_N_ROWS = 15;
const int WORLD_N_COLS = 15;
const int WORLD_N_TILES = WORLD_N_ROWS * WORLD_N_COLS;

std::array<Tile, WORLD_N_TILES> TILES;

TileMaterials::TileMaterials() = default;

TileMaterials::TileMaterials(Material floor, Material wall, Material ceil)
    : floor(floor)
    , wall(wall)
    , ceil(ceil) {}

Tile::Tile() = default;

Tile::Tile(uint16_t flags, TileMaterials materials)
    : flags(flags)
    , materials(materials) {}

bool Tile::has_flags(uint16_t flags) {
    return (this->flags & flags) == flags;
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

void load() {
    for (int i = 0; i < WORLD_N_TILES; ++i) {
        int row = i / WORLD_N_COLS;
        int col = i % WORLD_N_COLS;
        auto flags = TILE_FLOOR | TILE_CEIL;

        if (row == 0) flags |= TILE_NORTH_WALL;
        if (col == 0) flags |= TILE_WEST_WALL;
        if (row == WORLD_N_ROWS - 1) flags |= TILE_SOUTH_WALL;
        if (col == WORLD_N_COLS - 1) flags |= TILE_EAST_WALL;

        TileMaterials materials(
            resources::TILED_STONE_MATERIAL,
            resources::BRICK_WALL_MATERIAL,
            resources::TILED_STONE_MATERIAL
        );

        TILES[i] = Tile(flags, materials);
    }
}

Vector2 get_center() {
    return {0.5f * WORLD_N_COLS, 0.5f * WORLD_N_ROWS};
}

static Vector2 get_world_position(uint32_t idx) {
    uint32_t row = idx / WORLD_N_COLS;
    uint32_t col = idx % WORLD_N_COLS;

    float x = static_cast<float>(col) + 0.5;
    float y = static_cast<float>(row) + 0.5;

    return {x, y};
}

void draw_grid() {
    // z lines
    for (float x = 1.0; x < WORLD_N_COLS; x += 1.0) {
        Vector3 start_pos = {x, 0.0, 0.0};
        Vector3 end_pos = {x, 0.0, WORLD_N_ROWS};
        DrawLine3D(start_pos, end_pos, WHITE);
    }

    // x lines
    for (float z = 1.0; z < WORLD_N_ROWS; z += 1.0) {
        Vector3 start_pos = {0.0, 0.0, z};
        Vector3 end_pos = {WORLD_N_COLS, 0.0, z};
        DrawLine3D(start_pos, end_pos, WHITE);
    }

    // perimiter
    DrawLine3D({0.0, 0.0, 0.0}, {WORLD_N_COLS, 0.0, 0.0}, RED);
    DrawLine3D({0.0, 0.0, WORLD_N_ROWS}, {WORLD_N_COLS, 0.0, WORLD_N_ROWS}, RED);

    DrawLine3D({0.0, 0.0, 0.0}, {0.0, 0.0, WORLD_N_ROWS}, RED);
    DrawLine3D({WORLD_N_COLS, 0.0, 0.0}, {WORLD_N_COLS, 0.0, WORLD_N_ROWS}, RED);
}

void draw_tiles() {
    for (int i = 0; i < WORLD_N_COLS * WORLD_N_ROWS; ++i) {
        Tile &tile = TILES[i];
        Mesh mesh = resources::PLANE_MESH;
        Vector2 position = get_world_position(i);

        if (tile.has_flags(TILE_FLOOR)) {
            Material material = tile.materials.floor;

            Matrix t = MatrixTranslate(position.x, 0.0, position.y);
            Matrix matrix = t;

            set_shader_uniforms(material.shader);
            DrawMesh(mesh, material, matrix);
        }

        if (tile.has_flags(TILE_CEIL)) {
            Material material = tile.materials.ceil;

            Matrix t = MatrixTranslate(position.x, WORLD_HEIGHT, position.y);
            Matrix r = MatrixRotateX(PI);
            Matrix matrix = MatrixMultiply(r, t);

            set_shader_uniforms(material.shader);
            DrawMesh(mesh, material, matrix);
        }

        if (tile.has_flags(TILE_NORTH_WALL)) {
            Material material = tile.materials.wall;

            set_shader_uniforms(material.shader);
            for (int i = 0; i < WORLD_HEIGHT; ++i) {
                Matrix t = MatrixTranslate(position.x, i + 0.5, position.y - 0.5);
                Matrix r = MatrixRotateX(0.5 * PI);
                Matrix matrix = MatrixMultiply(r, t);
                DrawMesh(mesh, material, matrix);
            }
        }

        if (tile.has_flags(TILE_SOUTH_WALL)) {
            Material material = tile.materials.wall;

            set_shader_uniforms(material.shader);
            for (int i = 0; i < WORLD_HEIGHT; ++i) {
                Matrix t = MatrixTranslate(position.x, i + 0.5, position.y + 0.5);
                Matrix r = MatrixRotateX(-0.5 * PI);
                Matrix matrix = MatrixMultiply(r, t);
                DrawMesh(mesh, material, matrix);
            }
        }

        if (tile.has_flags(TILE_WEST_WALL)) {
            Material material = tile.materials.wall;

            set_shader_uniforms(material.shader);
            for (int i = 0; i < WORLD_HEIGHT; ++i) {
                Matrix t = MatrixTranslate(position.x - 0.5, i + 0.5, position.y);
                Matrix rx = MatrixRotateX(0.5 * PI);
                Matrix ry = MatrixRotateY(0.5 * PI);
                Matrix r = MatrixMultiply(rx, ry);
                Matrix matrix = MatrixMultiply(r, t);
                DrawMesh(mesh, material, matrix);
            }
        }

        if (tile.has_flags(TILE_EAST_WALL)) {
            Material material = tile.materials.wall;

            set_shader_uniforms(material.shader);
            for (int i = 0; i < WORLD_HEIGHT; ++i) {
                Matrix t = MatrixTranslate(position.x + 0.5, i + 0.5, position.y);
                Matrix rx = MatrixRotateX(0.5 * PI);
                Matrix ry = MatrixRotateY(-0.5 * PI);
                Matrix r = MatrixMultiply(rx, ry);
                Matrix matrix = MatrixMultiply(r, t);
                DrawMesh(mesh, material, matrix);
            }
        }
    }
}

}  // namespace soft_tissues::world
