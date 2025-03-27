#version 430 core

layout(location = 0) in vec2 aPos;  // Posizione del vertice
layout(location = 1) in vec2 aTex;  // Coordinate UV

out vec2 TexCoords;

void main() {
    TexCoords = aTex;
    gl_Position = vec4(aPos, 0.0, 1.0);
}