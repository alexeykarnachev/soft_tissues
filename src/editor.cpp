#include "editor.hpp"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raylib/raylib.h"

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

static void update_camera() {
    push_id();
    ImGui::Text("TODO");
    pop_id();
}

void load() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 420 core");
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
        if (collapsing_header("Camera")) update_camera();
    }
    ImGui::End();

    end();
}

}  // namespace soft_tissues::editor
