#version 300 es
precision highp float;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normals;
in vec4 Colors;
in vec3 CamPos;

out vec4 FragColor;

struct Material {
  vec3 baseColorFactor;
  float metallicFactor;
  float roughnessFactor;
  vec3 emissiveFactor;
  float emissiveStrength;
};

uniform Material material;

const vec3 ambient = vec3(0.1);
const vec3 lightColor = vec3(1.0, 0.27, 0.12);
const vec3 lightPos = vec3(0.0, 1.75, -17.85);

vec3 ACESFilm(vec3 x) {
  const float A = 2.51;
  const float B = 0.03;
  const float C = 2.43;
  const float D = 0.59;
  const float E = 0.14;

  return clamp((x * (A * x + B)) / (x * (C * x + D) + E), 0.0, 1.0);
}

vec3 getLitResult(vec3 objectColor) {

  vec3 norm = normalize(Normals);
  vec3 lightDir = normalize(lightPos - FragPos);

  // estimate specular strength from the metallic factor and roughness factor
  float specularStrength =
      0.5 + 0.5 * (1.0 - material.metallicFactor) * material.roughnessFactor;
  vec3 viewDir = normalize(CamPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
  vec3 specular = specularStrength * spec * lightColor;

  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = lightColor * diff;

  vec3 result = (ambient + diffuse + specular) * objectColor;

  return result;
}

void main() {
  vec3 objectColor = material.baseColorFactor * (1.0 - material.metallicFactor) * Colors.rgb;
  objectColor += material.emissiveFactor * material.emissiveStrength;

  // only apply lighting if the object is not emissive
  if (material.emissiveStrength > 0.0) {
    FragColor = vec4(objectColor, 1.0);
    return;
  }

  vec3 result = objectColor; //getLitResult(objectColor);

  result = ACESFilm(result);
  // gamma correction
  result = pow(result, vec3(1.0 / 2.2));

  FragColor = vec4(result, 1.0);
}