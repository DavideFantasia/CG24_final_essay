#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D uTexture; // Texture da mostrare

void main() {
    FragColor = texture(uTexture, TexCoords);
    //FragColor = vec4(TexCoords, 0.0, 1.0);
}
