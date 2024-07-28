
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
#include "..\common\frame_buffer_object.h"


//gestione delle texture

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"
#include "camera.h"

#include "terrainGenerator.h"
#include "lights.h"


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
//funzione per l'aggiornamento dell'altezza della camera secondo la heightmap caricata
void updateCameraHeight();

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

/* program shaders used */
shader heightmap_shader, skybox_shader, fsq_shader, depth_shader;
float heightmapScale = 0.25f; //scale of the dune height (heightmap scale value)
float heightmapRep = 4.f; //repetition of the heightmap on the heightmap

/* object that will be rendered in this scene*/
renderable r_terrain;
// cubo che funge da cubemap
renderable r_cube;
// quad da renderizzare a schermo intero a scopo di debug per le texture
renderable r_quad;

void draw_large_cube() {
	r_cube = shape_maker::cube();
	r_cube.bind();
	glUniformMatrix4fv(skybox_shader["uModel"], 1, GL_FALSE, &glm::scale(glm::mat4(1.f), glm::vec3(10.0, 10.0, 10.0))[0][0]);
	glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
}

void draw_full_screen_quad() {
	r_quad.bind();
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void draw_texture(GLint tex_id) {
	GLint at;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glUseProgram(fsq_shader.program);
	glUniform1i(fsq_shader["uTexture"], 0);
	draw_full_screen_quad();
	glUseProgram(0);
	glActiveTexture(at);
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
texture skybox;
/*
* id 0: heightmap terrain
* id 1: desert texture terrain
* id 2: skybox
* id 3: shadowmap, con scopo di shadow mapping
*/
/* ---- framebuffer object for shadowmapping ----*/
frame_buffer_object depthBuffer;

/* projector */
float depth_bias;
float distance_light;

struct projector {
	glm::mat4 view_matrix, proj_matrix;
	texture tex;
	glm::mat4 set_projection(glm::mat4 _view_matrix) {
		view_matrix = _view_matrix;

		//TBD: set the view volume properly so that they are a close fit of the bunding box passed as parameter
		proj_matrix = glm::ortho(-4.f, 4.f, -4.f, 4.f, 0.f, distance_light * 2.f);
		//proj_matrix = glm::perspective(3.14f/2.f,1.0f,0.1f, distance_light*2.f);
		return proj_matrix;
	}
	glm::mat4 light_matrix() {
		return proj_matrix * view_matrix;
	}
	// size of the shadow map in texels
	int sm_size_x, sm_size_y;
};
projector Lproj;


void load_textures() {
	heightmap.load(std::string(heightmap_name), 0, false);
	sandTexture.load(std::string(sandTexture_name), 1, true);
	
	std::string path = "./textures/cube_map/HD/";
	skybox.load_cubemap(path + "posx.png", path + "negx.png",
		path + "posy.png", path + "negy.png",
		path + "posz.png", path + "negz.png", 2);
}

void RenderScene(shader shader);

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
	skybox_shader.create_program((shaders_path + "skybox.vert").c_str(), (shaders_path + "skybox.frag").c_str());
	fsq_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "fsq.frag").c_str());
	depth_shader.create_program((shaders_path + "depthShader.vert").c_str(), (shaders_path + "depthShader.frag").c_str());

	validate_shader_program(heightmap_shader.program);
	validate_shader_program(skybox_shader.program);
	validate_shader_program(fsq_shader.program);
	validate_shader_program(depth_shader.program);

	/* Set the uT matrix to Identity */
	glUseProgram(heightmap_shader.program);
	glUniformMatrix4fv(heightmap_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniform1f(heightmap_shader["uHeightScale"], heightmapScale); // Imposta il valore desiderato per la scala delle dune
	glUniform1f(heightmap_shader["uTextureRep"], heightmapRep); // Imposta il numero di ripetizioni della texture
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/*----------- thread per la generazione del terreno --------------*/
	shape s_plane;
	std::thread terraingeneration_thread(TerrainGenerator, &s_plane);

	//matrice di modello del terreno
	glm::mat4 model_matrix = glm::scale(glm::mat4(1.f), glm::vec3(3.f, 1.f, 3.f));

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 1.f, 10.f);
	view = camera.GetViewMatrix();

	/* ------------ light projection ------------ */

	Lproj.sm_size_x = 2048;
	Lproj.sm_size_y = 2048;
	depth_bias = 0.005f;
	distance_light = 5;

	/* ----------- Passaggio Uniform per la creazione del Terrain ----------------*/
	glUseProgram(heightmap_shader.program);
	glUniformMatrix4fv(heightmap_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(heightmap_shader["uView"], 1, GL_FALSE, &view[0][0]);

	/*-------- Passaggio ID delle texture alla Shader ----------*/
	glUniform1i(heightmap_shader["uColorImage"], 0);


	/* ---------------- creazione e passaggio informazioni sulle luci --------------------*/
	Light sun = sun.directional_init(glm::vec3(0.75f, 3.f, -2.f));
	sun.set_uniform(heightmap_shader.program);


	Light spotLight = spotLight.spotlight_init(glm::vec3(1.f, 2.f, 1.f), glm::vec3(-0.5f, -1.f, -1.f), 25.f, 35.f);
	spotLight.set_uniform(heightmap_shader.program);
	
	glUniform3fv(heightmap_shader["uViewPos"], 1, &camera.Position[0]);
	glUniform1f(heightmap_shader["uBias"], depth_bias);
	/*
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);
	*/

	/* -------- Passaggio Uniform alla Texture Shader ------------*/
	glUseProgram(skybox_shader.program);
	glUniform1i(skybox_shader["uSkybox"], 2);
	glUniformMatrix4fv(skybox_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(skybox_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	check_gl_errors(__LINE__, __FILE__, true);


	load_textures();

	renderable r_cube = shape_maker::cube();

	/*---- attesa fine generazione del terreno -----*/
	terraingeneration_thread.join();
	s_plane.to_renderable(r_terrain);
	r_quad = shape_maker::quad();


	
	//Lproj.view_matrix = glm::lookAt(spotLight.position, spotLight.direction, glm::vec3(0.f, 0.f, 1.f));
	Lproj.view_matrix = glm::lookAt(sun.position, sun.direction, glm::vec3(0.f, 0.f, 1.f));
	
	depthBuffer.create(Lproj.sm_size_x, Lproj.sm_size_y, true);

	/* ------------------ RENDER LOOP ---------------------------*/
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.8f, 0.8f, 0.9f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//deph mapping
		
		glUseProgram(depth_shader.program);

		Lproj.set_projection(Lproj.view_matrix);
		glUniformMatrix4fv(depth_shader["uLightSpaceMatrix"], 1, GL_FALSE, &(Lproj.light_matrix())[0][0]);

		glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer.id_fbo);
		glViewport(0, 0, Lproj.sm_size_x, Lproj.sm_size_y);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glCullFace(GL_FRONT); //per sistemare il peter-panning
			// -------- render scene
			glUniformMatrix4fv(depth_shader["uModel"], 1, GL_FALSE, &model_matrix[0][0]);
			r_terrain.bind();
			glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);


			glUniformMatrix4fv(depth_shader["uModel"], 1, GL_FALSE, &(glm::scale(glm::mat4(1.f), glm::vec3(0.5f, 0.5f, 0.5f)))[0][0]);
			r_cube.bind();
			glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
			// ------
			glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		// ripristino del frame buffer attivo

		/*-----------------------------------------------------------------------------*/

		glViewport(0, 0, width, height);
		
		// Input
		processInput(window);
		view = camera.GetViewMatrix();

		//Terrain Rendering
		glUseProgram(heightmap_shader.program);
		glUniformMatrix4fv(heightmap_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(heightmap_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(heightmap_shader["uModel"], 1, GL_FALSE, &model_matrix[0][0]);
		glUniform3fv(heightmap_shader["uViewPos"], 1, &camera.Position[0]);
		//shadowmapping
		glUniformMatrix4fv(heightmap_shader["uLightSpaceMatrix"], 1, GL_FALSE, &(Lproj.light_matrix())[0][0]);

		GLint at;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthBuffer.id_depth);
		glUniform1i(glGetUniformLocation(heightmap_shader.program, "uShadowMap"), 3);
		glActiveTexture(at);

		//passaggio info per il materiale
		glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.diffuse_map"), 1);
		glUniform1f(glGetUniformLocation(heightmap_shader.program, "material.shininess"), 0.5f);
		glUniform3fv(glGetUniformLocation(heightmap_shader.program, "material.specular"), 1, glm::value_ptr(glm::vec3(0.94f, 0.80f, 0.49f)));

		r_terrain.bind();
		glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);

		//disegno del cubo
		glUniformMatrix4fv(heightmap_shader["uModel"], 1, GL_FALSE, &(glm::scale(glm::mat4(1.f), glm::vec3(0.5f, 0.5f, 0.5f)))[0][0]);
		glUniform1i(glGetUniformLocation(heightmap_shader.program, "material.diffuse_map"), 1);
		glUniform1f(glGetUniformLocation(heightmap_shader.program, "material.shininess"), 32.0f);
		glUniform3fv(glGetUniformLocation(heightmap_shader.program, "material.specular"), 1, glm::value_ptr(glm::vec3(0.94f, 0.80f, 0.49f)));

		r_cube.bind();
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);

		check_gl_errors(__LINE__, __FILE__, true);
		
		//skybox texture
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skybox_shader.program);
		glUniformMatrix4fv(skybox_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(skybox_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniform1i(skybox_shader["uSkybox"], 2);
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

void RenderScene(shader shader) {

}


/* --------------- callback per la gestione della camera ------------ */
void processInput(GLFWwindow* window) {
	//exit button ESC
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	//wasd movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
		updateCameraHeight();
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		updateCameraHeight();
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
		updateCameraHeight();
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
		updateCameraHeight();
	}
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

/* funzione che aggiorna la posizione (valore y) della camera seguendo l'altezza delle dune */
void updateCameraHeight() {
	float heightValue = heightmap.heightFunction(camera.Position.x, camera.Position.z, heightmapRep);
	camera.Position.y += (heightValue * heightmapScale);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
	//aggiornamento dello zoom
	proj = glm::perspective(glm::radians(camera.Zoom), width / float(height), 1.f, 10.f);
}
