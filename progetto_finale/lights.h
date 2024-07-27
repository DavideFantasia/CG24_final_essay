#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Light{
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

	void init_bulb(glm::vec3 pos) {
		constant = 1.f;
		linear = 0.045f;
		quadratic = 0.0075f;

		position = pos;
		direction = -pos;
	}

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


};