#include "../world.hpp"
#include <stdexcept>

namespace soft_tissues::editor::utils {

void draw_tile_perimiter(tile::Tile *tile, Color color, bool only_walls) {
    static float e = 1e-2;
    static float w = 0.2;
    static float h = w + 1.0;

    if (tile == NULL) {
        throw std::runtime_error("Can't draw tile perimiter for the NULL tile");
    };

    Vector2 center = tile->get_floor_position();

    if (!only_walls || tile->has_flags(tile::TILE_NORTH_WALL)) {
        DrawPlane({center.x, e, center.y - 0.5f}, {h, w}, color);
    }

    if (!only_walls || tile->has_flags(tile::TILE_SOUTH_WALL)) {
        DrawPlane({center.x, e, center.y + 0.5f}, {h, w}, color);
    }

    if (!only_walls || tile->has_flags(tile::TILE_WEST_WALL)) {
        DrawPlane({center.x - 0.5f, e, center.y}, {w, h}, color);
    }

    if (!only_walls || tile->has_flags(tile::TILE_EAST_WALL)) {
        DrawPlane({center.x + 0.5f, e, center.y}, {w, h}, color);
    }
}

void draw_tile_perimiter_walls(tile::Tile *tile, Color color) {
    return draw_tile_perimiter(tile, color, true);
}

void draw_room_perimiter_walls(int room_id, Color color) {
    for (auto tile : world::get_room_tiles(room_id)) {
        utils::draw_tile_perimiter_walls(tile, color);
    }
}

}  // namespace soft_tissues::editor::utils
