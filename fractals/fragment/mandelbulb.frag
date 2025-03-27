#version 430 core
#pragma optionNV(fastmath on)
#define MaximumRaySteps 250
#define MaximumDistance 4.
#define MinimumDistance 0.00005
#define PI 3.141592653589793238

out vec4 FragColor;

in vec2 TexCoords;

uniform vec2 uResolution;
uniform float uTime;

uniform mat4 uView;
uniform vec3 uCamPos;


vec3 R (vec2 uv, vec3 p, vec3 l, float z) {
  vec3 f = normalize (l - p),
    r = normalize (cross (vec3 (0, 1, 0), f)),
    u = cross (f, r),
    c = p + f * z,
    i = c + uv.x * r + uv.y * u,
    d = normalize (i - p);
  return d;
}

vec3 hsv2rgb (vec3 c) {
  vec4 K = vec4 (1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs (fract (c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix (K.xxx, clamp (p - K.xxx, 0.0, 1.0), c.y);
}

float map (float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float mandelbulb (vec3 position) {
  vec3 z = position;
  float dr = 1.0;
  float r = 0.0;
  int iterations = 0;
  float power = 8.0 + (5.0 * map(sin (uTime * PI / 10.0 + PI), -1.0, 1.0, 0.0, 1.0));
  //power = 8.0;
  for (int i = 0; i < 10; i++) {
    iterations = i;
    r = length(z);

    //viene superata la distanza di 2, che da mandelbrot sappiamo essere divergente
    if (r > 2.0){
      break;
    }

    // convert to polar coordinates
    float theta = acos (z.z / r);
    float phi = atan (z.y, z.x);
    dr = pow (r, power - 1.0) * power * dr + 1.0;

    // scale and rotate the point
    float zr = pow (r, power);
    theta = theta * power;
    phi = phi * power;

    // convert back to cartesian coordinates
    vec2 sincosTheta = sin(vec2(theta, phi));
    vec2 coscosTheta = cos(vec2(theta, phi));

    z = zr * vec3(sincosTheta.x * coscosTheta.y, sincosTheta.y * sincosTheta.x, coscosTheta.x);
    z += position;
  }
  float dst = 0.5 * log (r) * r / dr;
  return dst;
}

// Calculates de distance from a position p to the scene
float DistanceEstimator (vec3 p) {
  float mandelbulb = mandelbulb (p);
  return mandelbulb;
}


vec3 computeNormal(vec3 p) {
    //float eps = 0.0005;
    float eps = MinimumDistance;
    vec2 e = vec2(eps, 0.0);
    return normalize(vec3(
        DistanceEstimator(p + e.xyy) - DistanceEstimator(p - e.xyy),
        DistanceEstimator(p + e.yxy) - DistanceEstimator(p - e.yxy),
        DistanceEstimator(p + e.yyx) - DistanceEstimator(p - e.yyx)
    ));
}

//modello di illuminazione di Blinn-Phong
vec3 blinnPhong(vec3 position, vec3 normal, vec3 lightPos, vec3 viewPos, vec3 diffuseColor) {
    float distanceToLight = length(lightPos - position);
    float attenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * (distanceToLight * distanceToLight));

    // Definizione dei materiali
    float ambientStrength = 0.2;
    float diffuseStrength = 0.7;
    float specularStrength = 0.5;
    float shininess = 16.0;

    // Direzione della luce
    vec3 lightDir = normalize(lightPos - position);

    // **COMPONENTE AMBIENTALE**
    vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);

    // **COMPONENTE DIFFUSA**
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * diffuseColor;

    // **COMPONENTE SPECULARE (Blinn)**
    vec3 viewDir = normalize(viewPos - position);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(0.8, 0.8, 0.8);
    specular *= min(1.0, 1.0 / distanceToLight);

    // SOMMA DELLE COMPONENTI
    return ambient + diffuse + specular;
}

// Marches the ray in the scene
vec4 RayMarcher (vec3 ro, vec3 rd) {
  float steps = 0.0;
  float totalDistance = 0.0;
  float minDistToScene = 100.0;
  vec3 minDistToScenePos = ro;
  float minDistToOrigin = 100.0;
  vec3 minDistToOriginPos = ro;
  vec4 col = vec4 (0.0, 0.0, 0.0, 1.0);
  vec3 curPos = ro;
  bool hit = false;

  for (steps = 0.0; steps < float(MaximumRaySteps); steps++) {
    vec3 p = ro + totalDistance * rd; // Current position of the ray
    float distance = DistanceEstimator(p); // Distance from the current position to the scene
    curPos = ro + rd * totalDistance;
    if (minDistToScene > distance) {
      minDistToScene = distance;
      minDistToScenePos = curPos;
    }
    if(minDistToOrigin > length (curPos)) {
      minDistToOrigin = length (curPos);
      minDistToOriginPos = curPos;
    }
    totalDistance += distance; // Increases the total distance armched
    if (distance < MinimumDistance) {
      hit = true;
      break; // If the ray marched more than the max steps or the max distance, breake out
    }
    else if (distance > MaximumDistance) {
      break;
    }
  }

  if (hit) {
    vec3 normal = computeNormal(curPos);
    vec3 lightPos = vec3(2.0, 2.0, -3.0);
    
    float stepFactor = float(steps) / float(MaximumRaySteps);  // Normalized steps value
    vec3 colorBase = vec3(0.8 + (length(curPos) / 0.5), 1.0, 0.8);  //Color base
    colorBase = hsv2rgb(colorBase);

    colorBase *= (1.0 - stepFactor);  // Darken the color as steps increase
    
    col.rgb = blinnPhong(curPos, normal, lightPos, ro, colorBase);

  }
  else {
    col.rgb = vec3(0.0);
  }
  
  col.rgb /= steps * 0.08; // Ambeint occlusion
  col.rgb *= 3.0;
  
  return col;
}

void main() {
  // Normalized pixel coordinates (from 0 to 1)
  vec2 uv = (TexCoords * 2.0 - 1.0) * uResolution / uResolution.y;

  vec3 ro = uCamPos;
  vec3 rd =  normalize((inverse(uView) * vec4(uv.x, uv.y, -1.0, 0.0)).xyz);

  vec4 col = RayMarcher(ro, rd);

  //Reinhard tone mapping
  col.rgb = col.rgb / (1.0 + col.rgb);
  col.rgb = pow(col.rgb, vec3(1.0 / 2.2)); // gamma correction
  // Output to screen
  FragColor = vec4 (col);

}