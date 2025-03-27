#include "GUIManager.h"

int GUIManager::selected_fractal = 0;
bool GUIManager::is_3d = false;
std::vector<std::unique_ptr<Fractal>> GUIManager::fractals;

void GUIManager::init(GLFWwindow* window) {
    if (window) { std::cerr << "GLFW window is null!" << std::endl; }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    ImGui::GetStyle().ScaleAllSizes(2.f); // Ingrandisce tutti i widget del 50%
    //2D
    fractals.push_back(std::make_unique<Mandelbrot>());
    fractals.push_back(std::make_unique<Julia>());
    fractals.push_back(std::make_unique<Sierpinski>());
    fractals.push_back(std::make_unique<Koch>());
    fractals.push_back(std::make_unique<Barnsley>());
    fractals.push_back(std::make_unique<Newton>());
    //3D
    fractals.push_back(std::make_unique<MandelBulb>());
}

void GUIManager::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::BeginMainMenuBar();

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    if (ImGui::BeginMenu("Fractal's List")) {
        //si itera sulla lista di frattali disponibili, mostrando il bottone per selezionarlo
        for (int i = 0; i < fractals.size(); i++) {
            if (ImGui::Selectable(fractals[i]->getName(), selected_fractal == i)) {
                selected_fractal = i;
                fractals[selected_fractal]->reset_param();
                is_3d = fractals[selected_fractal]->get_3D();
            }
        }
        ImGui::EndMenu();
    }

    fractals[selected_fractal]->renderGUI();

    ImGui::EndMainMenuBar();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::cleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool GUIManager::is_3d_enabled() { return is_3d; }
Fractal& GUIManager::get_selected_fractal() { return *fractals[selected_fractal]; }


