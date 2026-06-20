#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

// Transform matrices.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;   // world -> light clip space (shadow mapping)

// Outputs to the fragment shader (positions/normals are in view space).
out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out vec4 fragPosLightSpace;      // fragment position in light space

void main() {

    // Clip-space position.
    gl_Position = projection * view * model * vec4(position, 1.0f);

    // View-space position (the camera is at the origin in view space).
    fragPos = vec3(view * model * vec4(position, 1.0f));

    // Normal matrix: handles non-uniform scaling; result stays in view space.
    fragNormal = mat3(transpose(inverse(view * model))) * normal;

    fragTexCoord = texCoords;

    // Fragment position in the light's clip space, used by the shadow map.
    fragPosLightSpace = lightSpaceMatrix * model * vec4(position, 1.0);
}
