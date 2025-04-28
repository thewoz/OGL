#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

// Uniform variables
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

// Output variables
out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragPosLightSpace;

void main() {
    // Projected position
    gl_Position = projection * view * model * vec4(position, 1.0f);

    // Outputs
    fragPos        = vec3(view * model * vec4(position, 1.0f));
    fragNormal     = mat3(transpose(inverse(view * model))) * normal; // correzione per scaling non uniforme
    fragTexCoord   = texCoords;
    fragPosLightSpace = lightSpaceMatrix * model * vec4(position, 1.0);
}
