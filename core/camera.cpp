#include "./camera.h"

#include <iostream>
#include <iomanip>

//Position(glm::vec3(0.f,0.f,2.f)),Front(glm::vec3(0.f, 0.f, -0.1f)), MovementSpeed(1.f), MouseSensitivity(0.5f), Zoom(45.0f) {
// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    :Position(glm::vec3(0.86f,0.49f,-1.38f)),Front(glm::vec3(-0.48f, -0.38f, 0.79f)), MovementSpeed(1.f), MouseSensitivity(.75f), Zoom(45.0f),Yaw(-238.f),Pitch(-22.f){
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

// Processes input received from any keyboard-like input system
void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position += WorldUp * velocity;
    if (direction == DOWN)
        Position -= WorldUp * velocity;
    // Aggiorna i vettori della camera
    //updateCameraVectors();
}

// Processes input received from a mouse input system
void Camera::ProcessMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

// Processes input received from a mouse scroll-wheel event
void Camera::ProcessMouseScroll(float yOffset) {
    Zoom -= yOffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    //calcolo dei vetttori ortogonali del VRF
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::PrettyPrint() {
    std::cout << "Camera Details:" << std::endl;
    std::cout << "-----------------" << std::endl;

    std::cout << "Position: ("
        << Position.x << ", "
        << Position.y << ", "
        << Position.z << ")" << std::endl;

    std::cout << "Front: ("
        << Front.x << ", "
        << Front.y << ", "
        << Front.z << ")" << std::endl;

    std::cout << "Up: ("
        << Up.x << ", "
        << Up.y << ", "
        << Up.z << ")" << std::endl;

    std::cout << "Right: ("
        << Right.x << ", "
        << Right.y << ", "
        << Right.z << ")" << std::endl;

    std::cout << "Yaw: " << Yaw << " degrees" << std::endl;
    std::cout << "Pitch: " << Pitch << " degrees" << std::endl;

    std::cout << "Movement Speed: " << MovementSpeed << std::endl;
    std::cout << "Mouse Sensitivity: " << MouseSensitivity << std::endl;
    std::cout << "Zoom: " << Zoom << std::endl;

    std::cout << "-----------------" << std::endl;
}

