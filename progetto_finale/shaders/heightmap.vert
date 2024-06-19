#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vColor;
out vec3 vPos; 

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

uniform sampler2D uColorImage;
uniform float uHeightScale;

void main(void)
{
    float textureRep = 4;
    float duneScale = 0.25;
    float height = texture2D(uColorImage,aTexCoord*textureRep).r * duneScale;
    vec3 newPosition = vec3(aPosition.x, aPosition.y+height, aPosition.z);

    vTexCoord = aTexCoord;
    vColor = aColor;
	vPos = (uView * uModel*vec4(newPosition, 1.0)).xyz;

    gl_Position = uProj * uView * uModel * vec4(newPosition, 1.0);
}