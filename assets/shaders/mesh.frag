#version 300 es
precision highp float;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normals;

out vec4 FragColor;

void main() {
  vec3 ambient = vec3(0.1, 0.1, 0.1);
  vec3 objectColor = vec3(1.0, 0.5, 0.31);
  vec3 lightColor = vec3(1.0);
  vec3 lightPos = vec3(5.0, 5.0, 0.0);

  vec3 norm = normalize(Normals);
  vec3 lightDir = normalize(lightPos - FragPos);

  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = lightColor * diff;

  vec3 result = (ambient + diffuse) * objectColor;
  FragColor = vec4(result, 1.0);
}