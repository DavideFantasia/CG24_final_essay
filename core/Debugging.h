#pragma once

/*
* Credits to Professor Ganovelli
*/

#ifndef _OPENGL_DEBUG_
#define _OPENGL_DEBUG_

#include <GL/glew.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

static void printout_opengl_glsl_info() {
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* version = glGetString(GL_VERSION);
	const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	std::cout << "GL Vendor            :" << vendor << std::endl;
	std::cout << "GL Renderer          :" << renderer << std::endl;
	std::cout << "GL Version (string)  :" << version << std::endl;
	std::cout << "GLSL Version         :" << glslVersion << std::endl;
}

static bool check_gl_errors(int line, const char* file, bool exit_on_error = true) {
	int err = glGetError();
	std::string err_string;
	switch (err) {

	case GL_INVALID_ENUM:
		std::cout << "GL_INVALID_ENUM\n An unacceptable value is specified for an enumerated argument.The offending command is ignoredand has no other side effect than to set the error flag." << "Line: " << line << " File: " << file << "\n";
		break;

	case GL_INVALID_VALUE:
		std::cout << "GL_INVALID_VALUE\n A numeric argument is out of range.The offending command is ignoredand has no other side effect than to set the error flag." << "Line: " << line << " File: " << file << "\n";
		break;

	case GL_INVALID_OPERATION:
		std::cout << "GL_INVALID_OPERATION\n The specified operation is not allowed in the current state.The offending command is ignoredand has no other side effect than to set the error flag." << "Line: " << line << " File: " << file << "\n";
		break;

	case GL_INVALID_FRAMEBUFFER_OPERATION:
		std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION\n  The framebuffer object is not complete.The offending command is ignoredand has no other side effect than to set the error flag." << "Line: " << line << " File: " << file << "\n";
		break;

	case GL_OUT_OF_MEMORY:
		std::cout << "GL_OUT_OF_MEMORY\n There is not enough memory left to execute the command.The state of the GL is undefined, except for the state of the error flags, after this error is recorded." << "Line: " << line << " File: " << file << "\n";
		break;

	case GL_STACK_UNDERFLOW:
		std::cout << "GL_STACK_UNDERFLOW\n An attempt has been made to perform an operation that would cause an internal stack to underflow." << "Line: " << line << " File: " << file << "\n";
		break;

	case GL_STACK_OVERFLOW:
		std::cout << "GL_STACK_OVERFLOW\n An attempt has been made to perform an operation that would cause an internal stack to overflow." << "Line: " << line << " File: " << file << "\n";
		break;
	}
	bool ok_res = (err == GL_NO_ERROR);
	if (!ok_res && exit_on_error) {
		std::string test;
		std::cin >> test;
		exit(-1);
	}
	return ok_res;
}

static bool check_gl_errors(bool exit_on_error = true) {
	return check_gl_errors(-1, ".", exit_on_error);
}

static bool check_shader(GLuint s, bool exit_on_error = true) {
	std::vector<GLchar> buf;
	GLint l;
	glGetShaderiv(s, GL_COMPILE_STATUS, &l);

	if (l == GL_FALSE) {
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &l);
		buf.resize(l);
		glGetShaderInfoLog(s, l, &l, &buf[0]);
		std::cout << &buf[0] << std::endl;
		if (exit_on_error)
			exit(0);
		return false;
	}
	return true;
}

static bool validate_shader_program(GLuint  s) {
	GLint res;
	glValidateProgram(s);
	glGetProgramiv(s, GL_VALIDATE_STATUS, &res);
	std::cout << "validation of program " << s << ", result: " << res << std::endl;
	if (res != GL_TRUE) {
		GLchar infoLog[65536];
		int length;
		glGetProgramInfoLog(s, 65536, &length, &infoLog[0]);
		std::cout << infoLog << "\n";
		return false;
	}
	glGetProgramiv(s, GL_LINK_STATUS, &res);
	std::cout << "linking of program " << s << ", result: " << res << std::endl;
	if (res != GL_TRUE) return false;

	glGetProgramiv(s, GL_ACTIVE_ATTRIBUTES, &res);
	std::cout << "active attribute of program " << s << ", result: " << res << std::endl;

	glGetProgramiv(s, GL_ACTIVE_UNIFORMS, &res);
	std::cout << "active uniform  of program " << s << ", result: " << res << std::endl;

	glGetProgramiv(s, GL_ACTIVE_UNIFORM_MAX_LENGTH, &res);
	std::cout << "active uniform Max Length of program " << s << ", result: " << res << "\n" << std::endl;
	return true;
}


// Funzione di supporto per convertire i tipi OpenGL in stringhe leggibili
static std::string getGLTypeString(GLenum type) {
	switch (type) {
	case GL_FLOAT: return "float";
	case GL_FLOAT_VEC2: return "vec2";
	case GL_FLOAT_VEC3: return "vec3";
	case GL_FLOAT_VEC4: return "vec4";
	case GL_INT: return "int";
	case GL_INT_VEC2: return "ivec2";
	case GL_INT_VEC3: return "ivec3";
	case GL_INT_VEC4: return "ivec4";
	case GL_BOOL: return "bool";
	case GL_BOOL_VEC2: return "bvec2";
	case GL_BOOL_VEC3: return "bvec3";
	case GL_BOOL_VEC4: return "bvec4";
	case GL_FLOAT_MAT2: return "mat2";
	case GL_FLOAT_MAT3: return "mat3";
	case GL_FLOAT_MAT4: return "mat4";
	case GL_SAMPLER_2D: return "sampler2D";
	case GL_SAMPLER_CUBE: return "samplerCube";
	default: return "unknown";
	}
}


// Funzione per stampare tutte le uniform attive di un programma shader
static void printActiveUniforms(GLuint program) {
	// Assicurati che il programma shader sia valido
	if (!glIsProgram(program)) {
		std::cerr << "Errore: Il programma shader fornito non è valido!" << std::endl;
		return;
	}

	// Ottieni il numero totale di uniform attive e la lunghezza massima dei loro nomi
	GLint numUniforms = 0;
	GLint maxNameLength = 0;
	glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);
	glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);

	// Controlla se ci sono uniform attive
	if (numUniforms == 0) {
		std::cout << "Non ci sono uniform attive nel programma shader." << std::endl;
		return;
	}

	// Buffer per i nomi delle uniform
	std::vector<GLchar> uniformName(maxNameLength);

	std::cout << "Uniform attive nel programma shader (ID: " << program << "):" << std::endl;

	// Itera attraverso tutte le uniform attive
	for (GLint i = 0; i < numUniforms; ++i) {
		GLsizei length = 0;
		GLint size = 0;
		GLenum type = 0;

		// Ottieni informazioni sulla uniform
		glGetActiveUniform(program, i, maxNameLength, &length, &size, &type, uniformName.data());

		// Ottieni il location della uniform
		GLint location = glGetUniformLocation(program, uniformName.data());

		// Stampa le informazioni
		std::cout << "  [" << i << "] Nome: " << uniformName.data()
			<< ", Location: " << location
			<< ", Tipo: " << getGLTypeString(type)
			<< ", Dimensione: " << size << std::endl;
	}
}


#endif
