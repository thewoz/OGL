#version 330 core

// Depth-only pass: render the scene from the light's point of view.
// Only the vertex position is needed; it is shared by every drawable at
// attribute location 0, so this single shader works for models and shapes alike.

layout (location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix; // light projection * light view (world -> light clip)
uniform mat4 model;            // object -> world

void main() {
  gl_Position = lightSpaceMatrix * model * vec4(position, 1.0);
}
