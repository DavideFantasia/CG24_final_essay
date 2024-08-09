#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vColor;
out vec3 vPos; 
out vec4 vPosSunLightSpace;
out vec4 vPosLampLightSpace;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

uniform sampler2D uColorImage;
uniform float uHeightScale;
uniform float uTextureRep;

uniform mat4 uSunLightSpaceMatrix;
uniform mat4 uLampLightSpaceMatrix;

uniform int has_heightmap; 

float sampleHeight();

void main(void)
{
    vec3 newPosition;
    if(has_heightmap == 1){
        float height = normalize(texture(uColorImage,aTexCoord*uTextureRep)).r * uHeightScale;
        newPosition = vec3(aPosition.x, aPosition.y+height, aPosition.z);
        //newPosition = vec3(aPosition.x, aPosition.y + sampleHeight(), aPosition.z);
    }else
        newPosition = vec3(aPosition.x, aPosition.y, aPosition.z);

    vTexCoord = aTexCoord;

    vColor = aColor;
	vPos = vec3((uModel*vec4(newPosition, 1.0)));
    vPosSunLightSpace = uSunLightSpaceMatrix * vec4(vPos, 1.0);
    vPosLampLightSpace = uLampLightSpaceMatrix * vec4(vPos, 1.0);

    gl_Position = uProj * uView * uModel * vec4(newPosition, 1.0);
}

float sampleHeight(){
    // Campionatura della heightmap
    vec2 texelSize = 1.0 / textureSize(uColorImage, 0); // Ottieni la dimensione del texel

    float height = 0.0;
    float weightSum = 0.0;

    // Esegui una media pesata della heightmap circostante
    for(int x = -1; x <= 1; ++x){
        for(int y = -1; y <= 1; ++y){
            vec2 offset = vec2(x, y) * texelSize * uTextureRep;
            float sampleHeight = texture(uColorImage, aTexCoord * uTextureRep + offset).r;
            float weight = 1.0; //peso per effetto blur

            height += sampleHeight * weight;
            weightSum += weight;
        }
    }
    height = (height / weightSum) * uHeightScale;

    return height; // Applica la heightmap alla posizione
}