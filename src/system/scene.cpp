#include "scene.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "core/resources.hpp"
#include "core/world.hpp"
#include "system/render.hpp"
#include "system/transform.hpp"
#include "utils.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace soft_tissues::system::scene {

using namespace utils;

// -----------------------------------------------------------------------
// wall mesh generation
//
// Per-tile approach: iterate every tile, for each solid wall direction
// generate a thick wall segment (4 quads: inner, outer, top, bottom).
// Corner posts fill gaps where perpendicular walls meet.

// Emit a wall segment for a tile's solid wall in a given direction.
// shrink_a/shrink_b: shorten the wall by half_t at each end where a corner
// post fills the gap, preventing overlap between wall and post geometry.
// cap_a/cap_b: emit end caps at the a/b ends (where the wall terminates
// without a corner post or perpendicular wall).
static void emit_wall_segment(
    MeshBuilder &mb, Vector2 tile_pos, Direction dir,
    bool cap_a, bool cap_b, bool shrink_a, bool shrink_b
) {
    float half_t = world::WALL_THICKNESS * 0.5f;
    float h = static_cast<float>(world::HEIGHT);
    float cx = tile_pos.x;
    float cz = tile_pos.y;

    Vector3 in_a, in_b, out_a, out_b;
    Vector3 inward_normal;
    Vector3 cap_a_normal, cap_b_normal;

    float sa = shrink_a ? half_t : 0.0f;
    float sb = shrink_b ? half_t : 0.0f;

    switch (dir) {
        case Direction::NORTH: {
            float z_edge = cz - 0.5f;
            in_a  = {cx - 0.5f + sa, 0, z_edge + half_t};
            in_b  = {cx + 0.5f - sb, 0, z_edge + half_t};
            out_a = {cx - 0.5f + sa, 0, z_edge - half_t};
            out_b = {cx + 0.5f - sb, 0, z_edge - half_t};
            inward_normal = {0, 0, 1};
            cap_a_normal = {-1, 0, 0};
            cap_b_normal = {1, 0, 0};
        } break;
        case Direction::SOUTH: {
            float z_edge = cz + 0.5f;
            in_a  = {cx + 0.5f - sa, 0, z_edge - half_t};
            in_b  = {cx - 0.5f + sb, 0, z_edge - half_t};
            out_a = {cx + 0.5f - sa, 0, z_edge + half_t};
            out_b = {cx - 0.5f + sb, 0, z_edge + half_t};
            inward_normal = {0, 0, -1};
            cap_a_normal = {1, 0, 0};
            cap_b_normal = {-1, 0, 0};
        } break;
        case Direction::WEST: {
            float x_edge = cx - 0.5f;
            in_a  = {x_edge + half_t, 0, cz + 0.5f - sa};
            in_b  = {x_edge + half_t, 0, cz - 0.5f + sb};
            out_a = {x_edge - half_t, 0, cz + 0.5f - sa};
            out_b = {x_edge - half_t, 0, cz - 0.5f + sb};
            inward_normal = {1, 0, 0};
            cap_a_normal = {0, 0, 1};
            cap_b_normal = {0, 0, -1};
        } break;
        case Direction::EAST: {
            float x_edge = cx + 0.5f;
            in_a  = {x_edge - half_t, 0, cz - 0.5f + sa};
            in_b  = {x_edge - half_t, 0, cz + 0.5f - sb};
            out_a = {x_edge + half_t, 0, cz - 0.5f + sa};
            out_b = {x_edge + half_t, 0, cz + 0.5f - sb};
            inward_normal = {-1, 0, 0};
            cap_a_normal = {0, 0, -1};
            cap_b_normal = {0, 0, 1};
        } break;
    }

    Vector3 outward_normal = {-inward_normal.x, -inward_normal.y, -inward_normal.z};

    // Inner face (faces toward room interior)
    mb.push_quad(
        {in_a.x, 0, in_a.z}, {in_b.x, 0, in_b.z},
        {in_b.x, h, in_b.z}, {in_a.x, h, in_a.z},
        inward_normal, 0, 0, 1, h
    );

    // Outer face (faces away from room)
    mb.push_quad(
        {out_b.x, 0, out_b.z}, {out_a.x, 0, out_a.z},
        {out_a.x, h, out_a.z}, {out_b.x, h, out_b.z},
        outward_normal, 0, 0, 1, h
    );

    // Top cap
    mb.push_quad(
        {in_a.x, h, in_a.z}, {in_b.x, h, in_b.z},
        {out_b.x, h, out_b.z}, {out_a.x, h, out_a.z},
        {0, 1, 0}, 0, 0, 1, world::WALL_THICKNESS
    );

    // Bottom cap
    mb.push_quad(
        {out_a.x, 0, out_a.z}, {out_b.x, 0, out_b.z},
        {in_b.x, 0, in_b.z}, {in_a.x, 0, in_a.z},
        {0, -1, 0}, 0, 0, 1, world::WALL_THICKNESS
    );

    // End cap at a-end
    if (cap_a) {
        mb.push_quad(
            {out_a.x, 0, out_a.z}, {in_a.x, 0, in_a.z},
            {in_a.x, h, in_a.z}, {out_a.x, h, out_a.z},
            cap_a_normal, 0, 0, world::WALL_THICKNESS, h
        );
    }

    // End cap at b-end
    if (cap_b) {
        mb.push_quad(
            {in_b.x, 0, in_b.z}, {out_b.x, 0, out_b.z},
            {out_b.x, h, out_b.z}, {in_b.x, h, in_b.z},
            cap_b_normal, 0, 0, world::WALL_THICKNESS, h
        );
    }
}

// Emit a corner post (half_t x half_t x h box) at grid vertex (vx, vz).
// The post fills the gap where perpendicular walls meet at a corner.
static void emit_corner_post(MeshBuilder &mb, float vx, float vz) {
    float half_t = world::WALL_THICKNESS * 0.5f;
    float h = static_cast<float>(world::HEIGHT);

    float x0 = vx - half_t;
    float x1 = vx + half_t;
    float z0 = vz - half_t;
    float z1 = vz + half_t;

    // +X face
    mb.push_quad(
        {x1, 0, z1}, {x1, 0, z0}, {x1, h, z0}, {x1, h, z1},
        {1, 0, 0}, 0, 0, world::WALL_THICKNESS, h
    );
    // -X face
    mb.push_quad(
        {x0, 0, z0}, {x0, 0, z1}, {x0, h, z1}, {x0, h, z0},
        {-1, 0, 0}, 0, 0, world::WALL_THICKNESS, h
    );
    // +Z face
    mb.push_quad(
        {x0, 0, z1}, {x1, 0, z1}, {x1, h, z1}, {x0, h, z1},
        {0, 0, 1}, 0, 0, world::WALL_THICKNESS, h
    );
    // -Z face
    mb.push_quad(
        {x1, 0, z0}, {x0, 0, z0}, {x0, h, z0}, {x1, h, z0},
        {0, 0, -1}, 0, 0, world::WALL_THICKNESS, h
    );
    // Top
    mb.push_quad(
        {x0, h, z1}, {x1, h, z1}, {x1, h, z0}, {x0, h, z0},
        {0, 1, 0}, 0, 0, world::WALL_THICKNESS, world::WALL_THICKNESS
    );
    // Bottom
    mb.push_quad(
        {x0, 0, z0}, {x1, 0, z0}, {x1, 0, z1}, {x0, 0, z1},
        {0, -1, 0}, 0, 0, world::WALL_THICKNESS, world::WALL_THICKNESS
    );
}

// Check if a corner post is needed at grid vertex (row, col).
// A corner post is needed when any wall from a surrounding tile
// has thickness that reaches this vertex — i.e., when at least two
// perpendicular walls meet here.
static bool needs_corner_post(int row, int col) {
    // The 4 tiles sharing this vertex:
    //   (row-1, col-1) = NW tile — its SE corner is this vertex
    //   (row-1, col)   = NE tile — its SW corner is this vertex
    //   (row, col-1)   = SW tile — its NE corner is this vertex
    //   (row, col)     = SE tile — its NW corner is this vertex
    //
    // A wall touches this vertex if it runs along one of the two
    // edges meeting at this corner. Check for any pair of perpendicular
    // walls from these 4 tiles.

    bool has_ns_wall = false;  // NORTH/SOUTH wall (runs along X)
    bool has_ew_wall = false;  // WEST/EAST wall (runs along Z)

    auto check_tile = [&](int r, int c, Direction d1, Direction d2) {
        tile::Tile *t = world::get_tile_at_row_col(r, c);
        if (t && world::get_tile_room_id(t) != -1) {
            if (t->has_solid_wall(d1)) has_ns_wall = true;
            if (t->has_solid_wall(d2)) has_ew_wall = true;
        }
    };

    // NW tile (row-1, col-1): SOUTH wall, EAST wall
    check_tile(row - 1, col - 1, Direction::SOUTH, Direction::EAST);
    if (has_ns_wall && has_ew_wall) return true;

    // NE tile (row-1, col): SOUTH wall, WEST wall
    check_tile(row - 1, col, Direction::SOUTH, Direction::WEST);
    if (has_ns_wall && has_ew_wall) return true;

    // SW tile (row, col-1): NORTH wall, EAST wall
    check_tile(row, col - 1, Direction::NORTH, Direction::EAST);
    if (has_ns_wall && has_ew_wall) return true;

    // SE tile (row, col): NORTH wall, WEST wall
    check_tile(row, col, Direction::NORTH, Direction::WEST);
    return has_ns_wall && has_ew_wall;
}

// Get the grid vertex (row, col) at each end of a wall segment.
// For a wall in direction `dir` on tile at (tile_row, tile_col):
//   a-end vertex and b-end vertex (matching get_end_directions convention).
static std::pair<std::pair<int,int>, std::pair<int,int>> get_end_vertices(
    int tile_row, int tile_col, Direction dir
) {
    // Vertex (row, col) is at the top-left corner of tile (row, col).
    // Tile (r, c) has 4 corners:
    //   NW vertex = (r, c), NE = (r, c+1), SW = (r+1, c), SE = (r+1, c+1)
    switch (dir) {
        case Direction::NORTH:
            // Edge at top of tile. a=WEST end=NW vertex, b=EAST end=NE vertex
            return {{tile_row, tile_col}, {tile_row, tile_col + 1}};
        case Direction::SOUTH:
            // Edge at bottom. a=EAST end=SE vertex, b=WEST end=SW vertex
            return {{tile_row + 1, tile_col + 1}, {tile_row + 1, tile_col}};
        case Direction::WEST:
            // Edge at left. a=SOUTH end=SW vertex, b=NORTH end=NW vertex
            return {{tile_row + 1, tile_col}, {tile_row, tile_col}};
        case Direction::EAST:
            // Edge at right. a=NORTH end=NE vertex, b=SOUTH end=SE vertex
            return {{tile_row, tile_col + 1}, {tile_row + 1, tile_col + 1}};
    }
    __builtin_unreachable();
}

// Determine which wall_key to use for a wall segment emitted by this tile.
// For shared walls (dedup'd to lower-index tile), use that tile's wall_key.
static const std::string &get_wall_key_for_tile(tile::Tile &tile) {
    return tile.materials.wall_key;
}

// Find the wall_key for a corner post at grid vertex (row, col).
// Uses the first surrounding tile that has a wall touching this vertex.
static const std::string &get_corner_post_wall_key(int row, int col) {
    static const std::string EMPTY;

    // Check the 4 surrounding tiles for any wall touching this vertex
    struct Check { int r; int c; Direction d1; Direction d2; };
    Check checks[] = {
        {row - 1, col - 1, Direction::SOUTH, Direction::EAST},
        {row - 1, col,     Direction::SOUTH, Direction::WEST},
        {row,     col - 1, Direction::NORTH, Direction::EAST},
        {row,     col,     Direction::NORTH, Direction::WEST},
    };

    for (auto &ch : checks) {
        tile::Tile *t = world::get_tile_at_row_col(ch.r, ch.c);
        if (t && world::get_tile_room_id(t) != -1) {
            if (t->has_solid_wall(ch.d1) || t->has_solid_wall(ch.d2)) {
                return t->materials.wall_key;
            }
        }
    }

    return EMPTY;
}

void rebuild_wall_meshes() {
    resources::unload_wall_meshes();

    std::unordered_map<std::string, MeshBuilder> builders;
    tile::Tile *tiles = world::get_tiles();
    int n_tiles = world::get_tiles_count();

    // Precompute corner post grid
    static constexpr int VR = world::N_ROWS + 1;
    static constexpr int VC = world::N_COLS + 1;
    bool post_grid[VR][VC] = {};
    for (int r = 0; r <= world::N_ROWS; ++r) {
        for (int c = 0; c <= world::N_COLS; ++c) {
            post_grid[r][c] = needs_corner_post(r, c);
        }
    }

    // Emit wall segments (one MeshBuilder per wall_key)
    for (int i = 0; i < n_tiles; ++i) {
        tile::Tile &tile = tiles[i];
        if (world::get_tile_room_id(&tile) == -1) continue;
        if (tile.materials.wall_key.empty()) continue;  // tiles without materials yet

        Vector2 pos = tile.get_floor_position();
        auto neighbors = world::get_tile_neighbors(&tile);
        auto [tile_row, tile_col] = world::get_tile_row_col(&tile);

        for (int d = 0; d < 4; ++d) {
            Direction dir = static_cast<Direction>(d);
            if (!tile.has_solid_wall(dir)) continue;

            // Dedup shared walls: only the lower-index tile emits.
            tile::Tile *nb_across = neighbors[d];
            if (nb_across && nb_across->has_solid_wall(flip_direction(dir))) {
                int nb_idx = static_cast<int>(nb_across - tiles);
                if (i > nb_idx) continue;
            }

            // Look up corner posts at each end vertex to decide shrink/cap.
            auto [va, vb] = get_end_vertices(tile_row, tile_col, dir);
            bool post_a = post_grid[va.first][va.second];
            bool post_b = post_grid[vb.first][vb.second];

            bool cap_a = !post_a;
            bool cap_b = !post_b;

            const std::string &key = get_wall_key_for_tile(tile);
            emit_wall_segment(builders[key], pos, dir, cap_a, cap_b, post_a, post_b);
        }
    }

    // Emit corner posts
    Vector2 size = {static_cast<float>(world::N_COLS), static_cast<float>(world::N_ROWS)};
    for (int row = 0; row <= world::N_ROWS; ++row) {
        for (int col = 0; col <= world::N_COLS; ++col) {
            if (!post_grid[row][col]) continue;

            float vx = static_cast<float>(col) - size.x * 0.5f + world::ORIGIN.x;
            float vz = static_cast<float>(row) - size.y * 0.5f + world::ORIGIN.y;

            const std::string &key = get_corner_post_wall_key(row, col);
            if (!key.empty()) {
                emit_corner_post(builders[key], vx, vz);
            }
        }
    }

    // Build meshes
    std::unordered_map<std::string, Mesh> meshes;
    for (auto &[key, mb] : builders) {
        if (!mb.vertices.empty()) {
            meshes.emplace(key, mb.build());
        }
    }
    resources::set_wall_meshes(std::move(meshes));
}

// -----------------------------------------------------------------------
// drawing

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

    // perimeter
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
    }

    // draw wall meshes (one mesh per wall material)
    const auto &wall_meshes = resources::get_wall_meshes();
    Matrix identity = MatrixIdentity();
    Color no_color = {0, 0, 0, 0};
    for (const auto &[wall_key, wall_mesh] : wall_meshes) {
        const auto &wall_material_pbr = resources::get_material_pbr(wall_key);
        render::draw_mesh(wall_mesh, wall_material_pbr, no_color, identity, render_state);
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
    if (view.empty()) return;
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
