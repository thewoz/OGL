#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int useUniformColor;
uniform vec4 uniformColor;

out vec4 vertColor;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0f);
  vertColor = (useUniformColor == 1) ? uniformColor : color;
}
