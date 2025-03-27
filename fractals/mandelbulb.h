#pragma once

#include "Fractal.h"

class MandelBulb : public Fractal {
public:
    MandelBulb() {
        shader.create_program("fractals/quad.vert", "fractals/fragment/mandelbulb.frag");
        name = "Mandelbulb";
        animationStartTime = 0.0f;
        is_3D = true;
    }

    void updateParameters(InputManager inputManager) override {
        camPos = inputManager.camera.Position;
        view = inputManager.camera.GetViewMatrix();
    }

    void setUniform() override {
        shader.use();
        
        if (is_animating) {
            // Se riprendiamo l'animazione, aggiorniamo animationStartTime in modo che elapsedTime non salti
            if (pausedTime > 0.0f) {
                animationStartTime += ImGui::GetTime() - pausedTime;
                pausedTime = 0.0f;  // Reset del tempo di pausa
            }
            elapsedTime = ImGui::GetTime() - animationStartTime;
        }
        else {
            if (pausedTime == 0.0f) {
                pausedTime = ImGui::GetTime();  // Memorizza il tempo in cui è stato fermato
            }
        }

        glUniform1f(glGetUniformLocation(shader.program, "uTime"), elapsedTime);

        glUniformMatrix4fv(glGetUniformLocation(shader.program, "uView"), 1, GL_FALSE, &view[0][0]);
        glUniform3fv(glGetUniformLocation(shader.program, "uCamPos"), 1, &camPos[0]);
    }

    void renderGUI() override {
        if (ToggleButton("Animation", &is_animating)) { ; }
    }

    GLuint getShaderProgram() const override {
        return shader.program;
    }

    void reset_param() override {
        animationStartTime = ImGui::GetTime();
    }

private:
    Shader shader;
    glm::vec2 center = glm::vec2(0.0f, 0.0f);
    float zoom = 1.0f;
    //parametri per fermare o far ripartire l'incremento della potenza del mandelbulb
    float animationStartTime = 0.0f;
    float elapsedTime = 0.0f;
    float pausedTime = 0.0f;  // Tempo in cui è stato in pausa
    bool is_animating = false;
    //glm::vec2 mouse;
    glm::mat4 view;
    glm::vec3 camPos;
    /*
    * Bottone per far riprendere l'animazione dell'incremento di potenza del frattale
    */
    bool ToggleButton(const char* label, bool* v) {
        if (ImGui::Button(*v ? "STOP" : "PLAY")) {
            *v = !*v;
            return true;
        }
        return false;
    }
};
