#pragma once

#include "Fractal.h"

class Mandelbrot : public Fractal {
public:
    Mandelbrot(){
        shader.create_program("fractals/quad.vert", "fractals/fragment/mandelbrot.frag");
        name = "Mandelbrot";
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
    }

    void renderGUI() override {
        ;
    }

    GLuint getShaderProgram() const override {
        return shader.program;
    }

    void reset_param() override {
        center = glm::vec2(-0.5f, 0.2f);
        zoom = 1.5f;
        InputManager::getInstance()->setZoom2D(zoom);
    }

private:
    Shader shader;
    glm::vec2 center = glm::vec2(-0.5f, 0.2f);
    float zoom = 1.0f;
};
