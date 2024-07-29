#version 410 core  
out vec4 color; 

in vec3 vSkyboxTexCoord;

uniform samplerCube uDaySkybox;
uniform samplerCube uNightSkybox;

uniform float uDayNightAngle;

void main(void) 
{ 
	// Normalize the angle to the range [0, 2 * pi]
    float normalizedAngle = radians(mod(uDayNightAngle, 360.0));

    // Calculate blend factor using cosine
    // We shift by pi/2 to make the transition start at 0 and end at 180 degrees
    float blendFactor = 0.5 * (1.0 + cos(normalizedAngle - 3.14159 / 2.0));

    // Sample from both cubemaps
    vec4 dayColor = texture(uDaySkybox, vSkyboxTexCoord);
    vec4 nightColor = texture(uNightSkybox, vSkyboxTexCoord);

    // Interpolate between the day and night colors based on the blend factor
    color = mix(nightColor, dayColor, blendFactor);
} 