#ifndef IMGUI_LAYER_H
#define IMGUI_LAYER_H

#include <GLFW/glfw3.h>

class ImGuiLayer {
public:
    ImGuiLayer(GLFWwindow* window);
    ~ImGuiLayer();

    void Begin(); // Inizio di un frame ImGui
    void End();   // Fine di un frame ImGui
    void Render(); // Rendering della GUI

private:
    GLFWwindow* m_Window; // Puntatore alla finestra GLFW
};

#endif // IMGUI_LAYER_H
