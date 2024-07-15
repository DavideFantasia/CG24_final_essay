#version 330 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal;
 
out vec3 vSkyboxTexCoord;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;


void main(void) 
{ 
    vec4 pos = uProj * uView *uModel * vec4(aPosition, 1.0);
    // z as w per fissare la z ad 1 e quindi dietro tutto in NDC
    gl_Position = pos.xyww;
    vSkyboxTexCoord = vec3(aPosition.x, aPosition.y, -aPosition.z);
}
