#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

out vec4 fragColor;
out vec3 fragPosView;   // point centre in view space, for lighting in the fragment

void main() {

  vec4 posView = view * model * vec4(position, 1.0f);

  gl_Position = projection * posView;

  // Scale the sprite by distance so points keep a consistent apparent size.
  float Z = length(posView.xyz);
  gl_PointSize = pointSize / Z;

  fragColor   = color;
  fragPosView = posView.xyz;

}
