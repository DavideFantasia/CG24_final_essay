#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <functional>

class Window {
public:
    using ResizeCallback = std::function<void(int, int)>;

    Window(int width, int height, const std::string& title)
        : width(width), height(height), title(title), window(nullptr) {}

    ~Window() {
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    bool init() {
        // Inizializza GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Crea la finestra
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        // Imposta il contesto OpenGL
        glfwMakeContextCurrent(window);

        // Inizializza GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            return false;
        }

        // Imposta il callback per il ridimensionamento della finestra
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Imposta il puntatore al contesto della finestra
        glfwSetWindowUserPointer(window, this);

        // Abilita il V-Sync
        glfwSwapInterval(1);

        return true;
    }

    bool shouldClose() const {
        return glfwWindowShouldClose(window);
    }

    void pollEvents() const {
        glfwPollEvents();
    }

    void swapBuffers() const {
        glfwSwapBuffers(window);
    }

    GLFWwindow* getGLFWwindow() const {
        return window;
    }

    void setResizeCallback(ResizeCallback callback) {
        resizeCallback = callback;
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

private:
    GLFWwindow* window;
    int width, height;
    std::string title;
    ResizeCallback resizeCallback;

    static void framebuffer_size_callback(GLFWwindow* window, int newWidth, int newHeight) {
        auto* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (instance) {
            instance->width = newWidth;
            instance->height = newHeight;

            // Chiama il callback se è stato impostato
            if (instance->resizeCallback) {
                instance->resizeCallback(newWidth, newHeight);
            }

            // Aggiorna la viewport di OpenGL
            glViewport(0, 0, newWidth, newHeight);
        }
    }
};

#endif // WINDOW_H
