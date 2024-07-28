#version 410 core

out vec4 color;

in vec2 vTexCoord;
in vec3 vColor; 
in vec3 vPos; 
in vec4 vPosLightSpace;

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

struct SpotLight{
	vec3 position;
	vec3 direction;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float cutOff;
	float outerCutOff;
};
uniform SpotLight spotlight;
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

//struct per i materiali
struct Material {
	sampler2D diffuse_map;

	vec3 specular;
	float shininess;
};

uniform Material material;
uniform vec3 uViewPos;
uniform sampler2D uShadowMap;
uniform float uBias;

//funzione per calcolare il fattore di ombra nel frammento
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	//normalizzazione
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	//passaggio a cordinate NDC
	projCoords = projCoords * 0.5 + 0.5;

	//sampling dalla shadow map
	float closestDepth = texture(uShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	//check per vedere se siamo in ombra nel frammento attuale
	float bias = uBias;
	float shadow = 0.0;

	//sampling dei n texel attorno all'attuale frammento per poter smussare il bordo dell'ombra
	int shadowSample = 2;
	vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
	for(int x = -shadowSample; x <= shadowSample; ++x){
		for(int y = -shadowSample; y <= shadowSample; ++y){
			float pcfDepth = texture(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= pow(((2*shadowSample)+1),2);

	return shadow;
}


void main(void) 
{ 
	float gamma = 2.2;
	vec3 viewDir = normalize(uViewPos - vPos);
	vec3 norm = normalize(cross(dFdx(vPos),dFdy(vPos)));

	vec3 result = vec3(CalcSpotLight(spotlight, norm, vPos, viewDir));
	result += vec3(CalcDirLight(dirLight, norm,viewDir));

	result = pow(result,vec3(1.0/gamma));

	color = vec4(result, 1.0);
} 

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir = normalize(-light.direction);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

	// combine results
	vec3 diffuseFactor = vec3(texture(material.diffuse_map, vTexCoord));
	vec3 ambient = light.ambient * diffuseFactor;
	vec3 diffuse = light.diffuse * diff * diffuseFactor;
	vec3 specular = light.specular * spec * material.specular;
	
	float shadow = ShadowCalculation(vPosLightSpace, normal, viewDir);
	return (ambient + (diffuse + specular)*(1.0-shadow));
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
	
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance +
	light.quadratic * (distance * distance));
	
	// combine results
	vec3 diffuseFactor = vec3(texture(material.diffuse_map, vTexCoord));
	vec3 ambient = light.ambient * diffuseFactor;
	vec3 diffuse = light.diffuse * diff * diffuseFactor;
	vec3 specular = light.specular * spec * material.specular;
	
	// soft-shading of the border
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	diffuse *= intensity;
	specular *= intensity;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	float shadow = ShadowCalculation(vPosLightSpace, normal, viewDir);
	return (ambient + (diffuse + specular)*(1.0-shadow));
}