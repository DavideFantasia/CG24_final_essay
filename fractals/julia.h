#pragma once

#include "Fractal.h"
#include <vector>
#include <imgui.h>
#include <glm/glm.hpp>

class Julia : public Fractal {
public:
    Julia() {
        shader.create_program("fractals/quad.vert", "fractals/fragment/julia.frag");
        name = "Julia";
        selectedConstantIndex = 3;  // Inizializza l'indice con il valore predefinito
    }

    void updateParameters(InputManager inputManager) override {
        zoom = inputManager.getZoom2D();
        center.x -= inputManager.getPanningX();
        center.y += inputManager.getPanningY();
    }

    void setUniform() override {
        shader.use();
        glUniform1f(glGetUniformLocation(shader.program, "uZoom"), zoom);
        glUniform2fv(glGetUniformLocation(shader.program, "uCenter"), 1, &center[0]);
        glUniform2fv(glGetUniformLocation(shader.program, "uCostant"), 1, &juliaConstant[0]);
        glUniform1f(glGetUniformLocation(shader.program, "uTime"), (float)ImGui::GetTime());
    }

    void renderGUI() override {
        // Menu a tendina per selezionare le costanti
        if (ImGui::BeginMenu("Select Julia's Constant:")) {
            for (int i = 0; i < juliaConstants.size(); i++) {
                if (ImGui::Selectable(labels[i], selectedConstantIndex == i)) {
                    selectedConstantIndex = i;  // Aggiorna l'indice selezionato
                    juliaConstant = juliaConstants[selectedConstantIndex];  // Aggiorna il valore della costante
                    reset_param();
                }
            }
            ImGui::EndMenu();
        }
    }

    GLuint getShaderProgram() const override {
        return shader.program;
    }

    void reset_param() override {
        zoom = 1.5f;
        InputManager::getInstance()->setZoom2D(zoom);
        center = juliaConstant;
    }

private:
    Shader shader;
    glm::vec2 center = glm::vec2(0.5f, 0.5f);
    glm::vec2 juliaConstant = glm::vec2(-0.7f, 0.27015f);
    float zoom = 1.0f;

    int selectedConstantIndex = 3;  // Indice della costante selezionata
    
    // Lista di costanti di Julia
    const std::vector<glm::vec2> juliaConstants = {
        glm::vec2(-0.7176f, -0.3842f),
        glm::vec2(-0.4f, -0.59f),
        glm::vec2(0.34f, -0.05f),
        glm::vec2(0.355f, 0.355f),
        glm::vec2(-0.54f, 0.54f),
        glm::vec2(0.355534f, -0.337292f)
    };
    // Etichette per le costanti
    const char* labels[6] = {
        "-0.7176, -0.3842",
        "-0.4, -0.59",
        "0.34, -0.05",
        "0.355, 0.355",
        "-0.54, 0.54",
        "0.355534, -0.337292"
    };
};
