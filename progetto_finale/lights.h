#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

/*struct Light {
	glm::vec3 position;
	glm::vec3 direction;
	//colore rgb (normalizato) della componente ambientale
	glm::vec3 ambient;
	//colore rgb (normalizato) della componente diffusiva
	glm::vec3 diffuse;
	//colore rgb (normalizato) della componente speculare
	glm::vec3 specular;

	//termini per l'attenuazione della luce puntiforme
	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;

	void init_spotLight(glm::vec3 pos, glm::vec3 dir, float angle_cutoff, float angle_outerCutOff) {
		//implementazione parametri spotlight
		position = pos;
		direction = dir;
		cutOff = glm::cos(glm::radians(angle_cutoff));
		outerCutOff = glm::cos(glm::radians(angle_outerCutOff));
	}

	void init_directional(glm::vec3 dir) {
		direction = -dir;
		position = dir;
	}


};*/

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

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

    bool is_spotlight;

    // Constructor
    Light()
        : position(0.0f), direction(0.0f), ambient(0.2f), diffuse(0.5f), specular(0.65f),
        constant(1.0f), linear(0.09f), quadratic(0.032f),
        cutOff(glm::cos(glm::radians(12.5f))), outerCutOff(glm::cos(glm::radians(15.0f))),
        is_spotlight(false) {}

    // Static methods to initialize lights
    static Light directional_init(const glm::vec3& direction) {
        Light light;
        light.direction = -direction; 
        light.position = direction;

        light.is_spotlight = false;
        return light;
    }

    static Light spotlight_init(const glm::vec3& position, const glm::vec3& direction, float angle_cutoff, float angle_outerCutOff) {
        Light light;
        light.position = position;
        light.direction = direction;
        light.cutOff = glm::cos(glm::radians(angle_cutoff));
        light.outerCutOff = glm::cos(glm::radians(angle_outerCutOff));
        light.is_spotlight = true;
        return light;
    }

    // Update methods
    void update_position(const glm::vec3& newPos) {
        position = newPos;
    }

    void update_direction(const glm::vec3& newDir) {
        direction = newDir;
    }

    // Set uniform method
    void set_uniform(GLuint shaderProgram) {
        std::string uniformNameBase;
        if (is_spotlight) {
            uniformNameBase = "spotlight";
            set_spotlight_uniforms(shaderProgram, uniformNameBase);
        }
        else {
            uniformNameBase = "dirLight";
            set_directional_uniforms(shaderProgram, uniformNameBase);
        }
    }

private:
    void set_directional_uniforms(GLuint shaderProgram, const std::string& uniformNameBase) {
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".direction").c_str()), 1, glm::value_ptr(direction));

        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".ambient").c_str()), 1, glm::value_ptr(ambient));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".diffuse").c_str()), 1, glm::value_ptr(diffuse));
        glUniform3fv(glGetUniformLocation(shaderProgram, (uniformNameBase + ".specular").c_str()), 1, glm::value_ptr(specular));
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
