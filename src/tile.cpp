#include "tile.hpp"

#include "globals.hpp"
#include "pbr.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "resources.hpp"
#include "utils.hpp"
#include <cstdint>

namespace soft_tissues::tile {

using namespace utils;

TileFlags get_wall_tile_flag(CardinalDirection direction) {
    switch (direction) {
        case utils::CardinalDirection::NORTH: return TILE_NORTH_WALL;
        case utils::CardinalDirection::SOUTH: return TILE_SOUTH_WALL;
        case utils::CardinalDirection::WEST: return TILE_WEST_WALL;
        case utils::CardinalDirection::EAST: return TILE_EAST_WALL;
    }
}

TileMaterials::TileMaterials() = default;

TileMaterials::TileMaterials(pbr::MaterialPBR material)
    : floor(material)
    , wall(material)
    , ceil(material) {}

TileMaterials::TileMaterials(
    pbr::MaterialPBR floor, pbr::MaterialPBR wall, pbr::MaterialPBR ceil
)
    : floor(floor)
    , wall(wall)
    , ceil(ceil) {}

Tile::Tile() = default;

Tile::Tile(uint32_t id)
    : id(id) {}

uint32_t Tile::get_id() {
    return this->id;
}

bool Tile::is_empty() {
    return this->flags == 0;
}

bool Tile::has_flags(uint16_t flags) {
    return (this->flags & flags) == flags;
}

Vector2 Tile::get_floor_position() {
    uint32_t row = this->id / globals::WORLD_N_COLS;
    uint32_t col = this->id % globals::WORLD_N_COLS;

    float x = static_cast<float>(col) + 0.5;
    float y = static_cast<float>(row) + 0.5;

    return {x, y};
}

Matrix Tile::get_floor_matrix() {
    Vector2 position = this->get_floor_position();
    Matrix matrix = MatrixTranslate(position.x, 0.0, position.y);

    return matrix;
}

Matrix Tile::get_ceil_matrix() {
    Vector2 position = this->get_floor_position();

    Matrix t = MatrixTranslate(position.x, globals::WORLD_HEIGHT, position.y);
    Matrix r = MatrixRotateX(PI);
    Matrix matrix = MatrixMultiply(r, t);

    return matrix;
}

Matrix Tile::get_wall_matrix(CardinalDirection side, int elevation) {
    Vector2 position = this->get_floor_position();
    float y = elevation + 0.5;

    Matrix matrix;
    switch (side) {
        case CardinalDirection::NORTH: {
            Matrix t = MatrixTranslate(position.x, y, position.y - 0.5);
            Matrix r = MatrixRotateX(0.5 * PI);
            matrix = MatrixMultiply(r, t);
        } break;
        case CardinalDirection::SOUTH: {
            Matrix t = MatrixTranslate(position.x, y, position.y + 0.5);
            Matrix r = MatrixRotateX(-0.5 * PI);
            matrix = MatrixMultiply(r, t);
        } break;
        case CardinalDirection::WEST: {
            Matrix t = MatrixTranslate(position.x - 0.5, y, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            matrix = MatrixMultiply(r, t);
        } break;
        case CardinalDirection::EAST: {
            Matrix t = MatrixTranslate(position.x + 0.5, y, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(-0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            matrix = MatrixMultiply(r, t);
        } break;
    }

    return matrix;
}

void Tile::draw() {
    if (this->is_empty()) return;

    Mesh mesh = resources::PLANE_MESH;

    if (this->has_flags(TILE_FLOOR)) {
        pbr::draw_mesh(
            mesh, this->materials.floor, this->constant_color, this->get_floor_matrix()
        );
    }

    if (this->has_flags(TILE_CEIL)) {
        pbr::draw_mesh(
            mesh, this->materials.ceil, this->constant_color, this->get_ceil_matrix()
        );
    }

    if (this->has_flags(TILE_NORTH_WALL)) {
        for (int i = 0; i < globals::WORLD_HEIGHT; ++i) {
            Matrix matrix = this->get_wall_matrix(CardinalDirection::NORTH, i);
            pbr::draw_mesh(mesh, this->materials.wall, this->constant_color, matrix);
        }
    }

    if (this->has_flags(TILE_SOUTH_WALL)) {
        for (int i = 0; i < globals::WORLD_HEIGHT; ++i) {
            Matrix matrix = this->get_wall_matrix(CardinalDirection::SOUTH, i);
            pbr::draw_mesh(mesh, this->materials.wall, this->constant_color, matrix);
        }
    }

    if (this->has_flags(TILE_WEST_WALL)) {
        for (int i = 0; i < globals::WORLD_HEIGHT; ++i) {
            Matrix matrix = this->get_wall_matrix(CardinalDirection::WEST, i);
            pbr::draw_mesh(mesh, this->materials.wall, this->constant_color, matrix);
        }
    }

    if (this->has_flags(TILE_EAST_WALL)) {
        for (int i = 0; i < globals::WORLD_HEIGHT; ++i) {
            Matrix matrix = this->get_wall_matrix(CardinalDirection::EAST, i);
            pbr::draw_mesh(mesh, this->materials.wall, this->constant_color, matrix);
        }
    }
}

}  // namespace soft_tissues::tile
