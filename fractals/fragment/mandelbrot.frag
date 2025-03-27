#version 430 core
//#pragma optionNV(fastmath on)
#define RECURSION_LIMIT 100000

out vec4 FragColor;

in vec2 TexCoords;

uniform vec2 uResolution;

uniform vec2 uCenter; // Centro del frattale
uniform float uZoom;  // Livello di zoom

// Method for the mathematical constructoin of the mandelbrot set
int mandelbrot (vec2 c) {
  int recursionCount;

  vec2 z = vec2 (0.0, 0.0);

  for (recursionCount = 0; recursionCount < RECURSION_LIMIT; recursionCount++) {
    z = vec2 (z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;

    if (length (z) > 2.0) {
      break;
    }
  }

  return recursionCount;
}

void main() {

    vec2 res = uResolution;
    vec2 center = uCenter;
    float zoom = uZoom;
    
    vec2 c = 2.0 * ((TexCoords - 0.5)*res)/res.y * zoom + center;
    vec2 z = vec2(0.0);

    uint maxIterations = 100u;
    uint i;
    for (i = 0u; i < maxIterations; ++i) {
        if (dot(z, z) > 4.0) break;
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
    }

    float t = float(i) / float(maxIterations);
 
    vec3 color;
    if (i == maxIterations) { // If it is in the mandelbrot set itself, colors black
        color = vec3(0.05);
    }
    else { // If its close to the mandelbrot set, colors based on how close it is to beeing on the mandelbrot
        float offset = 0.5;
        float smoothness = 1.0;
        color.r = smoothstep (0.0, smoothness, t) * (TexCoords.x * 0.5 + 0.5);
        color.b = smoothstep (0.0, smoothness, t) * (TexCoords.y * 0.5 + 0.5);
        color.g = smoothstep (0.0, smoothness, t) * (-TexCoords.x * 0.5 + 0.5);
    }

    FragColor = vec4(color, 1.0);
}