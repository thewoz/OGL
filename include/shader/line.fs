#version 330 core

in vec4 fragColor;
out vec4 outColor;

void main() {
  if(fragColor.w < 0.001) discard;
  outColor = fragColor;
}
