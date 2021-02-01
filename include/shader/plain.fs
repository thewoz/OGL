#version 330 core

uniform vec3 color;

out vec4 outColor;

void main() {

  float dist = length(vec2(0,0)-gl_PointCoord);
 
  outColor = vec4(color, 1.0);
 
}
