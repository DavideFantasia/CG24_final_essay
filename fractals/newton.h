#pragma once

#include "Fractal.h"

class Newton : public Fractal {
public:
    Newton() {
        shader.create_program("fractals/quad.vert", "fractals/fragment/newton.frag");
        name = "Newton";
        animationStartTime = 0.0f;
    }

    void updateParameters(InputManager inputManager) override {
        zoom = inputManager.getZoom2D();
        center.x -= inputManager.getPanningX();
        center.y += inputManager.getPanningY();
    }

    void setUniform() override {
        shader.use();
        float elapsedTime = ImGui::GetTime() - animationStartTime;
        glUniform1f(glGetUniformLocation(shader.program, "uZoom"), zoom);
        glUniform2fv(glGetUniformLocation(shader.program, "uCenter"), 1, &center[0]);
        glUniform1f(glGetUniformLocation(shader.program, "uTime"), elapsedTime);
        glUniform1i(glGetUniformLocation(shader.program, "uMaxIterations"), maxIterations);
    }

    void renderGUI() override {
        ImGui::SetNextItemWidth(ImGui::GetWindowWidth()*0.2);
        ImGui::SliderInt("Max Iterations", &maxIterations, 1, 50);
    }

    GLuint getShaderProgram() const override {
        return shader.program;
    }

    void reset_param() override {
        center = glm::vec2(0.33f, 0.33f);
        zoom = 1.5f;
        InputManager::getInstance()->setZoom2D(zoom);
        animationStartTime = ImGui::GetTime();
    }

private:
    Shader shader;
    glm::vec2 center = glm::vec2(-0.5f, 0.25f);
    float zoom = 1.f;
    float animationStartTime;  // Tempo di inizio animazione
    int maxIterations;
};
