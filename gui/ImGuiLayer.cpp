#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
/*
ImGuiLayer::ImGuiLayer(GLFWwindow* window) : m_Window(window) {
    // Inizializzazione di ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); // Configurazione di ImGui
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Abilita controlli tastiera
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Abilita docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Abilita multi-viewport

    // Inizializza i backend
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330"); // Specifica la versione GLSL
}

ImGuiLayer::~ImGuiLayer() {
    // Rilascio delle risorse
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin() {
    // Inizia un nuovo frame ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End() {
    // Renderizza ImGui nel contesto OpenGL
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Supporto multi-viewport
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void ImGuiLayer::Render() {
    // Aggiungi qui pannelli e controlli GUI
    ImGui::Begin("Control Panel");
    ImGui::Text("Zoom and Pan Controls");
    ImGui::End();
}
*/