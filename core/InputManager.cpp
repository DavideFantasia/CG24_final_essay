#include "InputManager.h"

// Definizione del puntatore statico
InputManager* InputManager::instance = nullptr;

//costruttore
InputManager::InputManager(GLFWwindow* window) : camera(Camera()){
    this->window = window;
    
    // Imposta le callback di GLFW
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Imposta un puntatore statico all'istanza di InputManager per utilizzarlo nelle callback
    instance = this;
    instance->updateProjMatrix(window); //inizializzazione della matrice di proieione

    // Imposta la modalità iniziale a 2D
    currentMode = Mode::MODE_2D;
}

void InputManager::setMode(Mode mode) {
    instance->currentMode = mode;
}

/**
* Funzione di smistamento della strategia 2D/3D sulla base del discriminante (currentMode)
**/
void InputManager::update() {
    // Gestisci la modalità 2D
    switch (currentMode) {
        case Mode::MODE_2D:
            update2D();
            break;
        case Mode::MODE_3D:
            update3D();
            break;
        default:
            std::cout << "errore nella gestione della modalità di input"<<std::endl;
            exit(1);
    }
}

void InputManager::update2D() {
    // Implementazione dello zoom tramite scroll del mouse
    if (scrollZoom < 0) {
        zoom *= 1.1f;  // Zoom in
    }
    else if (scrollZoom > 0) {
        zoom *= 0.9f;  // Zoom out
    }

    // Limita il range dello zoom
    zoom = std::max(0.0000001f, std::min(zoom, 100.0f));

    // Resetta lo scrollZoom
    scrollZoom = 0;

    // Se ImGui cattura il mouse, non fare nulla (non aggiornare il panning)
    if (ImGui::GetIO().WantCaptureMouse) { return; }
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    // Calcola la differenza rispetto alla posizione precedente del mouse
   
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {  // Mouse sinistro premuto per panning
        float deltaX = xpos - cursorX;
        float deltaY = ypos - cursorY;
        float deltaTime = ImGui::GetIO().DeltaTime;
        // Aggiorna la posizione del centro del frattale in base al movimento del mouse
        // `deltaX` e `deltaY` determinano il panning
        // Puoi scegliere una scala per quanto il mouse sposta la visualizzazione
        float panningVelocity = 0.75f * zoom * deltaTime;
        panningX = deltaX * panningVelocity;
        panningY = deltaY * panningVelocity;

        // Aggiorna la posizione del cursore
        cursorX = xpos;
        cursorY = ypos;
    }
    else {
        panningX = 0;
        panningY = 0;
    }
}

void InputManager::update3D() {
    float deltaTime = ImGui::GetIO().DeltaTime;

    // Tasti di movimento della telecamera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::DOWN, deltaTime);

    // Se ImGui cattura il mouse, non fare nulla (non aggiornare il panning)
    if (ImGui::GetIO().WantCaptureMouse) { return; }

    // Calcola la differenza rispetto alla posizione precedente del mouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        float xOffset = xpos - instance->cursorX;
        float yOffset = instance->cursorY - ypos;

        instance->cursorX = xpos;
        instance->cursorY = ypos;

        instance->camera.ProcessMouseMovement(xOffset, yOffset);
    }
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    // Cambio della modalità 2D/3D premendo il tasto "M"
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        if (instance->currentMode == Mode::MODE_2D) {
            instance->currentMode = Mode::MODE_3D;
            std::cout << "Switched to 3D mode!" << std::endl;
        }
        else {
            instance->currentMode = Mode::MODE_2D;
            std::cout << "Switched to 2D mode!" << std::endl;
        }
    }
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (ImGui::GetIO().WantCaptureMouse) return;

    if (instance) {
        // Passa la posizione del mouse all'istanza
        instance->cursorX = xpos;
        instance->cursorY = ypos;
    }
}

/*
* Callback che gestice lo scroll del mouse aggiornando lo zoom, se l'input è 3D allora aggiorna anche la matrice di proiezione
*/
void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    if (ImGui::GetIO().WantCaptureMouse) return;
    if (instance) {
        instance->scrollZoom = yoffset;  // Assegna il valore dello scroll alla variabile scrollZoom
        instance->camera.ProcessMouseScroll(yoffset);

        if (instance->currentMode == InputManager::Mode::MODE_3D)
            instance->updateProjMatrix(window);
    }
}

/*
* Aggiorna la matrice di proiezione sulla base della dimensione della finestra e del zoom, restituisce una glm::mat4 che rappresenta
* la matrice di proiezione
*/
glm::mat4 InputManager::updateProjMatrix(GLFWwindow* window) {
    glfwGetWindowSize(window, &instance->window_width, &instance->window_height);
    instance->projection_matrix =
        glm::perspective(glm::radians(instance->camera.Zoom), instance->window_width / float(instance->window_height), 0.0001f, 2.f);
    return instance->getProjectionMatrix();
}
