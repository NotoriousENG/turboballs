#version 300 es
precision highp float;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormals;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  FragPos = vec3(model * vec4(aPos, 1.0));
  TexCoords = aTexCoords;
  Normals = mat3(transpose(inverse(model))) * aNormals;
  gl_Position = projection * view * vec4(FragPos, 1.0);
}
