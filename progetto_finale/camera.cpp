#include "./camera.h"

// Constructor with vectors
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.075f), Zoom(45.0f) {
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

void Camera::SwitchPosition(glm::vec3 newPosition){
    Position = newPosition;
    updateCameraVectors();
}

float Camera::TerrainHeight(float x, float z) {
    /*
    int mapped_x = static_cast<int>((x + 1.f) / 2.0f * (heightmap.x_size - 1));
    int mapped_z = static_cast<int>((z + 1.f) / 2.0f * (heightmap.y_size - 1));

    if (mapped_x < 0) mapped_x = 0;
    if (mapped_z < 0) mapped_z = 0;

    if (mapped_x >= heightmap.x_size) mapped_x = heightmap.x_size;
    if (mapped_z >= heightmap.y_size) mapped_z = heightmap.y_size;

    int index = (mapped_z * heightmap.x_size + mapped_x) * heightmap.n_components;

    return heightmap.pixelData[index] / 255.f;
    */
    return 1.f;
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
        Position += Up * velocity;
    if (direction == DOWN)
        Position -= Up * velocity;

    // Imposta la posizione Y della camera sulla superficie del terreno
    Position.y = TerrainHeight(Position.x, Position.z);

    // Aggiorna i vettori della camera
    updateCameraVectors();
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

