#pragma once

#include <GL/glew.h>
#include <string>
/*
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
*/
struct texture {
	texture() { }
	~texture() {  }

	int x_size, y_size;
	int tu;
	int n_components;
	unsigned char* pixelData;
	GLuint id;
	GLuint load(std::string name, GLuint _tu, bool isGammaCorrected) {
		unsigned char* data;
		data = stbi_load(name.c_str(), &x_size, &y_size, &n_components, 0);
		stbi__vertical_flip(data, x_size, y_size, n_components);
		tu = _tu;
		glActiveTexture(GL_TEXTURE0 + tu);
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		int channels;
		int internalFormat;
		std::cout << name << std::endl;
		switch (n_components) {
			case 1: internalFormat = channels = GL_RED; break;
			case 3: channels = GL_RGB; internalFormat = isGammaCorrected ? GL_SRGB : GL_RGB; break;
			case 4: channels = GL_RGBA; internalFormat = isGammaCorrected ? GL_SRGB_ALPHA : GL_RGBA; break;
			default: std::cout << n_components << std::endl; assert(0);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, data);
		//stbi_image_free(data);
		pixelData = data;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		return id;
	}

	GLuint create(int x_size, int y_size, GLuint channels) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		return id;
	}

	//si consiglia una dimensione di 1024x1024
	GLuint createDepthMap(int x_size, int y_size) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, x_size, y_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		return id;
	}

	GLuint load_cubemap(std::string posx, std::string negx,
		std::string posy, std::string negy,
		std::string posz, std::string negz,
		GLuint tu) {
			unsigned char* data[6];
			data[0] = stbi_load(posx.c_str(), &x_size, &y_size, &n_components, 0);
			data[1] = stbi_load(negx.c_str(), &x_size, &y_size, &n_components, 0);
			data[2] = stbi_load(posy.c_str(), &x_size, &y_size, &n_components, 0);
			data[3] = stbi_load(negy.c_str(), &x_size, &y_size, &n_components, 0);
			data[4] = stbi_load(posz.c_str(), &x_size, &y_size, &n_components, 0);
			data[5] = stbi_load(negz.c_str(), &x_size, &y_size, &n_components, 0);

			glActiveTexture(GL_TEXTURE0 + tu);
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_CUBE_MAP, id);
			int channels;
			switch (n_components) {
			case 1: channels = GL_RED; break;
			case 3: channels = GL_RGB; break;
			case 4: channels = GL_RGBA; break;
			default: assert(0);
			}
			for (unsigned int i = 0; i < 6; ++i)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, data[i]);

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			for (unsigned int i = 0; i < 6; ++i)
				stbi_image_free(data[i]);
			return id;
	}

	GLuint create_cubemap(int x_size, int y_size, int n_components) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		int channels;
		switch (n_components) {
		case 1: channels = GL_RED; break;
		case 3: channels = GL_RGB; break;
		case 4: channels = GL_RGBA; break;
		default: assert(0);
		}
		for (unsigned int i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, x_size, y_size, 0, channels, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		return id;
	}
	// returna l'altezza normalizzata della heightmap 
	float heightFunction(float x, float z, float textureRep) {
		// converione da (x,z) a (u,v)
		int mapped_x = static_cast<int>((x + 1.f) / 2.0f * (x_size - 1));
		int mapped_z = static_cast<int>((z + 1.f) / 2.0f * (y_size - 1));

		//controllo di range
		if (mapped_x < 0) mapped_x = 0;
		if (mapped_z < 0) mapped_z = 0;

		if (mapped_x >= x_size) mapped_x = x_size;
		if (mapped_z >= y_size) mapped_z = y_size;

		//moltiplicazione per il repeat delle coordinate texture (UV)
		mapped_x = (int)(mapped_x * textureRep) % x_size;
		mapped_z = (int)(mapped_z * textureRep) % y_size;

		int index = (mapped_z * x_size + mapped_x) * n_components;

		return pixelData[index] / 255.f;
	}
};
