#version 410 core
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vColor;
out vec3 vPos; 
out vec3 vNormal;
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
vec3 computeNormal(vec3 pos, vec2 texCoord);

void main(void)
{
    vec3 newPosition;
    vec3 newNorm = aNormal;
    if(has_heightmap == 1){
        float height = normalize(texture(uColorImage,aTexCoord*uTextureRep)).r * uHeightScale;
        newPosition = vec3(aPosition.x, aPosition.y+height, aPosition.z);
        newNorm = computeNormal(newPosition, aTexCoord);
    }else
        newPosition = vec3(aPosition.x, aPosition.y, aPosition.z);

    vTexCoord = aTexCoord;

    vColor = aColor;
	vPos = vec3((uModel*vec4(newPosition, 1.0)));
    vPosSunLightSpace = uSunLightSpaceMatrix * vec4(vPos, 1.0);
    vPosLampLightSpace = uLampLightSpaceMatrix * vec4(vPos, 1.0);

    vNormal = newNorm;
    gl_Position = uProj * uView * uModel * vec4(newPosition, 1.0);
}


vec3 computeNormal(vec3 pos, vec2 texCoord) {
    vec2 texelSize = 1.0 / textureSize(uColorImage, 0); // Dimensione del texel
    
    float heightL = texture(uColorImage, texCoord * uTextureRep + vec2(-texelSize.x, 0)).r * uHeightScale;
    float heightR = texture(uColorImage, texCoord * uTextureRep + vec2(texelSize.x, 0)).r * uHeightScale;
    float heightD = texture(uColorImage, texCoord * uTextureRep + vec2(0, -texelSize.y)).r * uHeightScale;
    float heightU = texture(uColorImage, texCoord * uTextureRep + vec2(0, texelSize.y)).r * uHeightScale;

    vec3 normal;
    normal.x = heightL - heightR;
    normal.y = aNormal.y + (texture(uColorImage,aTexCoord*uTextureRep)).r * uHeightScale;
    normal.z = heightD - heightU;

    return normalize(normal);
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