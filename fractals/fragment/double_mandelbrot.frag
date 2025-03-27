#version 430 core
#define RECURSION_LIMIT 10000

out vec4 FragColor;
in vec2 TexCoords;

uniform dvec2 uCenter;   // Ora il centro è a precisione doppia
uniform double uZoom;    // Zoom a precisione doppia

uniform vec2 uResolution;
uniform int uFrame;
uniform float uTime;

int mandelbrot(dvec2 c) {
    dvec2 z = dvec2(0.0, 0.0);
    int recursionCount;
    for (recursionCount = 0; recursionCount < RECURSION_LIMIT; recursionCount++) {
        z = dvec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        if (length(z) > 2.0) break;
    }
    return recursionCount;
}

void main() {
    dvec2 uv = dvec2(TexCoords) * 2.0 - 1.0;  // Normalizzazione con double
    dvec2 c = uCenter + uv / uZoom;
    
    int iteration = mandelbrot(c);
    float color = float(iteration) / float(RECURSION_LIMIT);
    
    FragColor = vec4(color, color * 0.6, color * 0.3, 1.0);
}
