#include "editor.hpp"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raylib/raylib.h"
#include "resources.hpp"
#include "tile.hpp"
#include "world.hpp"
#include <cmath>
#include <unordered_set>

namespace soft_tissues::editor {

using namespace utils;

enum class EditorState {
    NONE,
    NEW_ROOM_CREATION,
};

class NewRoom {
private:
    std::unordered_set<tile::Tile *> tiles;

public:
    tile::TileMaterials materials;

    NewRoom() {}

    bool can_add_tile(tile::Tile *tile) {
        if (!tile->is_empty()) return false;
        if (this->tiles.size() == 0) return true;

        auto neighbors = world::get_tile_neighbors(tile->get_id());
        bool has_neighbor = false;
        for (auto tile : neighbors) {
            has_neighbor |= this->tiles.count(tile) == 1;
        }

        if (!has_neighbor) return false;

        return true;
    }

    void reset() {
        for (auto tile : this->tiles) {
            tile->flags = 0;
        }
        *this = NewRoom();
    }

    void add_tile(tile::Tile *tile) {
        tile->materials = this->materials;

        if (!this->can_add_tile(tile)) return;

        world::set_room_tile_flags(tile);
        auto neighbors = world::get_tile_neighbors(tile->get_id());

        for (auto nb : neighbors) {
            if (nb && !nb->is_empty()) {
                nb->materials = this->materials;
                world::set_room_tile_flags(nb);
            }
        }

        world::set_room_tile_flags(tile);
        this->tiles.insert(tile);
    }

    const std::unordered_set<tile::Tile *> get_tiles() {
        return this->tiles;
    }
};

static const ImVec4 COLOR_CANCEL = {0.8, 0.2, 0.1, 1.0};
static const ImVec4 COLOR_ACCEPT = {0.2, 0.8, 0.1, 1.0};

static Material FREE_TILE_MATERIAL;
static Material OCCUPIED_TILE_MATERIAL;
static Material ADDED_TILE_MATERIAL;

static int ID = 0;
static EditorState STATE;
static NewRoom NEW_ROOM;

static void push_id() {
    ImGui::PushID(ID++);
}

static void pop_id() {
    ImGui::PopID();
}

static void reset_id() {
    ID = 0;
}

static bool collapsing_header(const char *name, bool is_opened) {
    int flags = is_opened ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    return ImGui::CollapsingHeader(name, flags);
}

static bool collapsing_header(const char *name) {
    return collapsing_header(name, true);
}

static void begin() {
    reset_id();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static bool button(const char *name, bool is_enabled = true) {
    push_id();
    if (!is_enabled) ImGui::BeginDisabled();
    bool is_clicked = ImGui::Button(name);
    if (!is_enabled) ImGui::EndDisabled();
    pop_id();

    return is_clicked;
}

static bool button_color(const char *name, ImVec4 color, bool is_enabled = true) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    bool is_clicked = button(name, is_enabled);
    ImGui::PopStyleColor();

    return is_clicked;
}

static bool button_cancel(bool is_enabled = true) {
    return button_color("Cancel", COLOR_CANCEL, is_enabled);
}

// static bool button_accept(bool is_enabled = true) {
//     return button_color("Accept", COLOR_ACCEPT, is_enabled);
// }

static void update_rooms_inspector() {
    if (STATE == EditorState::NEW_ROOM_CREATION) {
        if (button_cancel()) {
            STATE = EditorState::NONE;
            NEW_ROOM.reset();
        }
    } else {
        if (button("Create")) {
            STATE = EditorState::NEW_ROOM_CREATION;
        }

        ImGui::SameLine();

        button_cancel(false);
    }
}

static void update_new_room_creation() {
    tile::Tile *tile = world::get_tile_at_cursor();

    // ghost tile
    if (tile) {
        bool can_add_tile = NEW_ROOM.can_add_tile(tile);

        Matrix matrix = tile->get_floor_matrix();
        Material material = can_add_tile ? FREE_TILE_MATERIAL : OCCUPIED_TILE_MATERIAL;
        DrawMesh(resources::PLANE_MESH, material, matrix);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            NEW_ROOM.add_tile(tile);
        }
    }

    // new room tiles
    for (auto tile : NEW_ROOM.get_tiles()) {
        Matrix matrix = tile->get_floor_matrix();
        DrawMesh(resources::PLANE_MESH, ADDED_TILE_MATERIAL, matrix);
    }
}

void load() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    ImGui::StyleColorsDark();

    FREE_TILE_MATERIAL = LoadMaterialDefault();
    FREE_TILE_MATERIAL.maps[0].color = GREEN;

    OCCUPIED_TILE_MATERIAL = LoadMaterialDefault();
    OCCUPIED_TILE_MATERIAL.maps[0].color = RED;

    ADDED_TILE_MATERIAL = LoadMaterialDefault();
    ADDED_TILE_MATERIAL.maps[0].color = RAYWHITE;

    NEW_ROOM.reset();
}

void unload() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    UnloadMaterial(FREE_TILE_MATERIAL);
    UnloadMaterial(OCCUPIED_TILE_MATERIAL);
    UnloadMaterial(ADDED_TILE_MATERIAL);
}

void update_and_draw() {
    begin();

    if (IsKeyPressed(KEY_ESCAPE)) STATE = EditorState::NONE;

    ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    {
        if (collapsing_header("Rooms")) update_rooms_inspector();
    }
    ImGui::End();

    if (STATE == EditorState::NEW_ROOM_CREATION) update_new_room_creation();

    end();
}

}  // namespace soft_tissues::editor
