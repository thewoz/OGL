#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

// uniform variable
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;

out vec4 fragColor;

void main() {
  
  gl_Position = projection * view * model * vec4(position, 1.0f);
  
  vec3 posEye = vec3(view * model * vec4(position, 1.0f));
  
  float Z = length(posEye);
  
  gl_PointSize = pointSize / Z;
      
  fragColor = color;

}

