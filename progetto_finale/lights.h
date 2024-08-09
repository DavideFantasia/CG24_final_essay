#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

class Light {
public:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;

    enum class TypeOfLight {
        SPOT_LIGHT,
        DIRECTIONAL_LIGHT,
        POINT_LIGHT
    };

    TypeOfLight type_of_light;

    // Constructor
    Light()
        : position(0.0f), direction(0.0f), ambient(0.2f), diffuse(0.5f), specular(0.65f),
        constant(1.0f), linear(0.09f), quadratic(0.032f),
        cutOff(glm::cos(glm::radians(12.5f))), outerCutOff(glm::cos(glm::radians(15.0f))){}

    // Static methods to initialize lights
    static Light directional_init(const glm::vec3& direction) {
        Light light;
        light.direction = direction; 
        light.position = direction;

        light.ambient = glm::vec3(0.2f);
        light.diffuse = glm::vec3(1.0f, 0.95f, 0.8f);
        light.specular = glm::vec3(1.0f, 1.0f, 1.0f);

        light.type_of_light = TypeOfLight::DIRECTIONAL_LIGHT;
        return light;
    }

    static Light pointLight_init(const glm::vec3& position) {
        Light light;

        light.ambient = glm::vec3(0.05, 0.05, 0.04);
        light.diffuse = glm::vec3(0.9, 0.8, 0.7);
        light.specular = glm::vec3(1.0, 0.9, 0.8);

        light.constant = 1.f;
        light.linear = 5.f;
        light.quadratic = 5.f;

        light.position = position;
        light.type_of_light = TypeOfLight::POINT_LIGHT;
        return light;
    }

    static Light spotlight_init(const glm::vec3& position, const glm::vec3& direction, float angle_cutoff, float angle_outerCutOff) {
        Light light;

        light.ambient = glm::vec3(0.05, 0.05, 0.04);
        light.diffuse = glm::vec3(0.9, 0.8, 0.7);
        light.specular = glm::vec3(1.0, 0.9, 0.8);

        light.position = position;
        light.direction = direction;
        light.cutOff = glm::cos(glm::radians(angle_cutoff));
        light.outerCutOff = glm::cos(glm::radians(angle_outerCutOff));
        light.type_of_light = TypeOfLight::SPOT_LIGHT;
        return light;
    }

    // Update methods
    void update_position(const glm::vec3& newPos) {
        position = newPos;
    }

    void update_direction(const glm::vec3& newDir) {
        direction = newDir;
    }

    void rotate_direction(float angle) {
        // Calcolo della direzione iniziale del sole senza rotazione
        glm::vec3 sun_direction = glm::normalize(glm::vec3(cos(glm::radians(angle)), sin(glm::radians(angle)), 0.0f));

        // Creazione della matrice di rotazione di 90° sull'asse y
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        // Applicazione della rotazione alla direzione del sole
        glm::vec4 rotated_sun_direction = rotation * glm::vec4(sun_direction, 1.0f);

        // Aggiornamento della direzione del sole
        direction = glm::vec3(rotated_sun_direction);
    }

    // Set uniform method
    void set_uniform(GLuint shaderProgram) {
        std::string uniformNameBase;
        switch(type_of_light){
            case(TypeOfLight::SPOT_LIGHT): uniformNameBase = "spotlight"; set_spotlight_uniforms(shaderProgram, uniformNameBase); break;
            case(TypeOfLight::DIRECTIONAL_LIGHT): uniformNameBase = "dirLight"; set_directional_uniforms(shaderProgram, uniformNameBase); break;
            case(TypeOfLight::POINT_LIGHT): uniformNameBase = "pointLight"; set_pointlight_uniforms(shaderProgram, uniformNameBase); break;
            default: exit(-2);
        }
    }

private:
    void set_directional_uniforms(GLuint shaderProgram, const std::string& uniformNameBase) {
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".direction").c_str()), 1, glm::value_ptr(direction));

        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".specular").c_str()), 1, glm::value_ptr(specular));
    }

    void set_pointlight_uniforms(GLuint shaderProgram, const std::string& uniformNameBase){
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".position").c_str()), 1, glm::value_ptr(position));

        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".specular").c_str()), 1, glm::value_ptr(specular));

        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".constant").c_str()), constant);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".quadratic").c_str()), quadratic);
    }

    void set_spotlight_uniforms(GLuint shaderProgram, const std::string& uniformNameBase) {
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".position").c_str()), 1, glm::value_ptr(position));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".direction").c_str()), 1, glm::value_ptr(direction));

        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".specular").c_str()), 1, glm::value_ptr(specular));

        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".constant").c_str()), constant);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".linear").c_str()), linear);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".quadratic").c_str()), quadratic);

        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".cutOff").c_str()), cutOff);
        glUniform1f(glGetUniformLocation(shaderProgram, (uniformNameBase + ".outerCutOff").c_str()), outerCutOff);
    }
};
