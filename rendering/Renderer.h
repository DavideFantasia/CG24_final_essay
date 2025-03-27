#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#include "Shader.h"
#include "Framebuffer.h"

class Renderer {
public:
    GLuint quadVAO, quadVBO;
    Shader quadShader;

    Renderer() : quadVAO(0), quadVBO(0) {}

    // Inizializzazione
    void init(const GLchar *quadVertexPath,const GLchar *quadFragmentPath) {
        // crea il programma shader per il rendering del quad
        quadShader.create_program(quadVertexPath, quadFragmentPath);
    }

    // Funzione per configurare il quad (una tantum)
    void setupQuad() {
        if (quadVAO == 0) {
            float quadVertices[] = {
                // Posizione      // UV
                -1.0f,  1.0f,     0.0f, 1.0f, // Vertice in alto a sinistra
                -1.0f, -1.0f,     0.0f, 0.0f, // Vertice in basso a sinistra
                 1.0f, -1.0f,     1.0f, 0.0f, // Vertice in basso a destra

                -1.0f,  1.0f,     0.0f, 1.0f, // Vertice in alto a sinistra
                 1.0f, -1.0f,     1.0f, 0.0f, // Vertice in basso a destra
                 1.0f,  1.0f,     1.0f, 1.0f  // Vertice in alto a destra
            };

            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);

            glBindVertexArray(quadVAO);

            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

            glEnableVertexAttribArray(0); // Posizione
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

            glEnableVertexAttribArray(1); // UV
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        }
    }

    // Renderizzazione del quad con una texture
    void renderQuad(GLuint textureID) {
        // Usa lo shader del quad
        quadShader.use();

        // Collega la texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        //passaggio texture allo shader

        glUniform1i(glGetUniformLocation(quadShader.program, "uTexture"), 0);
        
        // Renderizza il quad
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    // Distruzione delle risorse
    void cleanUp() {
        if (quadVAO != 0) {
            glDeleteVertexArrays(1, &quadVAO);
            glDeleteBuffers(1, &quadVBO);
        }
    }
};
