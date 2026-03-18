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

namespace soft_tissues::system::scene {

using namespace utils;

// -----------------------------------------------------------------------
// wall mesh generation
//
// Per-tile approach: iterate every tile, for each solid wall direction
// generate a thick wall segment (4 quads: inner, outer, top, bottom).
// Corner posts fill gaps where perpendicular walls meet.

// Emit a wall segment sitting entirely on one side of the tile edge.
// Used for shared walls: each room emits its own half-width wall on its side.
// inner_depth = half_t (from edge inward), outer_depth = 0 (flush with edge).
static void emit_inner_wall_segment(
    MeshBuilder &mb, Vector2 tile_pos, Direction dir,
    bool cap_a, bool cap_b, bool shrink_a, bool shrink_b
) {
    float half_t = world::WALL_THICKNESS * 0.5f;
    float h = static_cast<float>(world::HEIGHT);
    float cx = tile_pos.x;
    float cz = tile_pos.y;

    float sa = shrink_a ? half_t : 0.0f;
    float sb = shrink_b ? half_t : 0.0f;

    Vector3 in_a, in_b, out_a, out_b;
    Vector3 inward_normal;
    Vector3 cap_a_normal, cap_b_normal;

    // From tile edge to edge + half_t inward (into this tile's room)
    switch (dir) {
        case Direction::NORTH: {
            float z_edge = cz - 0.5f;
            in_a  = {cx - 0.5f + sa, 0, z_edge + half_t};
            in_b  = {cx + 0.5f - sb, 0, z_edge + half_t};
            out_a = {cx - 0.5f + sa, 0, z_edge};
            out_b = {cx + 0.5f - sb, 0, z_edge};
            inward_normal = {0, 0, 1};
            cap_a_normal = {-1, 0, 0};
            cap_b_normal = {1, 0, 0};
        } break;
        case Direction::SOUTH: {
            float z_edge = cz + 0.5f;
            in_a  = {cx + 0.5f - sa, 0, z_edge - half_t};
            in_b  = {cx - 0.5f + sb, 0, z_edge - half_t};
            out_a = {cx + 0.5f - sa, 0, z_edge};
            out_b = {cx - 0.5f + sb, 0, z_edge};
            inward_normal = {0, 0, -1};
            cap_a_normal = {1, 0, 0};
            cap_b_normal = {-1, 0, 0};
        } break;
        case Direction::WEST: {
            float x_edge = cx - 0.5f;
            in_a  = {x_edge + half_t, 0, cz + 0.5f - sa};
            in_b  = {x_edge + half_t, 0, cz - 0.5f + sb};
            out_a = {x_edge, 0, cz + 0.5f - sa};
            out_b = {x_edge, 0, cz - 0.5f + sb};
            inward_normal = {1, 0, 0};
            cap_a_normal = {0, 0, 1};
            cap_b_normal = {0, 0, -1};
        } break;
        case Direction::EAST: {
            float x_edge = cx + 0.5f;
            in_a  = {x_edge - half_t, 0, cz - 0.5f + sa};
            in_b  = {x_edge - half_t, 0, cz + 0.5f - sb};
            out_a = {x_edge, 0, cz - 0.5f + sa};
            out_b = {x_edge, 0, cz + 0.5f - sb};
            inward_normal = {-1, 0, 0};
            cap_a_normal = {0, 0, -1};
            cap_b_normal = {0, 0, 1};
        } break;
    }

    Vector3 outward_normal = {-inward_normal.x, -inward_normal.y, -inward_normal.z};

    // UV: U goes from 0 at a-end to 1 at b-end (original convention).
    // Shrink adjusts to [sa, 1-sb].
    float u_a = sa;
    float u_b = 1.0f - sb;

    // Inner face
    mb.push_quad(
        {in_a.x, 0, in_a.z}, {in_b.x, 0, in_b.z},
        {in_b.x, h, in_b.z}, {in_a.x, h, in_a.z},
        inward_normal, u_a, 0, u_b, h
    );

    // Outer face (at tile edge — visible if neighbor has different material)
    mb.push_quad(
        {out_b.x, 0, out_b.z}, {out_a.x, 0, out_a.z},
        {out_a.x, h, out_a.z}, {out_b.x, h, out_b.z},
        outward_normal, u_a, 0, u_b, h
    );

    // Top cap
    mb.push_quad(
        {in_a.x, h, in_a.z}, {in_b.x, h, in_b.z},
        {out_b.x, h, out_b.z}, {out_a.x, h, out_a.z},
        {0, 1, 0}, u_a, 0, u_b, half_t
    );

    // Bottom cap
    mb.push_quad(
        {out_a.x, 0, out_a.z}, {out_b.x, 0, out_b.z},
        {in_b.x, 0, in_b.z}, {in_a.x, 0, in_a.z},
        {0, -1, 0}, u_a, 0, u_b, half_t
    );

    if (cap_a) {
        mb.push_quad(
            {out_a.x, 0, out_a.z}, {in_a.x, 0, in_a.z},
            {in_a.x, h, in_a.z}, {out_a.x, h, out_a.z},
            cap_a_normal, 0, 0, half_t, h
        );
    }

    if (cap_b) {
        mb.push_quad(
            {in_b.x, 0, in_b.z}, {out_b.x, 0, out_b.z},
            {out_b.x, h, out_b.z}, {in_b.x, h, in_b.z},
            cap_b_normal, 0, 0, half_t, h
        );
    }
}

// Emit a corner fill (half_t x half_t x h) at grid vertex (vx, vz).
// The fill plugs the niche visible from the room at tile (owner_row, owner_col),
// which sits in the given quadrant relative to the vertex.
// quadrant_dx/dz: +1 or -1, indicating which direction the tile center is from the vertex.
// ns_dir/ew_dir: the actual wall directions on this tile at this vertex.
static void emit_corner_fill(
    MeshBuilder &mb, float vx, float vz, int dx, int dz,
    Direction ns_dir, Direction ew_dir
) {
    float half_t = world::WALL_THICKNESS * 0.5f;
    float h = static_cast<float>(world::HEIGHT);

    float x0 = (dx > 0) ? vx : vx - half_t;
    float x1 = (dx > 0) ? vx + half_t : vx;
    float z0 = (dz > 0) ? vz : vz - half_t;
    float z1 = (dz > 0) ? vz + half_t : vz;

    // X-face continues EW wall. Compute U at z0 and z1.
    // EW wall: u=0 at a-end, u=1 at b-end.
    // WEST: a-end at +Z, b-end at -Z → u increases toward -Z.
    // EAST: a-end at -Z, b-end at +Z → u increases toward +Z.
    // The fill's Z range is [z0, z1] (z0 < z1). We need u at each.
    float x_u_at_z0, x_u_at_z1;
    if (ew_dir == Direction::WEST) {
        // u increases toward -Z. Fill is at one end.
        // dz<0: vertex at +Z = a-end. Fill [z0=vz-ht, z1=vz]. z1 is at vertex (+Z, a-end, u≈0). z0 is further -Z (u≈ht).
        // dz>0: vertex at -Z = b-end. Fill [z0=vz, z1=vz+ht]. z0 is at vertex (-Z, b-end, u≈1). z1 is further +Z (u≈1-ht).
        if (dz < 0) { x_u_at_z1 = 0.0f; x_u_at_z0 = half_t; }
        else         { x_u_at_z0 = 1.0f; x_u_at_z1 = 1.0f - half_t; }
    } else { // EAST
        // u increases toward +Z.
        // dz>0: vertex at -Z = a-end. Fill [z0=vz, z1=vz+ht]. z0 at vertex (-Z, a-end, u≈0). z1 further +Z (u≈ht).
        // dz<0: vertex at +Z = b-end. Fill [z0=vz-ht, z1=vz]. z1 at vertex (+Z, b-end, u≈1). z0 further -Z (u≈1-ht).
        if (dz > 0) { x_u_at_z0 = 0.0f; x_u_at_z1 = half_t; }
        else         { x_u_at_z0 = 1.0f - half_t; x_u_at_z1 = 1.0f; }
    }

    // Z-face continues NS wall. Compute U at x0 and x1.
    // NORTH: a-end at -X, b-end at +X → u increases toward +X.
    // SOUTH: a-end at +X, b-end at -X → u increases toward -X.
    float z_u_at_x0, z_u_at_x1;
    if (ns_dir == Direction::NORTH) {
        // u increases toward +X.
        if (dx > 0) { z_u_at_x0 = 0.0f; z_u_at_x1 = half_t; }
        else         { z_u_at_x0 = 1.0f - half_t; z_u_at_x1 = 1.0f; }
    } else { // SOUTH
        // u increases toward -X.
        if (dx < 0) { z_u_at_x0 = half_t; z_u_at_x1 = 0.0f; }
        else         { z_u_at_x0 = 1.0f; z_u_at_x1 = 1.0f - half_t; }
    }

    // Room-facing X quad: faces toward the tile center (dx direction)
    float x_face = (dx > 0) ? x1 : x0;
    Vector3 x_normal = (dx > 0) ? Vector3{1, 0, 0} : Vector3{-1, 0, 0};
    if (dx > 0) {
        // v0 at z1, v1 at z0 → u0 = x_u_at_z1, u1 = x_u_at_z0
        mb.push_quad(
            {x_face, 0, z1}, {x_face, 0, z0}, {x_face, h, z0}, {x_face, h, z1},
            x_normal, x_u_at_z1, 0, x_u_at_z0, h
        );
    } else {
        // v0 at z0, v1 at z1 → u0 = x_u_at_z0, u1 = x_u_at_z1
        mb.push_quad(
            {x_face, 0, z0}, {x_face, 0, z1}, {x_face, h, z1}, {x_face, h, z0},
            x_normal, x_u_at_z0, 0, x_u_at_z1, h
        );
    }

    // Room-facing Z quad: faces toward the tile center (dz direction)
    float z_face = (dz > 0) ? z1 : z0;
    Vector3 z_normal = (dz > 0) ? Vector3{0, 0, 1} : Vector3{0, 0, -1};
    if (dz > 0) {
        // v0 at x0, v1 at x1 → u0 = z_u_at_x0, u1 = z_u_at_x1
        mb.push_quad(
            {x0, 0, z_face}, {x1, 0, z_face}, {x1, h, z_face}, {x0, h, z_face},
            z_normal, z_u_at_x0, 0, z_u_at_x1, h
        );
    } else {
        // v0 at x1, v1 at x0 → u0 = z_u_at_x1, u1 = z_u_at_x0
        mb.push_quad(
            {x1, 0, z_face}, {x0, 0, z_face}, {x0, h, z_face}, {x1, h, z_face},
            z_normal, z_u_at_x1, 0, z_u_at_x0, h
        );
    }

    // Top and bottom caps omitted — occluded by floor/ceiling tiles.
}

void rebuild_wall_meshes() {
    resources::unload_wall_meshes();

    std::unordered_map<std::string, MeshBuilder> builders;
    tile::Tile *tiles = world::get_tiles();
    int n_tiles = world::get_tiles_count();

    // Precompute fill grid: which vertices have perpendicular walls meeting.
    static constexpr int VR = world::N_ROWS + 1;
    static constexpr int VC = world::N_COLS + 1;

    struct TileAt { int r, c; Direction ns_dir; Direction ew_dir; int dx, dz; };
    static constexpr TileAt VERTEX_CHECKS[] = {
        {-1, -1, Direction::SOUTH, Direction::EAST, -1, -1},  // NW tile
        {-1,  0, Direction::SOUTH, Direction::WEST, +1, -1},  // NE tile
        { 0, -1, Direction::NORTH, Direction::EAST, -1, +1},  // SW tile
        { 0,  0, Direction::NORTH, Direction::WEST, +1, +1},  // SE tile
    };

    bool fill_grid[VR][VC] = {};
    for (int r = 0; r < VR; ++r) {
        for (int c = 0; c < VC; ++c) {
            bool has_ns = false, has_ew = false;
            for (auto &ch : VERTEX_CHECKS) {
                tile::Tile *t = world::get_tile_at_row_col(r + ch.r, c + ch.c);
                if (!t || world::get_tile_room_id(t) == -1) continue;
                if (t->has_solid_wall(ch.ns_dir)) has_ns = true;
                if (t->has_solid_wall(ch.ew_dir)) has_ew = true;
            }
            fill_grid[r][c] = has_ns && has_ew;
        }
    }

    // Emit wall segments. Suppress end caps at vertices where a fill exists.
    for (int i = 0; i < n_tiles; ++i) {
        tile::Tile &tile = tiles[i];
        if (world::get_tile_room_id(&tile) == -1) continue;
        if (tile.materials.wall_key.empty()) continue;

        Vector2 pos = tile.get_floor_position();
        auto [tile_row, tile_col] = world::get_tile_row_col(&tile);

        for (int d = 0; d < 4; ++d) {
            Direction dir = static_cast<Direction>(d);
            if (!tile.has_solid_wall(dir)) continue;

            // Find grid vertices at each end of this wall segment.
            int va_r, va_c, vb_r, vb_c;
            switch (dir) {
                case Direction::NORTH:
                    va_r = tile_row; va_c = tile_col;
                    vb_r = tile_row; vb_c = tile_col + 1; break;
                case Direction::SOUTH:
                    va_r = tile_row + 1; va_c = tile_col + 1;
                    vb_r = tile_row + 1; vb_c = tile_col; break;
                case Direction::WEST:
                    va_r = tile_row + 1; va_c = tile_col;
                    vb_r = tile_row; vb_c = tile_col; break;
                case Direction::EAST:
                    va_r = tile_row; va_c = tile_col + 1;
                    vb_r = tile_row + 1; vb_c = tile_col + 1; break;
            }

            bool fill_a = fill_grid[va_r][va_c];
            bool fill_b = fill_grid[vb_r][vb_c];

            const std::string &key = tile.materials.wall_key;
            emit_inner_wall_segment(
                builders[key], pos, dir, !fill_a, !fill_b, fill_a, fill_b
            );
        }
    }

    // Emit corner fills at vertices where perpendicular walls meet.
    Vector2 grid_size = {
        static_cast<float>(world::N_COLS), static_cast<float>(world::N_ROWS)
    };
    for (int row = 0; row < VR; ++row) {
        for (int col = 0; col < VC; ++col) {
            if (!fill_grid[row][col]) continue;

            float vx = static_cast<float>(col) - grid_size.x * 0.5f + world::ORIGIN.x;
            float vz = static_cast<float>(row) - grid_size.y * 0.5f + world::ORIGIN.y;

            for (auto &ch : VERTEX_CHECKS) {
                tile::Tile *t = world::get_tile_at_row_col(row + ch.r, col + ch.c);
                if (!t || world::get_tile_room_id(t) == -1) continue;
                if (t->materials.wall_key.empty()) continue;

                emit_corner_fill(
                    builders[t->materials.wall_key], vx, vz, ch.dx, ch.dz,
                    ch.ns_dir, ch.ew_dir
                );
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
