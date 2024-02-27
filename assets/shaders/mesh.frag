#version 300 es
precision highp float;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normals;

out vec4 FragColor;

struct Material {
  vec3 baseColorFactor;
  float metallicFactor;
  float roughnessFactor;
  vec3 emissiveFactor;
  float emissiveStrength;
};

uniform Material material;

void main() {
  vec3 ambient = vec3(0.1, 0.1, 0.1);
  vec3 objectColor = material.baseColorFactor * (1.0 - material.metallicFactor);
  objectColor += material.emissiveFactor * material.emissiveStrength;

  // only apply lighting if the object is not emissive
  if (material.emissiveStrength > 0.0) {
    FragColor = vec4(objectColor, 1.0);
    return;
  }

  vec3 lightColor = vec3(1.0);
  vec3 lightPos = vec3(5.0, 5.0, 0.0);

  vec3 norm = normalize(Normals);
  vec3 lightDir = normalize(lightPos - FragPos);

  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = lightColor * diff;

  vec3 result = (ambient + diffuse) * objectColor;
  FragColor = vec4(result, 1.0);
}