#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>

#include <thread>

#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"

//gestione delle texture

#include "..\common\texture.h"


#include "./terrainGenerator.h"
#include "./camera.h"

/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>


int width = 1000, height = 800;

// Callback function for mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// Callback function for mouse button
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// Callback function for mouse scroll
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Process all input
void processInput(GLFWwindow* window);

// Camera
//array contenente le posizioni migliori per la camera
glm::vec3 cameraPositions[2] = { glm::vec3(0.0f, 0.0f, 3.0f) , glm::vec3(0.0f, 1.0f, 0.0f) };
Camera camera(cameraPositions[0]);
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;
bool isLeftMouseButtonPressed = false;
// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

/* light direction */
glm::vec4 Ldir;

/* (light) projector */
struct projector {
	glm::mat4 view, proj;
	texture tex;
};

projector Lproj;

/* program shaders used */
shader heightmap_shader, flat_shader, texture_shader;

/* object that will be rendered in this scene*/
renderable r_terrain;
// cubo che funge da cubemap
renderable r_cube;
void draw_large_cube() {
	r_cube = shape_maker::cube();
	r_cube.bind();
	glUniformMatrix4fv(texture_shader["uModel"], 1, GL_FALSE, &glm::scale(glm::mat4(1.f), glm::vec3(10.0, 10.0, 10.0))[0][0]);
	glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
}

/* callback function called when the windows is resized */
void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);

	glUseProgram(heightmap_shader.program);
	glUniformMatrix4fv(heightmap_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(heightmap_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);

}
/*------- terrain texture ----------*/
texture heightmap, sandTexture;
char heightmap_name[256] = { "./textures/terrain/height_map_blurred.png" };
char sandTexture_name[256] = { "./textures/terrain/sand_texture.jpg" };
/*-------- skybox texture ----------*/
texture skybox, reflection_map;
/*
* id 0: heightmap terrain
* id 1: desert texture terrain
* id 2: skybox
* id 3: reflection map (skybox)
*/
void load_textures() {
	heightmap.load(std::string(heightmap_name), 0);
	sandTexture.load(std::string(sandTexture_name), 1);

	std::string path = "./textures/cube_map/HD/";
	skybox.load_cubemap(path + "posx.png", path + "negx.png",
		path + "posy.png", path + "negy.png",
		path + "posz.png", path + "negz.png", 2);
	//glActiveTexture(GL_TEXTURE3);
	//reflection_map.create_cubemap(2048, 2048, 2);
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	width = 1000;
	height = 800;
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Desolazione - Davide Fantasia's Project", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetWindowSizeCallback(window, window_size_callback);
	//callback della camera
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	printout_opengl_glsl_info();

	/*impostazione delle callback per l'input del mouse*/


	glfwSetWindowSizeCallback(window, window_size_callback);

	/* load the shaders */
	std::string shaders_path = "./shaders/";
	heightmap_shader.create_program((shaders_path + "heightmap.vert").c_str(), (shaders_path + "heightmap.frag").c_str());
	flat_shader.create_program((shaders_path + "flat.vert").c_str(), (shaders_path + "flat.frag").c_str());
	texture_shader.create_program((shaders_path + "texture.vert").c_str(), (shaders_path + "texture.frag").c_str());

	/* Set the uT matrix to Identity */
	glUseProgram(heightmap_shader.program);
	glUniformMatrix4fv(heightmap_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniform1f(heightmap_shader["uHeightScale"], 0.5f); // Imposta il valore desiderato per la scala della dislocazione

	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/*----------- thread per la generazione del terreno --------------*/
	shape s_plane;
	std::thread terraingeneration_thread(TerrainGenerator, &s_plane);

	//glm::mat4 model_matrix = glm::mat4(1.f);
	glm::mat4 model_matrix = glm::scale(glm::mat4(1.f), glm::vec3(3.f, 1.f, 3.f));

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 2.f, 20.f);
	view = camera.GetViewMatrix();


	/* ----------- Passaggio Uniform per la creazione del Terrain ----------------*/
	glUseProgram(heightmap_shader.program);
	glUniformMatrix4fv(heightmap_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(heightmap_shader["uView"], 1, GL_FALSE, &view[0][0]);

	/*-------- Passaggio ID delle texture alla Shader ----------*/
	glUniform1i(heightmap_shader["uColorImage"], 0);
	glUniform1i(heightmap_shader["uSandTexture"], 1);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	/* -------- Passaggio Uniform alla Texture Shader ------------*/
	glUseProgram(texture_shader.program);
	glUniform1i(texture_shader["uSkybox"], 2);
	//glUniform1i(texture_shader["uReflectionMap"], 3);
	glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	/* -------- Passaggio Uniform alla Flat Shader ------------*/
	glUseProgram(flat_shader.program);
	glUniformMatrix4fv(flat_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	check_gl_errors(__LINE__, __FILE__, true);

	/* define the viewport  */
	glViewport(0, 0, width, height);

	load_textures();

	/*---- attesa fine generazione del terreno -----*/
	terraingeneration_thread.join();
	s_plane.to_renderable(r_terrain);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// Per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		/* Render here */
		glClearColor(0.8f, 0.8f, 0.9f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


		// Input
		processInput(window);
		view = camera.GetViewMatrix();

		/*Terrain Rendering*/
		glUseProgram(heightmap_shader.program);
		glUniformMatrix4fv(heightmap_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(heightmap_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(heightmap_shader["uModel"], 1, GL_FALSE, &model_matrix[0][0]);
		r_terrain.bind();
		glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);

		check_gl_errors(__LINE__, __FILE__, true);


		/*Skybox texture*/
		glDepthFunc(GL_LEQUAL);
		glUseProgram(texture_shader.program);
		glUniformMatrix4fv(texture_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(texture_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniform1i(texture_shader["uSkybox"], 2);
		draw_large_cube();
		glDepthFunc(GL_LESS);
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

/* --------------- callback per la gestione della camera ------------ */
void processInput(GLFWwindow* window) {
	//exit button ESC
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	//wasd movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	//switch camera position
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		camera.SwitchPosition(cameraPositions[0]);
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		camera.SwitchPosition(cameraPositions[1]);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			isLeftMouseButtonPressed = true;
			glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR)); //mette il cursore della mano
		}
		else if (action == GLFW_RELEASE) {
			isLeftMouseButtonPressed = false;
			glfwSetCursor(window, nullptr); // Reimposta il cursore predefinito quando il tasto del mouse viene rilasciato
		}
	}
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//ruotiamo la camera solo se viene premuto il tasto sinistro del mouse
	if (isLeftMouseButtonPressed) {
		float xOffset = xpos - lastX;
		float yOffset = lastY - ypos;

		camera.ProcessMouseMovement(xOffset, yOffset, GL_TRUE);
	}
	lastX = xpos;
	lastY = ypos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
	//aggiornamento dello zoom
	proj = glm::perspective(glm::radians(camera.Zoom), width / float(height), 2.f, 20.f);
}