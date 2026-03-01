#include "scene.hpp"

#include "../component/component.hpp"
#include "../globals.hpp"
#include "../resources.hpp"
#include "../world.hpp"
#include "render.hpp"
#include "transform.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::system::scene {

using namespace utils;

void draw_grid() {
    Rectangle rect = world::get_bound_rect();

    float min_x = rect.x;
    float max_x = rect.x + rect.width;
    float min_y = rect.y;
    float max_y = rect.y + rect.height;

    Vector3 top_left = {min_x, 0.0, min_y};
    Vector3 top_right = {max_x, 0.0, min_y};
    Vector3 bot_right = {max_x, 0.0, max_y};
    Vector3 bot_left = {min_x, 0.0, max_y};

    // z lines
    for (float x = min_x + 1.0; x < max_x; x += 1.0) {
        Vector3 start_pos = {x, 0.0, min_y};
        Vector3 end_pos = {x, 0.0, max_y};
        DrawLine3D(start_pos, end_pos, WHITE);
    }

    // x lines
    for (float z = min_y + 1.0; z < max_y; z += 1.0) {
        Vector3 start_pos = {min_x, 0.0, z};
        Vector3 end_pos = {max_x, 0.0, z};
        DrawLine3D(start_pos, end_pos, WHITE);
    }

    // perimiter
    DrawLine3D(top_left, top_right, RED);
    DrawLine3D(top_right, bot_right, RED);
    DrawLine3D(bot_right, bot_left, RED);
    DrawLine3D(bot_left, top_left, RED);
}

void draw_tiles(const RenderState &render_state) {
    Mesh mesh = resources::get_mesh("plane");
    tile::Tile *tiles = world::get_tiles();
    int n_tiles = world::get_tiles_count();

    for (int i = 0; i < n_tiles; ++i) {
        tile::Tile &tile = tiles[i];

        // don't draw tile which doesn't belong to any room
        if (world::get_tile_room_id(&tile) == -1) continue;

        // draw floor
        render::draw_mesh(
            mesh,
            resources::get_material_pbr(tile.materials.floor_key),
            tile.constant_color,
            tile.get_floor_matrix(),
            render_state
        );

        // draw ceil
        render::draw_mesh(
            mesh,
            resources::get_material_pbr(tile.materials.ceil_key),
            tile.constant_color,
            tile.get_ceil_matrix(),
            render_state
        );

        // draw solid walls
        auto wall_material_pbr = resources::get_material_pbr(tile.materials.wall_key);
        for (int i_direction = 0; i_direction < 4; ++i_direction) {
            Direction direction = (Direction)i_direction;

            if (tile.has_solid_wall(direction)) {
                for (int i_height = 0; i_height < world::HEIGHT; ++i_height) {
                    Matrix matrix = tile.get_wall_matrix(direction, i_height);
                    render::draw_mesh(mesh, wall_material_pbr, tile.constant_color, matrix, render_state);
                }
            }
        }
    }
}

void draw_meshes(const RenderState &render_state) {
    auto view = globals::registry.view<component::MyMesh>();

    for (auto entity : view) {
        auto my_mesh = globals::registry.get<component::MyMesh>(entity);
        Matrix matrix = transform::get_world_matrix(entity);

        auto mesh = resources::get_mesh(my_mesh.mesh_key);
        auto material_pbr = resources::get_material_pbr(my_mesh.material_pbr_key);

        render::draw_mesh(mesh, material_pbr, my_mesh.constant_color, matrix, render_state);
    }
}

}  // namespace soft_tissues::system::scene
