#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp> 

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "../rendering/Shader.h"
#include "../core/InputManager.h"


class Fractal {
public:
    bool is_3D = false;

    virtual ~Fractal() = default;

    // Metodo per aggiornare i parametri dei frattali
    virtual void updateParameters(InputManager inputManager) = 0;

    // Metodo per impostare le uniform
    virtual void setUniform() = 0;

    // Metodo per disegnare i controlli GUI specifici del singolo frattale
    virtual void renderGUI() = 0;

    // Restituisce lo shader program associato
    virtual GLuint getShaderProgram() const = 0;

    virtual void reset_param() = 0;

    const char* getName() const { return name; }
    //true se il frattale è di tipo tridimensionale, false altrimenti
    bool get_3D(){ return is_3D; }

protected:
    const char* name;
};