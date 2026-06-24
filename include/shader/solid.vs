#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // world -> light clip space (for shadow mapping)

out vec3 fragPos;
out vec3 fragNormal;
out vec4 fragPosLightSpace;

void main() {

  gl_Position = projection * view * model * vec4(position, 1.0f);
  fragPos = vec3(view * model * vec4(position, 1.0f));
  fragNormal = mat3(transpose(inverse(view * model))) * normal;

  // World-space position projected into the light's clip space for the shadow test.
  fragPosLightSpace = lightSpaceMatrix * model * vec4(position, 1.0f);

}
