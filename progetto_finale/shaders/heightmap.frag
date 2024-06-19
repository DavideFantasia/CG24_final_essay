#version 410 core

out vec4 color;

in vec2 vTexCoord;
in vec3 vColor; 
in vec3 vPos; 

uniform sampler2D uSandTexture;

void main(void) 
{ 
   // this part uses concept we haven't covered yet. Please ignore it
   vec3 N = normalize(cross(dFdx(vPos),dFdy(vPos)));
   vec3 L0 = normalize(vec3(2,2,0)-vPos);
   vec3 L1 = normalize(vec3(-2,1,0)-vPos);
   float contrib = (max(0.f,dot(N,L0))+max(0.f,dot(N,L1)))*0.5;

   vec3 fixedColor = vec3(237.0 / 255.0, 211.0 / 255.0, 140.0 / 255.0);
   //color = vec4(fixedColor*contrib, 1.0); 
   vec3 texColor = texture2D(uSandTexture,vTexCoord).rgb;
   color = vec4(texColor*contrib,1.0);
} 