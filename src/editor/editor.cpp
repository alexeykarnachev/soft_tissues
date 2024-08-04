#include "editor.hpp"

#include "../camera.hpp"
#include "../globals.hpp"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raylib/raylib.h"
#include <array>
#include <cmath>
#include <functional>

namespace soft_tissues::editor {

class Tab {
public:
    std::string name;
    int key;
    std::function<void()> fn;

    Tab(std::string name, int key, std::function<void()> fn)
        : name(name)
        , key(key)
        , fn(fn) {}
};

static std::array<Tab, 2> TABS = {
    Tab("[1]Rooms", KEY_ONE, rooms_editor::update_and_draw),
    Tab("[2]Entities", KEY_TWO, entities_editor::update_and_draw),
};

static void begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void load() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    ImGui::StyleColorsDark();

    gizmo::load();
}

void unload() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    gizmo::unload();
}

static void update_and_draw_tabs() {
    static std::string active_tab_name = TABS[0].name;

    ImGui::BeginTabBar("Tabs");

    for (const auto &tab : TABS) {
        if (IsKeyPressed(tab.key)) active_tab_name = tab.name;
    }

    for (const auto &tab : TABS) {
        bool is_open = (active_tab_name == tab.name);
        int flag = is_open ? ImGuiTabItemFlags_SetSelected : 0;
        if (ImGui::BeginTabItem(tab.name.c_str(), NULL, flag)) {
            if (ImGui::IsItemActive()) active_tab_name = tab.name;
            tab.fn();
            ImGui::EndTabItem();
        }
    }

    ImGui::EndTabBar();
}

static void update_and_draw_debug() {
    ImGui::SeparatorText("Graphics");

    ImGui::Checkbox("is_light_enabled", &globals::GRAPHICS_OPTIONS.is_light_enabled);
}

void update_and_draw() {
    BeginMode3D(camera::CAMERA);
    begin();

    ImGui::Begin("Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    {
        update_and_draw_tabs();
        update_and_draw_debug();
    }
    ImGui::End();

    end();
    EndMode3D();

    gizmo::update_and_draw();
}

}  // namespace soft_tissues::editor
