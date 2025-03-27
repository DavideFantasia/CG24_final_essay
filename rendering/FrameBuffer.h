#pragma once

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <GL/glew.h>
#include <stdexcept>
#include <string>

class FrameBuffer {
public:
    FrameBuffer()
        : fbo(0), texture(0), rbo(0), width(800), height(600) {}

    FrameBuffer(int w, int h)
        : fbo(0), texture(0), rbo(0), width(w), height(h) {
        init();
    }

    ~FrameBuffer() {
        cleanUp();
    }

    void init(int w=1000, int h=800){
        // Generate the framebuffer
        width = w;
        height = h;

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // Generate the texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Attach the texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        // Generate and attach a renderbuffer for depth and stencil
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        // Check if the framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer is not complete!");
        }

        // Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
    }

    static void unbind(int screenWidth, int screenHeight) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screenWidth, screenHeight);
    }

    GLuint getTexture() const {
        return texture;
    }

    GLuint getFBO() const {
        return fbo;
    }

    void resize(int newWidth, int newHeight) {
        width = newWidth;
        height = newHeight;

        cleanUp();  // Delete existing resources
        init();     // Reinitialize with the new dimensions
    }

    void cleanUp() {
        if (rbo) glDeleteRenderbuffers(1, &rbo);
        if (texture) glDeleteTextures(1, &texture);
        if (fbo) glDeleteFramebuffers(1, &fbo);
    }

    int getWidth() { return width; }
    int getHeight() { return height; }

private:
    GLuint fbo;       // Framebuffer object
    GLuint texture;   // Color texture
    GLuint rbo;       // Renderbuffer object
    int width, height;
};

#endif // FRAMEBUFFER_H
