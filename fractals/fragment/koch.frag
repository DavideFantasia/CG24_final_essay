#version 430 core
#define PI 3.141592653589793238
#define RECURSION_LIMIT 20

out vec4 FragColor;

in vec2 TexCoords;

uniform vec2 uResolution;
uniform vec2 uCenter; 
uniform float uZoom;  
uniform float uTime;  

vec2 polarToCartesian(float angle) {
    return vec2(sin(angle), cos(angle));
}

vec2 ref(vec2 uv, vec2 p, float angle) {
    vec2 dir = polarToCartesian(angle); 
    return uv - dir * min(dot(uv - p, dir), 0.0) * 2.0;
}

float kochsCurve(inout vec2 uv, int recursionCount) {
    float scale = 1.25;  
    uv *= scale;          

    if (recursionCount >= 0) {
        uv.y -= sqrt(3.0) / 6.0; 
        uv.x = abs(uv.x);        
        uv = ref(uv, vec2(0.5, 0.0), 11.0 / 6.0 * PI);
        uv.x += 0.5;             
    }

    for (int i = 0; i < min(recursionCount, RECURSION_LIMIT); ++i) {
        uv.x -= 0.5;
        scale *= 3.0;
        uv *= 3.0;

        uv.x = abs(uv.x);  
        uv.x -= 0.5;

        uv = ref(uv, vec2(0.0, 0.0), (2.0 / 3.0) * PI);
    }

    uv.x = abs(uv.x);
    float d = length(uv - vec2(min(uv.x, 1.0), 0.0)) / scale;
    uv /= scale;
    return d;
}

void main() {
    vec2 uv = 2.0 * ((TexCoords - 0.5) * uResolution) / uResolution.y * uZoom + uCenter;
    vec3 col = vec3(0.0);

    int recursionCount = max(0, min(RECURSION_LIMIT, -1 + int(mod(uTime, 20.0 ) * 0.5)));

    float d = kochsCurve(uv, recursionCount);

    float lineSmoothness = 4.0 / uResolution.y;
    col.r += smoothstep(lineSmoothness, 0.0, d) * 0.5;

    lineSmoothness = 1.0 / uResolution.y;
    col.rb += smoothstep(lineSmoothness, 0.0, length(uv.y));
    col.bg += smoothstep(lineSmoothness, 0.0, length(uv.x));

    FragColor = vec4(col, 1.0);
}
