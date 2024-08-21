
#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>

#include <thread>
#include <cmath> // Per fmod

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

/*---------------- texture ----------------*/
//texture per il terreno
texture heightmap, sandTexture;
//char heightmap_name[256] = { "./textures/terrain/height_map.png" };
char heightmap_name[256] = { "./textures/terrain/test_blur.png" };
char sandTexture_name[256] = { "./textures/terrain/sand_texture_tileable.png" };

//texture del busto del david
texture diffuse_david, roughness_david, metallic_david, normal_david, ao_david, uv_david;
texture diffuse_lamp, roughness_lamp, metallic_lamp, normal_lamp, ao_lamp, emissive_lamp;
//texture per lo skybox
texture daySkybox, nightSkybox;
//indice delle texture
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
*
* id 8: lamp diffuse
* id 9: lamp roughness
* id 10: lamp metallic
* id 11: lamp normal
* id 12: lamp emissive map
* id 13: lamp ao

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
	diffuse_david.load(path + "diffuse.png", 4, true);
	roughness_david.load(path + "roughness.png", 5, false);
	metallic_david.load(path + "metallic.png", 6, false);
	normal_david.load(path + "normal_tangent.png", 7, false);

	//texture del lampione
	//path = "./models/lamp/textures/";
	path = "./models/lamp_v2/textures/";
	diffuse_lamp.load(path + "diffuse.png", 8, true);
	roughness_lamp.load(path + "roughness.png", 9, true);
	metallic_lamp.load(path + "metallic.png", 10, true);
	normal_lamp.load(path + "normal_tangent.png", 11, false);
	emissive_lamp.load(path + "emit.png", 12, false);
	ao_lamp.load(path + "ao.png", 13, false);

	std::cout << "texture caricate correttamente" << std::endl;
}


// Camera

Camera camera;
std::vector<glm::vec3> cameraPositions;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;
bool isLeftMouseButtonPressed = false;
// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
//funzione per l'aggiornamento dell'altezza della camera secondo la heightmap caricata
float updateCameraHeight(float x, float z);

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

/* program shaders used */
shader lighting_shader, skybox_shader, fsq_shader, uv_shader, depth_shader;
float heightmapScale = 1.65f; //scale of the dune height (heightmap scale value)
float heightmapRep = 0.75f; //repetition of the heightmap on the heightmap
float terrain_lato = 10.f * heightmapScale; //dimensione del lato del terreno (10 = max)

/* object that will be rendered in this scene*/
renderable r_terrain;
// cubo che funge da cubemap
renderable r_cube;
// quad da renderizzare a schermo intero a scopo di debug per le texture
renderable r_quad;

box3 bust_box, lamp_box;
std::vector <renderable> bust_r, lamp_r;

glm::mat4 lamp_model_mat, bust_model_mat;

SandTerrainMaterial sand_material; //materiale del terreno
BustMaterial bust_material;
LampMaterial lamp_material;

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
//for debug purpose
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
	proj = glm::perspective(glm::radians(camera.Zoom), width / float(height), 0.25f, 10.f);

	glUseProgram(lighting_shader.program);
	glUniformMatrix4fv(lighting_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(lighting_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUseProgram(0);

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
		proj_matrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, distance_light * 2.f);
		return proj_matrix;
	}
	glm::mat4 light_matrix() {
		return proj_matrix * view_matrix;
	}
	// size of the shadow map in texels
	int sm_size_x, sm_size_y;
};
projector sunProjector, lampProjector;
Light spotLight, lamp_bulb, sun;


//function to render a single gltf/glb model, return the global matrix of the model
glm::mat4 render_gltf(shader shader, bool isLightShader, std::vector<renderable> obj, box3 bbox, glm::mat4 global_position);
void renderScene(shader shader, bool isLightShader); //function to render the whole scene

matrix_stack stack;

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
	glfwWindowHint(GLFW_SAMPLES, 4);  // 4x MSAA

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
	glUniform1f(lighting_shader["uHeightScale"], heightmapScale); // Imposta il valore desiderato per la scala delle dune
	glUniform1f(lighting_shader["uTextureRep"], heightmapRep); // Imposta il numero di ripetizioni della texture
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/*----------- thread per la generazione del terreno --------------*/
	shape s_plane;
	std::thread terraingeneration_thread(TerrainGenerator, &s_plane);

	check_gl_errors(__LINE__, __FILE__, true);

	// --------------------------------------------------------------------------------------------------------------
	// CARICAMENTO MODELLI DA FILE
	gltf_loader gltfLoader_1, gltfLoader_2;

	gltfLoader_1.load_to_renderable("./models/bust/bust.glb", bust_r, bust_box);
	gltfLoader_2.load_to_renderable("./models/lamp_v2/street_lamp.glb", lamp_r, lamp_box);

	bust_model_mat = glm::translate(glm::mat4(1.f), glm::vec3(0.5f, 0.35f, 0.f));
	lamp_model_mat = glm::translate(glm::mat4(1.f), glm::vec3(0.25f, 0.25f, -0.25f)) * glm::rotate(glm::mat4(1.f), glm::radians(-20.f), glm::vec3(1.f, 0.f, 0.f)) * glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(1.25f, 1.25f, 1.25f));

	for (int i = 0; i < bust_r.size(); i++) {
		bust_r[i].transform = glm::rotate(bust_r[i].transform, glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));
		bust_r[i].transform = glm::rotate(bust_r[i].transform, glm::radians(30.f), glm::vec3(0.f, 0.f, 1.f));
	}
	// --------------------------------------------------------------------------------------------------------------

	/* Transformation to setup the point of view on the scene */

	load_textures();
	updateCameraHeight(2.045f, 1.266f);
	cameraPositions.push_back(glm::vec3(2.045f, camera.Position.y, 1.266f));
	updateCameraHeight(-1.062f, 1.518f);
	cameraPositions.push_back(glm::vec3(-1.062f, camera.Position.y, 1.518f));

	proj = glm::perspective(glm::radians(camera.Zoom), width / float(height), 0.25f, 10.f);
	view = camera.GetViewMatrix();

	/* ------------ light projection ------------ */

	sunProjector.sm_size_x = 2048;
	sunProjector.sm_size_y = 2048;

	lampProjector.sm_size_x = 1924;
	lampProjector.sm_size_y = 1924;

	depth_bias = 0.005f;
	distance_light = 10;

	/* ----------- Passaggio Uniform per la creazione del Terrain ----------------*/
	glUseProgram(lighting_shader.program);
	glUniformMatrix4fv(lighting_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(lighting_shader["uView"], 1, GL_FALSE, &view[0][0]);

	/*-------- Passaggio ID delle texture alla Shader ----------*/
	glUniform1i(lighting_shader["uColorImage"], 0);

	/* ---------------- creazione e passaggio informazioni sulle luci --------------------*/
	sun = sun.directional_init(glm::vec3(0.f, -1.f, 0.f));
	sun.set_uniform(lighting_shader.program);

	glm::vec4 spotLight_position = glm::vec4(0.294f, 1.5f, -0.424f, 1.f);
	glm::vec4 spotLight_direction = glm::normalize(-glm::vec4(-0.13f, 0.775f, -0.618f, 0.f));

	spotLight = spotLight.spotlight_init(glm::vec3(spotLight_position.x, spotLight_position.y, spotLight_position.z), glm::vec3(spotLight_direction.x, spotLight_direction.y, spotLight_direction.z), 25.f, 35.f);
	spotLight.set_uniform(lighting_shader.program);

	lamp_bulb = lamp_bulb.pointLight_init(glm::vec3(spotLight_position));
	lamp_bulb.set_uniform(lighting_shader.program);

	glUniform3fv(lighting_shader["uViewPos"], 1, &camera.Position[0]);
	glUniform1f(lighting_shader["uBias"], depth_bias);

	check_gl_errors(__LINE__, __FILE__, true);

	/* -------- Passaggio Uniform alla skybox Shader ------------*/
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

	sunProjector.view_matrix = glm::lookAt(sun.position, sun.direction, glm::vec3(0.f, 0.f, 1.f));
	lampProjector.view_matrix = glm::lookAt(spotLight.position, spotLight.direction, glm::vec3(0.f, 0.f, 1.f));

	sunDepthBuffer.create(sunProjector.sm_size_x, sunProjector.sm_size_y, true);
	lampDepthBuffer.create(lampProjector.sm_size_x, lampProjector.sm_size_y, true);

	renderable r_cube = shape_maker::cube();

	//creazione del materiale dei modelli sulla base delle texture caricate
	bust_material.init(diffuse_david.tu, roughness_david.tu, metallic_david.tu, normal_david.tu);
	lamp_material.init(diffuse_lamp.tu, roughness_lamp.tu, metallic_lamp.tu, normal_lamp.tu, emissive_lamp.tu, ao_lamp.tu);

	/*---- attesa fine generazione del terreno e caricamento modello -----*/
	terraingeneration_thread.join();

	s_plane.to_renderable(r_terrain);
	r_quad = shape_maker::quad();
	//matrice di modello del terreno
	r_terrain.transform *= glm::scale(glm::mat4(1.f), glm::vec3(terrain_lato, heightmapScale, terrain_lato));

	float dayNight_rotation_angle = 0.f;
	float dayNight_rotation_speed = 10.0f; // velocit? di rotazione del sole (gradi per secondo)


	/* ------------------ RENDER LOOP ---------------------------*/
	while (!glfwWindowShouldClose(window)) {
		
		/* Render here */
		glClearColor(0.8f, 0.8f, 0.9f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		stack.load_identity();

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
		// -------- 
		//render scene
		renderScene(depth_shader, false);
		// ------
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);

		//condizione per determinare se renderizzare la luce e le ombre del lampione
		float normalized_angle = 0.5 * (1.0 + glm::cos(glm::radians(fmod(dayNight_rotation_angle, 360.0)) - 3.14159 / 2.0));
		//maggiore di alba e minore di tramonto
		bool isNight = (normalized_angle > 0.f) && (normalized_angle < 0.57f);
		if (isNight) {
			//rendering depth map del faretto
			glUseProgram(depth_shader.program);

			lampProjector.view_matrix = glm::lookAt(spotLight.position, spotLight.direction, glm::vec3(0.f, 0.f, 1.f));

			lampProjector.set_projection(lampProjector.view_matrix);
			glUniformMatrix4fv(depth_shader["uLightSpaceMatrix"], 1, GL_FALSE, &(lampProjector.light_matrix())[0][0]);

			glBindFramebuffer(GL_FRAMEBUFFER, lampDepthBuffer.id_fbo);
			glViewport(0, 0, lampProjector.sm_size_x, lampProjector.sm_size_y);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glCullFace(GL_FRONT); //per sistemare il peter-panning
			// --------

			//render scene
			renderScene(depth_shader, false);

			// ------
			glCullFace(GL_BACK);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glUseProgram(0);
			// ripristino del frame buffer attivo
		}
		/*-----------------------------------------------------------------------------*/

		glViewport(0, 0, width, height);

		// Input
		processInput(window);
		view = camera.GetViewMatrix();
		//Terrain Rendering
		glUseProgram(lighting_shader.program);
		sun.set_uniform(lighting_shader.program); //aggiornamento dei dati del sole che ruota

		if (isNight) glUniform1i(lighting_shader["isNight"], 1);
		else glUniform1i(lighting_shader["isNight"], 0);

		glUniformMatrix4fv(lighting_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(lighting_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		//glUniformMatrix4fv(lighting_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
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

		//shadowmapping del faretto
		glUniformMatrix4fv(lighting_shader["uLampLightSpaceMatrix"], 1, GL_FALSE, &(lampProjector.light_matrix())[0][0]);
		//passaggio della shadowmap del faretto(slot 21)
		glGetIntegerv(GL_ACTIVE_TEXTURE, &at);
		glActiveTexture(GL_TEXTURE21);
		glBindTexture(GL_TEXTURE_2D, lampDepthBuffer.id_depth);
		glUniform1i(glGetUniformLocation(lighting_shader.program, "uLampShadowMap"), 21);
		glActiveTexture(at);
		check_gl_errors(__LINE__, __FILE__, true);

		renderScene(lighting_shader, true);

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

glm::mat4 render_gltf(shader shader, bool isLightShader, std::vector<renderable> obj, box3 bbox, glm::mat4 global_position) {
	// Scala il modello per adattarsi alla scena
	float scale = 1.f / bbox.diagonal();
	glm::mat4 scale_box = glm::scale(glm::mat4(1.f), glm::vec3(scale));
	glm::mat4 center = glm::translate(glm::mat4(1.f), glm::vec3(-bbox.center()));
	glm::mat4 global_matrix;
	for (unsigned int i = 0; i < obj.size(); ++i) {
		//float height_value = heightmap.heightFunction(bbox.min.x, bbox.min.z, heightmapRep);
		float height_value = heightmap.heightFunction(obj[i].transform[3][0], obj[i].transform[3][2], heightmapRep);

		//float new_y = r_terrain.transform[3][1] + (height_value * heightmapScale);
		float new_y = 0.07f + (height_value * heightmapScale);
		glm::mat4 terrain_level = glm::translate(glm::mat4(1.f), glm::vec3(0.f, new_y, 0.f));

		obj[i].bind();
		global_matrix = global_position * terrain_level * scale_box * center * obj[i].transform;
		glUniformMatrix4fv(shader["uModel"], 1, GL_FALSE, &(global_matrix)[0][0]);
		glDrawElements(obj[i]().mode, obj[i]().count, obj[i]().itype, 0);
	}
	return global_matrix;
}

void renderScene(shader shader, bool isLightShader) {
	//rendering del terreno

	if (isLightShader) sand_material.set_shader_uniforms(shader.program);
	r_terrain.bind();
	stack.push();
	stack.mult(r_terrain.transform);
	glUniformMatrix4fv(shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype, 0);
	stack.pop();
	check_gl_errors(__LINE__, __FILE__, true);

	//rendering del busto
	if (isLightShader) bust_material.set_shader_uniforms(shader.program);
	render_gltf(shader, isLightShader, bust_r, bust_box, bust_model_mat);
	check_gl_errors(__LINE__, __FILE__, true);

	//rendering della street light
	if (isLightShader) lamp_material.set_shader_uniforms(shader.program);
	glm::mat4 lamp_global_matrix = render_gltf(shader, isLightShader, lamp_r, lamp_box, lamp_model_mat);
	check_gl_errors(__LINE__, __FILE__, true);
}

/* --------------- callback per la gestione della camera ------------ */
void processInput(GLFWwindow* window) {
	//exit button ESC
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	//wasd movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
		updateCameraHeight(camera.Position.x, camera.Position.z);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		updateCameraHeight(camera.Position.x, camera.Position.z);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
		updateCameraHeight(camera.Position.x, camera.Position.z);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
		updateCameraHeight(camera.Position.x, camera.Position.z);
	}
	//switch camera position
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		camera.SwitchPosition(cameraPositions[0]);
		camera.Yaw = -137.025f;
		camera.Pitch = 2.1;
		camera.updateCameraVectors();
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		camera.SwitchPosition(cameraPositions[1]);
		camera.Yaw = -57.75f;
		camera.Pitch = -1.425;
		camera.updateCameraVectors();
	}
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
float updateCameraHeight(float x, float z) {
	float heightValue = heightmap.heightFunction(x, z, heightmapRep);
	float new_y = 0.5f + (heightValue * heightmapScale);
	camera.Position.y = new_y;
	return new_y;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
	//aggiornamento dello zoom
	proj = glm::perspective(glm::radians(camera.Zoom), width / float(height), 0.25f, 10.f);
}
