#include "../resources.hpp"
#include "../tile.hpp"
#include "../world.hpp"
#include "editor.hpp"
#include "raylib/raylib.h"
#include <cstdio>
#include <unordered_set>

namespace soft_tissues::editor::room_editor {

using namespace utils;

static std::unordered_set<tile::Tile *> RELEASED_ROOM_TILES;
static std::unordered_set<tile::Tile *> CREATING_ROOM_TILES;
static tile::TileMaterials MATERIALS;

static void set_room_tile_flags(tile::Tile *tile) {
    auto nbs = world::get_tile_neighbors(tile->get_id());
    tile->flags = tile::TileFlags::TILE_FLOOR | tile::TileFlags::TILE_CEIL;
    tile->materials = MATERIALS;

    // TODO: manual enumeration is very bad in this case, IMPROVE!
    auto nb = nbs[(int)CardinalDirection::NORTH];
    bool has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_NORTH_WALL;
    }

    nb = nbs[(int)CardinalDirection::SOUTH];
    has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_SOUTH_WALL;
    }

    nb = nbs[(int)CardinalDirection::WEST];
    has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_WEST_WALL;
    }

    nb = nbs[(int)CardinalDirection::EAST];
    has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_EAST_WALL;
    }
}

static void update_material(pbr::MaterialPBR *material) {
    auto name = material->get_name();

    if (ImGui::BeginMenu(name.c_str())) {
        for (auto &another_material : resources::MATERIALS_PBR) {
            auto another_name = another_material.get_name();
            bool is_selected = name == another_name;

            if (ImGui::MenuItem(another_name.c_str(), NULL, is_selected)) {
                *material = another_material;
            }
            gui::image(another_material.get_texture(), 30.0);
            ImGui::Separator();
        }
        ImGui::EndMenu();
    }

    gui::image(material->get_texture(), 150.0);
}

static void update_active() {
    // ---------------------------------------------------------------
    // buttons
    gui::button("Create", false);

    ImGui::SameLine();

    if (gui::button_cancel()) {
        for (auto tile : RELEASED_ROOM_TILES) {
            tile->flags = 0;
        }
        RELEASED_ROOM_TILES.clear();
        STATE = EditorState::NONE;
    }

    // ---------------------------------------------------------------
    // materials
    ImGui::SeparatorText("Materials");

    ImGui::BeginTabBar("Materials");

    if (ImGui::BeginTabItem("floor")) {
        update_material(&MATERIALS.floor);
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("wall")) {
        update_material(&MATERIALS.wall);
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("ceil")) {
        update_material(&MATERIALS.ceil);
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    // ---------------------------------------------------------------
    // creating tiles
    tile::Tile *tile = world::get_tile_at_cursor();

    static tile::Tile *start_tile = NULL;
    static tile::Tile *end_tile = NULL;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        start_tile = tile;
        end_tile = tile;
    } else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {

        if (tile != NULL) {
            end_tile = tile;
        }

        if (start_tile != NULL && end_tile != NULL) {
            CREATING_ROOM_TILES = world::get_tiles_between_corners(
                start_tile->get_id(), end_tile->get_id()
            );
            for (auto tile : CREATING_ROOM_TILES) {
                Material material = resources::get_color_material(WHITE);
                Matrix matrix = tile->get_floor_matrix();
                DrawMesh(resources::PLANE_MESH, material, matrix);
            }
        }
    } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        for (auto tile : CREATING_ROOM_TILES) {
            if (tile == NULL) continue;
            if (!tile->is_empty()) continue;

            set_room_tile_flags(tile);
            auto neighbors = world::get_tile_neighbors(tile->get_id());

            for (auto nb : neighbors) {
                if (nb && !nb->is_empty()) {
                    set_room_tile_flags(nb);
                }
            }

            set_room_tile_flags(tile);
            RELEASED_ROOM_TILES.insert(tile);
        }

        CREATING_ROOM_TILES.clear();
        start_tile = NULL;
        end_tile = NULL;
    }

    if (tile != NULL) {
        Material material = resources::get_color_material(GREEN);
        Matrix matrix = tile->get_floor_matrix();
        DrawMesh(resources::PLANE_MESH, material, matrix);
    }

    // set materials for all room tiles
    for (auto tile : RELEASED_ROOM_TILES) {
        tile->materials = MATERIALS;
    }
}

static void update_inactive() {
    if (gui::button("Create")) {
        STATE = EditorState::NEW_ROOM_CREATION;
    }

    ImGui::SameLine();

    gui::button_cancel(false);
}

void update_and_draw() {
    // lazy load initial materials
    static bool is_loaded = false;
    if (!is_loaded) {
        MATERIALS = tile::TileMaterials(resources::MATERIALS_PBR[0]);
        is_loaded = true;
    }

    // update depending on state
    if (editor::STATE == editor::EditorState::NEW_ROOM_CREATION) update_active();
    else update_inactive();
}

}  // namespace soft_tissues::editor::room_editor
