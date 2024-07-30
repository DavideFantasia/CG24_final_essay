#version 410 core

out vec4 color;

in vec2 vTexCoord;
in vec3 vColor; 
in vec3 vPos; 
in vec4 vPosSunLightSpace;
in vec4 vPosLampLightSpace;

struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuse);

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
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse);

//struct per i materiali
struct Material {
	sampler2D diffuse_map;

	vec3 specular;

	float metallic;
	float roughness;
	float ao;

};

uniform Material material;
uniform vec3 uViewPos;
uniform sampler2D uSunShadowMap;
uniform sampler2D uLampShadowMap;
uniform float uBias;

//funzione per calcolare il fattore di ombra nel frammento
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir, int shadowSample, sampler2D shadowMap){
	//normalizzazione
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	//passaggio a cordinate NDC
	projCoords = projCoords * 0.5 + 0.5;

	//sampling dalla shadow map
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	//check per vedere se siamo in ombra nel frammento attuale
	float bias = uBias;
	float shadow = 0.0;

	//sampling dei n texel attorno all'attuale frammento per poter smussare il bordo dell'ombra
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -shadowSample; x <= shadowSample; ++x){
		for(int y = -shadowSample; y <= shadowSample; ++y){
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= pow(((2*shadowSample)+1),2);

	return shadow;
}

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(void) 
{ 
	float gamma = 2.2;
	vec3 viewDir = normalize(uViewPos - vPos); //V
	vec3 norm = normalize(cross(dFdx(vPos),dFdy(vPos))); //N

	vec3 diffuse = vec3(texture(material.diffuse_map, vTexCoord));

	vec3 result = vec3(CalcDirLight(dirLight, norm,viewDir, diffuse));
	result +=  vec3(CalcSpotLight(spotlight, norm, vPos, viewDir, diffuse));

	vec3 ambient = spotlight.ambient * diffuse * material.ao;
	result += ambient;

	// HDR tonemapping
    result = result / (result + vec3(1.0));

	result = pow(result,vec3(1.0/gamma));

	color = vec4(result, 1.0);
} 

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuse){
	vec3 F0 = vec3(0.04); 
	
    F0 = mix(F0, diffuse, material.metallic);

	// calculate per-light radiance
	vec3 L = normalize(light.direction);
    vec3 H = normalize(viewDir + L);
    float distance = length(light.direction - vPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.diffuse * attenuation;

	// Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, H, material.roughness);   
    float G   = GeometrySmith(normal, viewDir, L, material.roughness);      
    vec3 F    = fresnelSchlick(clamp(dot(H, viewDir), 0.0, 1.0), F0);
       
	vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
	// kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - material.metallic;	  

	// scale light by NdotL
    float NdotL = max(dot(normal, L), 0.0);     
	
	//float shadow = ShadowCalculation(vPosLightSpace, normal, viewDir, 2);
	float shadow = ShadowCalculation(vPosSunLightSpace, normal, viewDir, 2, uSunShadowMap);

	// add to outgoing radiance Lo
    return (kD * diffuse / PI + material.specular) * radiance * NdotL * (1.0-shadow);  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse){
	vec3 F0 = vec3(0.04); 
	
    F0 = mix(F0, diffuse, material.metallic);

	// calculate per-light radiance
	vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(viewDir + L);
    // attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance +light.quadratic * (distance * distance));
    vec3 radiance = light.diffuse * attenuation;

	// soft-shading of the border
	float theta = dot(L, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, H, material.roughness);   
    float G   = GeometrySmith(normal, viewDir, L, material.roughness);      
    vec3 F    = fresnelSchlick(clamp(dot(H, viewDir), 0.0, 1.0), F0);
       
	vec3 numerator    = NDF * G * F; 
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;
    
	// kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - material.metallic;	  

	// scale light by NdotL
    float NdotL = max(dot(normal, L), 0.0);     
	
	//float shadow = ShadowCalculation(vPosLightSpace, normal, viewDir, 2);
	float shadow = ShadowCalculation(vPosLampLightSpace, normal, viewDir, 2, uLampShadowMap);

	// add to outgoing radiance Lo
    return (kD * diffuse / PI + material.specular) * radiance * NdotL * intensity * (1.0-shadow);  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
}