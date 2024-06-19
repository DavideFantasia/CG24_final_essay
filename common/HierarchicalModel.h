#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <time.h>

#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"

class HierarhicalModel {
public:
	HierarhicalModel() {
		;
	}

	void createHierarhicalModel(shader input_shader, renderable r ,glm::mat4 local_matrix=glm::mat4(1.f)){
		basic_shader = input_shader;
		model_matrix = local_matrix;
		model_r = r;
	}

	void addChild(HierarhicalModel child) {
		children.push_back(child);
	}

	void set_renderable(renderable r) {
		model_r = r;
	}
	/*
	* Funzione che disegna il modello ricorsivamente
	*/
	void render(glm::mat4 transformation=glm::mat4(1.f)) {
		render_model(transformation);
		if (children.size() > 0) {
			for (int i = 0; i < children.size(); i++) {
				children[i].render(transformation * model_matrix);
			}
		}
		return;
	}
private:
	void render_model(glm::mat4 parent_model_matrix=glm::mat4(1.f)) {
		model_r.bind();
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &(parent_model_matrix * model_matrix)[0][0]);
		glDrawElements(model_r().mode, model_r().count, model_r().itype, NULL);
		return;
	}
	renderable model_r;
	float angle = 0;
	glm::mat4 model_matrix = glm::mat4(1.f);
	shader basic_shader;
	std::vector<HierarhicalModel> children;

};