#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>  
#include <glm/ext.hpp>  

/*
* Per poter usare questa classe bisogna instanziare la classe SceneRotator dopo aver creato la GLFWWindow e dopo averla impostata
* come contesto attuale
* Successivamente vanno settati i metodi di callback:
* `
    SceneRotator sceneRotator;
    glfwSetWindowUserPointer(window, &sceneRotator);
    glfwSetCursorPosCallback(window, SceneRotator::mouseCallback);
    glfwSetMouseButtonCallback(window, SceneRotator::mouseButtonCallback);
* `
* 
* Nel DrawLoop va moltiplicata la root della scena per la matrice di rotazione data da
* glm::mat4 RotationMatrix = sceneRotator.getRotationMatrix();
*/

class SceneRotator {
private:
    bool leftMouseButtonPressed;
    double lastX, lastY;
    float rotationSpeed;
    glm::vec3 rotationAxis;
    glm::mat4 rotationMatrix;

public:
    SceneRotator() : leftMouseButtonPressed(false), lastX(0.0), lastY(0.0), rotationSpeed(0.015f), rotationAxis(0.0f, 1.0f, 0.0f), rotationMatrix(1.0f) {}

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        SceneRotator* sceneRotator = static_cast<SceneRotator*>(glfwGetWindowUserPointer(window));
        if (sceneRotator) {
            sceneRotator->handleMouseMovement(xpos, ypos);
        }
    }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        SceneRotator* sceneRotator = static_cast<SceneRotator*>(glfwGetWindowUserPointer(window));
        if (sceneRotator) {
            sceneRotator->handleMouseButton(window, button, action, mods);
        }
    }

    /*
    * Returna la matrice di rotazione da applicare alla scena per poterla ruotare con il muose
    */
    glm::mat4 getRotationMatrix() const {
        return glm::inverse(rotationMatrix);
    }

private:
    /*
    * Non lancia propriamente un raycast per mappare il mouse sull'oggetto
    * Praticamente se si muove il mouse, la callback chiama questa funzione, se si preme il tasto, si misura la "velocità" di cambiamento
    * nelle coordinate del puntatore e la si usa per aggiornare la matrice di rotazione
    */
    void handleMouseMovement(double xpos, double ypos) {
        if (leftMouseButtonPressed) {
            //calcolo della differenza di posizione
            double deltaX = xpos - lastX;
            double deltaY = ypos - lastY;
            //aggiornamento della matrice di rotazione
            /*si ruota di - deltaX * rotationSpeed perché in OpenGL 
            * la rotazione avviene in senso antiorario quando l'angolo di rotazione è positivo e viceversa
            * Essendo che tipo deltaY aumenta muovendo il mouse verso destra, e quindi vogliamo che l'asse Y ruoti in senso anti-orario
            * moltiplichiamo per -1 per avere l'inverso
            */
            rotationMatrix = glm::rotate(rotationMatrix, (float)(-deltaX * rotationSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
            rotationMatrix = glm::rotate(rotationMatrix, (float)(-deltaY * rotationSpeed), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        lastX = xpos;
        lastY = ypos;
    }

    void handleMouseButton(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS){
                leftMouseButtonPressed = true;
                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR)); //mette il cursore della mano
            }
            else if (action == GLFW_RELEASE) {
                leftMouseButtonPressed = false;
                glfwSetCursor(window, nullptr); // Reimposta il cursore predefinito quando il tasto del mouse viene rilasciato
            }
        }
    }
};