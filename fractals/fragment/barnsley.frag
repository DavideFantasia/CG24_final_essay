#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec2 uResolution;

uniform vec2 uCenter; // Centro del frattale
uniform float uZoom;  // Livello di zoom
uniform float uTime; //tempo in secondi

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 barnsleyFern(vec2 point, float r) {
    if (r < 0.01) { 
        point = vec2(0.0, 0.16 * point.y);
    }
    else if (r < 0.86) {
        point = vec2(
            0.85 * point.x + 0.04 * point.y,
            -0.04 * point.x + 0.85 * point.y + 1.6
        );
    }
    else if (r < 0.93) {
        point = vec2(
            0.2 * point.x - 0.26 * point.y,
            0.23 * point.x + 0.22 * point.y + 1.6
        );
    }
    else {
        point = vec2(
            -0.15 * point.x + 0.28 * point.y,
            0.26 * point.x + 0.24 * point.y + 0.44
        );
    }
    return point;
}

void main() {
    vec2 uv = 2.0 * ((TexCoords - 0.5)*uResolution)/uResolution.y * uZoom + uCenter;

    vec2 point = vec2(0.0);
    vec3 col = vec3(0.0);

    const int IterationsPerFrame = 1000;  // Più alto = frattale più veloce ma più pesante da calcolare

    for (int i = 0; i < IterationsPerFrame; ++i) {
        float r = rand(vec2(float(i), uTime)); 
        point = barnsleyFern(point, r);

        // Trasforma il punto in coordinate dello schermo normalizzate
        vec2 screenPos = point;
        screenPos.x = (screenPos.x + 2.182) / 4.837;
        screenPos.y = screenPos.y / 9.9983;

        // Controlla se il punto corrente è vicino al pixel corrente
        if (distance(uv, screenPos * 2.0 - 1.0) < 0.005) { 
            col = vec3(0.0, 1.0, 0.0); // Verde per il frattale
            break;
        }
    }

    FragColor = vec4(col, 1.0);
}
