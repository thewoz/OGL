#version 330 core

uniform vec4 color;

out vec4 outColor;

void main() {

  outColor = vec4(color.xyz, 0.5);

}
