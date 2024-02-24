#version 300 es
precision highp float;

out vec4 FragColor;

uniform vec3 objectColor;

void main() {
  // red color
  FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  // FragColor = vec4(lightColor * objectColor, 1.0);
}