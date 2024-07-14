#include "editor.hpp"

#include "GLFW/glfw3.h"
#include "camera.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "resources.hpp"
#include "world.hpp"
#include <cmath>

namespace soft_tissues::editor {

static int ID = 0;

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

static void update_rooms() {
    ImGui::Button("New Room");

    Vector2 mouse_position = GetMousePosition();
    Ray ray = GetScreenToWorldRay(mouse_position, camera::CAMERA);
    Rectangle world_rect = world::get_bound_rect();
    Vector3 p1 = {world_rect.x, 0.0, world_rect.y};
    Vector3 p2 = {p1.x + world_rect.width, 0.0, p1.y};
    Vector3 p3 = {p1.x + world_rect.width, 0.0, p1.y + world_rect.height};
    Vector3 p4 = {world_rect.x, 0.0, p1.y + world_rect.height};

    auto collision = GetRayCollisionQuad(ray, p1, p2, p3, p4);
    if (collision.hit) {
        Matrix matrix = MatrixTranslate(
            std::floor(collision.point.x) + 0.5f,
            0.0f,
            std::floor(collision.point.z) + 0.5f
        );
        DrawMesh(resources::PLANE_MESH, resources::DEFAULT_MATERIAL, matrix);
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
}

void unload() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void update_and_draw() {
    begin();

    ImGui::Begin("Inspector", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    {
        if (collapsing_header("Rooms")) update_rooms();
    }
    ImGui::End();

    end();
}

}  // namespace soft_tissues::editor
