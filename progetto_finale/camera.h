#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

enum CameraMovement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front; //-z
    glm::vec3 Up; //y
    glm::vec3 Right; //x
    glm::vec3 WorldUp; //y'

    // Euler Angles
    float Yaw;
    float Pitch;

    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(2.19f, 0.5f, 1.46f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -138.15f, float pitch = -4.95f);

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();

    // switch (set) camera position to the indicated one
    void SwitchPosition(glm::vec3 newPosition);

    // Processes input received from any keyboard-like input system blocking the camera to the terrain map used
    void ProcessKeyboard(CameraMovement direction, float deltaTime);

    // Processes input received from a mouse input system
    void ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch = true);

    // Processes input received from a mouse scroll-wheel event
    void ProcessMouseScroll(float yOffset);

    //return y from heightmap, given x and z
    float TerrainHeight(float x, float z);

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};