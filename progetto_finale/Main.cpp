
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
#include "material.h"
#include "terrainGenerator.h"
#include "lights.h"
#include "obj_loader.h"

#include "..\common\gltf_loader.h"


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
shader lighting_shader, skybox_shader, fsq_shader, uv_shader, depth_shader;
float heightmapScale = 0.25f; //scale of the dune height (heightmap scale value)
float heightmapRep = 4.f; //repetition of the heightmap on the heightmap

/* object that will be rendered in this scene*/
renderable r_terrain;
// cubo che funge da cubemap
renderable r_cube;
// quad da renderizzare a schermo intero a scopo di debug per le texture
renderable r_quad;

renderable lamp_r, bust_r;
glm::mat4 lamp_model_mat, bust_model_mat;

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

	glUseProgram(lighting_shader.program);
	glUniformMatrix4fv(lighting_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(lighting_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);

}
/*---------------- texture ----------------*/
//texture per il terreno
texture heightmap, sandTexture;
char heightmap_name[256] = { "./textures/terrain/height_map_blurred.png" };
char sandTexture_name[256] = { "./textures/terrain/sand_texture.jpg" };
//texture del busto del david
texture diffuse_david, roughness_david, metallic_david, normal_david, ao_david, uv_david;
texture diffuse_lamp, roughness_lamp, metallic_lamp, normal_lamp, ao_lamp;
//texture per lo skybox
texture daySkybox, nightSkybox;

/*
* id 0: heightmap terrain
* id 1: desert texture terrain
* id 2: daySkybox
* id 3: nightSkybox
* 
* id 4: david_diffuse
* id 5: david_roughness
* id 6: daivd_metallic
* id 7: david_normal
* id 8: david_ao
* id 9: david_uv
* 
* id 10: lamp diffuse
* id 11: lamp roughness
* id 12: lamp metallic
* id 13: lamp normal
* id 14: lamp ao

* ------
* id 20: shadowmap del sole, con scopo di shadow mapping
* id 21: shadowmap dello spot light, con scopo di shadow mapping
*/
void load_textures() {
	heightmap.load(std::string(heightmap_name), 0, false);
	sandTexture.load(std::string(sandTexture_name), 1, true);
	
	std::string path = "./textures/cube_map/day/";
	daySkybox.load_cubemap(path + "posx.bmp", path + "negx.bmp",
		path + "posy.bmp", path + "negy.bmp",
		path + "posz.bmp", path + "negz.bmp", 2);

	path = "./textures/cube_map/night/";
	nightSkybox.load_cubemap(path + "posx.png", path + "negx.png",
		path + "posy.png", path + "negy.png",
		path + "posz.png", path + "negz.png", 3);
	//texture del busto
	path = "./models/bust/textures/";
	diffuse_david.load(path + "diffuse.png",4,true);
	roughness_david.load(path + "roughness.png", 5, false);
	metallic_david.load(path + "metallic.png", 6, false);
	normal_david.load(path + "normal_tangent.png", 7, false);
	ao_david.load(path + "ao.png", 8, false);

	//texture del lampione
	path = "./models/lamp/textures/";
	

	diffuse_lamp.load(path + "diffuse.png", 10, true);
	roughness_lamp.load(path + "roughness.png", 11, false);
	metallic_lamp.load(path + "metallic.png", 12, false);
	normal_lamp.load(path + "normal_tangent.png", 13, false);
	ao_lamp.load(path + "ambient_occlusion.png", 14, false);
}

/* ---- framebuffer object for shadowmapping ----*/
frame_buffer_object sunDepthBuffer, lampDepthBuffer;

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
projector sunProjector, lampProjector;

void RenderScene(shader shader);
void render_gltf(shader shader, std::vector<renderable> obj, box3 bbox);

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

	glEnable(GL_MULTISAMPLE);

	printout_opengl_glsl_info();

	/*impostazione delle callback per l'input del mouse*/


	glfwSetWindowSizeCallback(window, window_size_callback);

	/* load the shaders */
	std::string shaders_path = "./shaders/";
	lighting_shader.create_program((shaders_path + "pbr.vert").c_str(), (shaders_path + "pbr.frag").c_str());
	skybox_shader.create_program((shaders_path + "skybox.vert").c_str(), (shaders_path + "skybox.frag").c_str());
	fsq_shader.create_program((shaders_path + "fsq.vert").c_str(), (shaders_path + "fsq.frag").c_str());
	depth_shader.create_program((shaders_path + "depthShader.vert").c_str(), (shaders_path + "depthShader.frag").c_str());

	validate_shader_program(lighting_shader.program);
	validate_shader_program(skybox_shader.program);
	validate_shader_program(fsq_shader.program);
	validate_shader_program(depth_shader.program);

	/* Set the uT matrix to Identity */
	glUseProgram(lighting_shader.program);
	glUniformMatrix4fv(lighting_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUniform1f(lighting_shader["uHeightScale"], heightmapScale); // Imposta il valore desiderato per la scala delle dune
	glUniform1f(lighting_shader["uTextureRep"], heightmapRep); // Imposta il numero di ripetizioni della texture
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/*----------- thread per la generazione del terreno --------------*/
	shape s_plane;
	std::thread terraingeneration_thread(TerrainGenerator, &s_plane);


	// --------------------------------------------------------------------------------------------------------------
	// load an obj object
	//loadOBJ("./models/bust/bust.obj", bust_r);

	loadOBJ("./models/lamp/street_light.obj", lamp_r);
	check_gl_errors(__LINE__, __FILE__, true);

	gltf_loader gltfLoader;
	box3 bbox;
	std::vector <renderable> obj;

	gltfLoader.load_to_renderable("./models/bust/bust_v3.glb", obj , bbox);

	//matrice di modello del lampione
	lamp_model_mat = glm::scale(glm::mat4(1.f), glm::vec3(0.25f, 0.25f, 0.25f));
	//matrice di modello del busto
	bust_model_mat = glm::scale(glm::mat4(1.f), glm::vec3(0.005f, 0.005f, 0.005f));
	//matrice di modello del terreno
	glm::mat4 terrain_model_mat = glm::scale(glm::mat4(1.f), glm::vec3(3.f, 1.f, 3.f));

	// --------------------------------------------------------------------------------------------------------------

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 1.f, 10.f);
	view = camera.GetViewMatrix();

	/* ------------ light projection ------------ */

	sunProjector.sm_size_x = 2048;
	sunProjector.sm_size_y = 2048;

	lampProjector.sm_size_x = 2048;
	lampProjector.sm_size_y = 2048;

	depth_bias = 0.01f;
	distance_light = 5;

	/* ----------- Passaggio Uniform per la creazione del Terrain ----------------*/
	glUseProgram(lighting_shader.program);
	glUniformMatrix4fv(lighting_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(lighting_shader["uView"], 1, GL_FALSE, &view[0][0]);

	/*-------- Passaggio ID delle texture alla Shader ----------*/
	glUniform1i(lighting_shader["uColorImage"], 0);


	/* ---------------- creazione e passaggio informazioni sulle luci --------------------*/
	Light sun = sun.directional_init(glm::vec3(0.f, -1.f, 0.f));
	sun.set_uniform(lighting_shader.program);


	Light spotLight = spotLight.spotlight_init(glm::vec3(1.f, 2.f, 1.f), glm::vec3(-0.5f, -1.f, -1.f), 25.f, 35.f);
	spotLight.set_uniform(lighting_shader.program);
	
	glUniform3fv(lighting_shader["uViewPos"], 1, &camera.Position[0]);
	glUniform1f(lighting_shader["uBias"], depth_bias);
	
	check_gl_errors(__LINE__, __FILE__, true);
	/*
	glUseProgram(0);
	
	*/

	/* -------- Passaggio Uniform alla Texture Shader ------------*/
	glUseProgram(skybox_shader.program);
	glUniform1i(skybox_shader["uDaySkybox"], 2);
	glUniform1i(skybox_shader["uNightSkybox"], 3);
	glUniformMatrix4fv(skybox_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(skybox_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);
	check_gl_errors(__LINE__, __FILE__, true);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	check_gl_errors(__LINE__, __FILE__, true);


	load_textures();

	sunProjector.view_matrix = glm::lookAt(sun.position, sun.direction, glm::vec3(0.f, 0.f, 1.f));
	lampProjector.view_matrix = glm::lookAt(spotLight.position, spotLight.direction, glm::vec3(0.f, 0.f, 1.f));

	sunDepthBuffer.create(sunProjector.sm_size_x, sunProjector.sm_size_y, true);
	lampDepthBuffer.create(lampProjector.sm_size_x, lampProjector.sm_size_y, true);

	renderable r_cube = shape_maker::cube();

	SandTerrainMaterial sand_material; //materiale del terreno
	BustMaterial bust_material;
	LampMaterial lamp_material;
	//cube_material.init(diffuse_cube.id,roughness_cube.id, metallic_cube.id, normal_cube.id);
	
	bust_material.init(diffuse_david.tu,roughness_david.tu,metallic_david.tu,normal_david.tu,ao_david.tu);
	lamp_material.init(diffuse_lamp.tu,roughness_lamp.tu,metallic_lamp.tu,normal_lamp.tu,ao_lamp.tu);

	/*---- attesa fine generazione del terreno e caricamento modello -----*/
	terraingeneration_thread.join();

	s_plane.to_renderable(r_terrain);
	r_quad = shape_maker::quad();


	float dayNight_rotation_angle = 0.f;
	float dayNight_rotation_speed = 10.0f; // velocità di rotazione del sole (gradi per secondo)
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
		
		// Calcolare l'angolo di rotazione del sole
		dayNight_rotation_angle += (dayNight_rotation_speed * deltaTime);
		
		sun.rotate_direction(dayNight_rotation_angle);
		//aggiornamento della view matrix del proiettore
		sunProjector.view_matrix = glm::lookAt(sun.direction, glm::vec3(0.0f), glm::vec3(0.f, 0.f, 1.f));
		//deph mapping

		glUseProgram(depth_shader.program);

		sunProjector.set_projection(sunProjector.view_matrix);
		//ruotiamo la matrice di Light Space di dayNight_rotation_angle per simulare il ciclo giorno notte
		glUniformMatrix4fv(depth_shader["uLightSpaceMatrix"], 1, GL_FALSE, &(sunProjector.light_matrix())[0][0]);

		glBindFramebuffer(GL_FRAMEBUFFER, sunDepthBuffer.id_fbo);
		glViewport(0, 0, sunProjector.sm_size_x, sunProjector.sm_size_y);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glCullFace(GL_FRONT); //per sistemare il peter-panning
			// -------- render scene
			glUniformMatrix4fv(depth_shader["uModel"], 1, GL_FALSE, &terrain_model_mat[0][0]);
			r_terrain.bind();
			glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
			check_gl_errors(__LINE__, __FILE__, true);
			//RenderScene(depth_shader);
			render_gltf(depth_shader, obj, bbox);
			// ------
			glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		/*
		//rendering depth map del faretto
		glUseProgram(depth_shader.program);

		lampProjector.set_projection(lampProjector.view_matrix);
		glUniformMatrix4fv(depth_shader["uLightSpaceMatrix"], 1, GL_FALSE, &(lampProjector.light_matrix())[0][0]);

		glBindFramebuffer(GL_FRAMEBUFFER, lampDepthBuffer.id_fbo);
		glViewport(0, 0, lampProjector.sm_size_x, lampProjector.sm_size_y);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glCullFace(GL_FRONT); //per sistemare il peter-panning
			// -------- render scene
			glUniformMatrix4fv(depth_shader["uModel"], 1, GL_FALSE, &terrain_model_mat[0][0]);
			r_terrain.bind();
			glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
			render_gltf(depth_shader, obj, bbox);
		// ------
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
		// ripristino del frame buffer attivo
		*/

		/*-----------------------------------------------------------------------------*/
		
		glViewport(0, 0, width, height);

		// Input
		processInput(window);
		view = camera.GetViewMatrix();

		//Terrain Rendering
		glUseProgram(lighting_shader.program);
		sun.set_uniform(lighting_shader.program); //aggiornamento dei dati del sole che ruota

		glUniformMatrix4fv(lighting_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(lighting_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(lighting_shader["uModel"], 1, GL_FALSE, &terrain_model_mat[0][0]);
		glUniform3fv(lighting_shader["uViewPos"], 1, &camera.Position[0]);

		//shadowmapping del sole
		glUniformMatrix4fv(lighting_shader["uSunLightSpaceMatrix"], 1, GL_FALSE, &(sunProjector.light_matrix())[0][0]);
		//passaggio della shadowmap del sole(slot 20)
		GLint at;
		glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
		glActiveTexture(GL_TEXTURE20);
		glBindTexture(GL_TEXTURE_2D, sunDepthBuffer.id_depth);
		glUniform1i(glGetUniformLocation(lighting_shader.program, "uSunShadowMap"), 20);
		glActiveTexture(at);
		/*
		//shadowmapping del faretto
		glUniformMatrix4fv(lighting_shader["uLampLightSpaceMatrix"], 1, GL_FALSE, &(lampProjector.light_matrix())[0][0]);
		//passaggio della shadowmap del faretto(slot 21)
		glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
		glActiveTexture(GL_TEXTURE21);
		glBindTexture(GL_TEXTURE_2D, lampDepthBuffer.id_depth);
		glUniform1i(glGetUniformLocation(lighting_shader.program, "uLampShadowMap"), 21);
		glActiveTexture(at);
		check_gl_errors(__LINE__, __FILE__, true);
		*/
		//passaggio uniform del materiale
		sand_material.set_shader_uniforms(lighting_shader.program);
		check_gl_errors(__LINE__, __FILE__, true);
		r_terrain.bind();
		check_gl_errors(__LINE__, __FILE__, true);
		glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
		check_gl_errors(__LINE__, __FILE__, true);
		
		//disegno del busto
		bust_material.set_shader_uniforms(lighting_shader.program);
		//lamp_material.set_shader_uniforms(lighting_shader.program);
		render_gltf(lighting_shader, obj, bbox);

		//skybox texture
		glDepthFunc(GL_LEQUAL);
		glUseProgram(skybox_shader.program);
		glUniformMatrix4fv(skybox_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(skybox_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniform1f(skybox_shader["uDayNightAngle"], dayNight_rotation_angle);
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

void render_gltf(shader shader, std::vector<renderable> obj, box3 bbox) {
	for (unsigned int i = 0; i < obj.size(); ++i) {
		obj[i].bind();
		// each object had its own transformation that was read in the gltf file
		// glm::translate(bust_model_mat, glm::vec3(0.f, 3.f, 0.f)) * glm::translate(glm::rotate(obj[i].transform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)), glm::vec3(-bbox.center()))
		// glm::translate(glm::rotate(bust_model_mat * obj[i].transform, glm::radians(20.f), glm::vec3(0.f, 0.f, 1.f)), glm::vec3(0.f, 5.f, 0.f))
		glUniformMatrix4fv(shader["uModel"], 1, GL_FALSE, &(glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.35f, 0.f)) * glm::translate(glm::rotate(bust_model_mat*obj[i].transform, glm::radians(20.f), glm::vec3(0.f, 0.f, 1.f)), glm::vec3(-bbox.center())))[0][0]);
		glDrawElements(obj[i]().mode, obj[i]().count, obj[i]().itype, 0);
	}
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
