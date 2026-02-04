#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 fragNormal;

void main() {
  
  gl_Position = projection * view * model * vec4(position, 1.0f);
  fragPos = vec3(view * model * vec4(position, 1.0f));
  fragNormal = mat3(transpose(inverse(view * model))) * normal;

}
