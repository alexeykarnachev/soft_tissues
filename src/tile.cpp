#include "tile.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "serializers.hpp"
#include "utils.hpp"
#include "world.hpp"
#include <cstdint>
#include <cstdio>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

namespace soft_tissues::tile {

using namespace utils;

TileMaterials::TileMaterials() = default;

TileMaterials::TileMaterials(std::string material_pbr_key)
    : floor_key(material_pbr_key)
    , wall_key(material_pbr_key)
    , ceil_key(material_pbr_key) {}

TileMaterials::TileMaterials(
    std::string floor_key, std::string wall_key, std::string ceil_key
)
    : floor_key(floor_key)
    , wall_key(wall_key)
    , ceil_key(ceil_key) {}

nlohmann::json TileMaterials::to_json() {
    return {
        {"floor", this->floor_key},
        {"wall", this->wall_key},
        {"ceiling", this->ceil_key},
    };
}

TileMaterials TileMaterials::from_json(const nlohmann::json &json_data) {
    auto floor_key = json_data["floor"].get<std::string>();
    auto wall_key = json_data["wall"].get<std::string>();
    auto ceiling_key = json_data["ceiling"].get<std::string>();

    return TileMaterials(floor_key, wall_key, ceiling_key);
}

Tile::Tile() = default;

Tile::Tile(uint32_t id)
    : id(id) {}

Tile::Tile(
    uint32_t id,
    std::array<TileWall, 4> walls,
    TileMaterials materials,
    Color constant_color
)
    : id(id)
    , walls(walls)
    , materials(materials)
    , constant_color(constant_color) {}

uint32_t Tile::get_id() {
    return this->id;
}

void Tile::remove_wall(Direction direction) {
    this->walls[direction] = TileWall::NONE;
}

void Tile::remove_all_walls() {
    this->remove_wall(Direction::NORTH);
    this->remove_wall(Direction::SOUTH);
    this->remove_wall(Direction::WEST);
    this->remove_wall(Direction::EAST);
}

void Tile::set_solid_wall(Direction direction) {
    this->walls[direction] = TileWall::SOLID;
}

void Tile::set_door_wall(Direction direction) {
    this->walls[direction] = TileWall::DOOR;
}

bool Tile::has_any_wall(Direction direction) {
    return this->walls[direction] != TileWall::NONE;
}

bool Tile::has_solid_wall(Direction direction) {
    return this->walls[direction] == TileWall::SOLID;
}

bool Tile::has_door_wall(Direction direction) {
    return this->walls[direction] == TileWall::DOOR;
}

bool Tile::has_any_wall() {
    for (int i = 0; i < 4; ++i) {
        if (this->has_any_wall((Direction)i)) return true;
    }
    return false;
}

bool Tile::has_solid_wall() {
    for (int i = 0; i < 4; ++i) {
        if (this->has_solid_wall((Direction)i)) return true;
    }
    return false;
}

bool Tile::has_door_wall() {
    for (int i = 0; i < 4; ++i) {
        if (this->has_door_wall((Direction)i)) return true;
    }
    return false;
}

Vector2 Tile::get_floor_position() {
    uint32_t row = this->id / world::N_COLS;
    uint32_t col = this->id % world::N_COLS;

    Vector2 pos = {col + 0.5f, row + 0.5f};
    pos = Vector2Subtract(pos, Vector2Scale(world::get_size(), 0.5));
    pos = Vector2Add(pos, world::ORIGIN);

    return pos;
}

Matrix Tile::get_floor_matrix() {
    Vector2 position = this->get_floor_position();
    Matrix matrix = MatrixTranslate(position.x, 0.0, position.y);

    return matrix;
}

Matrix Tile::get_ceil_matrix() {
    Vector2 position = this->get_floor_position();

    Matrix t = MatrixTranslate(position.x, world::HEIGHT, position.y);
    Matrix r = MatrixRotateX(PI);
    Matrix matrix = MatrixMultiply(r, t);

    return matrix;
}

Matrix Tile::get_wall_matrix(Direction direction, int elevation) {
    Vector2 position = this->get_floor_position();
    float y = elevation + 0.5;

    Matrix matrix;
    switch (direction) {
        case Direction::NORTH: {
            Matrix t = MatrixTranslate(position.x, y, position.y - 0.5);
            Matrix r = MatrixRotateX(0.5 * PI);
            matrix = MatrixMultiply(r, t);
        } break;
        case Direction::SOUTH: {
            Matrix t = MatrixTranslate(position.x, y, position.y + 0.5);
            Matrix r = MatrixRotateX(-0.5 * PI);
            matrix = MatrixMultiply(r, t);
        } break;
        case Direction::WEST: {
            Matrix t = MatrixTranslate(position.x - 0.5, y, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            matrix = MatrixMultiply(r, t);
        } break;
        case Direction::EAST: {
            Matrix t = MatrixTranslate(position.x + 0.5, y, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(-0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            matrix = MatrixMultiply(r, t);
        } break;
    }

    return matrix;
}

nlohmann::json Tile::to_json() {
    return nlohmann::json{
        {"id", this->id},
        {"walls", this->walls},
        {"materials", this->materials.to_json()},
        {"constant_color", this->constant_color}
    };
}

Tile Tile::from_json(const nlohmann::json &json) {
    uint32_t tile_id = json["id"].get<uint32_t>();
    auto tile_walls = json["walls"].get<std::array<TileWall, 4>>();
    auto tile_materials = TileMaterials::from_json(json["materials"]);
    auto tile_color = json["constant_color"].get<Color>();

    return Tile(tile_id, tile_walls, tile_materials, tile_color);
}

}  // namespace soft_tissues::tile
