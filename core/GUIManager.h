#pragma once

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "../fractals/mandelbrot.h"
#include "../fractals/julia.h"
#include "../fractals/sierpinski.h"
#include "../fractals/koch.h"
#include "../fractals/barnsley.h"
#include "../fractals/newton.h"

#include "../fractals/mandelbulb.h"

class GUIManager {
public:
    static void init(GLFWwindow* window);
    static void render();
    static void cleanUp();

    static bool is_3d_enabled();
    static Fractal& get_selected_fractal();

private:
    static int selected_fractal;
    static bool is_3d;
    //lista dei frattali disponibili
    static std::vector<std::unique_ptr<Fractal>> fractals;
};

