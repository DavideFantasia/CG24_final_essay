#version 430 core

out vec4 FragColor;
in vec2 TexCoords;

uniform int uMaxIterations;

uniform vec2 uResolution;
uniform float uTime;
uniform float uZoom = 1.0;
uniform vec2 uCenter = vec2(0.0, 0.0);

vec3 smoothColor(float t) {
    float r = 0.5 + 0.5 * cos(6.2831 * (t + 0.0));
    float g = 0.5 + 0.5 * cos(6.2831 * (t + 0.33));
    float b = 0.5 + 0.5 * cos(6.2831 * (t + 0.67));
    return vec3(r, g, b);
}

vec2 complexFunc(vec2 z) {
    float x = z.x, y = z.y;
    return vec2(x * x * x - 3.0 * x * y * y - 1.0,
                3.0 * x * x * y - y * y * y);
}

vec2 complexFuncDeriv(vec2 z) {
    float x = z.x, y = z.y;
    return vec2(3.0 * (x * x - y * y), 6.0 * x * y);
}

void main() {
    // Da coordinate pixel in coordinate complesse
    vec2 uv = (TexCoords * 2.0 - 1.0) * uResolution / uResolution.y;
    vec2 z = (uv + uCenter) / uZoom;
    
    const float epsilon = 0.0001;
    int iteration = 0;
    bool converged = false;

    for (int i = 0; i < uMaxIterations; i++) {
        iteration = i;

        vec2 fz = complexFunc(z);
        vec2 fz_deriv = complexFuncDeriv(z);

        vec2 dz = vec2(
            (fz.x * fz_deriv.x + fz.y * fz_deriv.y) / (fz_deriv.x * fz_deriv.x + fz_deriv.y * fz_deriv.y),
            (fz.y * fz_deriv.x - fz.x * fz_deriv.y) / (fz_deriv.x * fz_deriv.x + fz_deriv.y * fz_deriv.y)
        );

        z -= dz;

        if (length(dz) < epsilon) {
            converged = true;
            break;
        }
    }

     vec3 color;
    if (converged) {
        float normIter = float(iteration) / float(uMaxIterations);
        float smoothFactor = 0.5 + 0.5 * cos(normIter * 3.0);
        
        // Palette Bilanciata tra Blu, Viola e Turchese
        if (normIter < 0.3)
            color = mix(vec3(0.2, 0.4, 0.7), vec3(0.3, 0.7, 0.9), smoothFactor);  // Blu -> Turchese
        else if (normIter < 0.6)
            color = mix(vec3(0.3, 0.1, 0.4), vec3(0.4, 0.2, 0.6), smoothFactor);  // Viola Scuro -> Viola Chiaro
        else
            color = mix(vec3(0.1, 0.2, 0.5), vec3(0.2, 0.3, 0.8), smoothFactor);  // Blu Notte -> Blu Elettrico

        color *= smoothFactor * 1.2;  // Evidenziamo i bordi leggermente
    } else {
        color = vec3(0.0); // Nero per punti non convergenti
    }

    FragColor = vec4(color, 1.0);
}
