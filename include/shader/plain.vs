#version 330 core

layout (location = 0) in vec3 position;

// uniform variable
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 viewport;

out vec2 lineCenter;

void main() {
  
  // coordinata proiettiva del punto
  gl_Position = projection * view * model * vec4(position, 1.0f);

  lineCenter = 0.5 * (gl_Position.xy + vec2(1,1))*viewport;

}

