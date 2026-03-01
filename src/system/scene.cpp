#include "scene.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "core/resources.hpp"
#include "core/world.hpp"
#include "system/render.hpp"
#include "system/transform.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

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
    const Mesh &mesh = resources::get_mesh("plane");
    tile::Tile *tiles = world::get_tiles();
    int n_tiles = world::get_tiles_count();

    for (int i = 0; i < n_tiles; ++i) {
        tile::Tile &tile = tiles[i];

        // don't draw tile which doesn't belong to any room
        if (world::get_tile_room_id(&tile) == -1) continue;

        // draw floor
        const auto &floor_material_pbr = resources::get_material_pbr(tile.materials.floor_key);
        render::draw_mesh(
            mesh, floor_material_pbr, tile.constant_color,
            tile.get_floor_matrix(), render_state
        );

        // draw ceil
        const auto &ceil_material_pbr = resources::get_material_pbr(tile.materials.ceil_key);
        render::draw_mesh(
            mesh, ceil_material_pbr, tile.constant_color,
            tile.get_ceil_matrix(), render_state
        );

        // draw solid walls
        const auto &wall_material_pbr = resources::get_material_pbr(tile.materials.wall_key);
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
        const auto &my_mesh = globals::registry.get<component::MyMesh>(entity);
        Matrix matrix = transform::get_world_matrix(entity);

        const auto &mesh = resources::get_mesh(my_mesh.mesh_key);
        const auto &material_pbr = resources::get_material_pbr(my_mesh.material_pbr_key);

        render::draw_mesh(mesh, material_pbr, my_mesh.constant_color, matrix, render_state);
    }
}

void draw_player() {
    auto view = globals::registry.view<component::Player>();
    if (view.size() == 0) return;
    auto player = view.front();

    Vector3 position = transform::get_world_position(player);
    Matrix matrix = MatrixTranslate(position.x, position.y, position.z);

    const Mesh &mesh = resources::get_mesh("player_cylinder");
    Material material = resources::get_material_color({220, 95, 30, 255});
    DrawMesh(mesh, material, matrix);
}

void draw_light_shells() {
    auto view = globals::registry.view<component::Light>();
    for (auto entity : view) {
        DrawSphere(transform::get_world_position(entity), 0.2, WHITE);
    }
}

}  // namespace soft_tissues::system::scene
