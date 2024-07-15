#version 330 core  
out vec4 color; 

in vec3 vSkyboxTexCoord;

uniform samplerCube uSkybox;

void main(void) 
{ 
	//skybox
	color = texture(uSkybox,vSkyboxTexCoord);

	//prof
	//color = texture(uSkybox,normalize(vSkyboxTexCoord.xyz)); 		
} 