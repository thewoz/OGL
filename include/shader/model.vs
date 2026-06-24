#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

// Transform matrices.
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // world → light clip space (for shadow mapping)

// Outputs to the fragment shader (all in view space).
out vec3 fragPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
out mat3 TBN;       // tangent-space → view-space matrix for normal mapping
out vec4 fragPosLightSpace;

void main() {

    gl_Position = projection * view * model * vec4(position, 1.0f);

    fragPos = vec3(view * model * vec4(position, 1.0f));

    // World-space position projected into the light's clip space for the shadow test.
    fragPosLightSpace = lightSpaceMatrix * model * vec4(position, 1.0f);

    // Normal matrix handles non-uniform scaling; compute once, reuse for T/B/N.
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));

    fragNormal = normalMatrix * normal;

    vec3 T = normalize(normalMatrix * tangent);
    vec3 B = normalize(normalMatrix * bitangent);
    vec3 N = normalize(fragNormal);
    TBN = mat3(T, B, N);

    fragTexCoord = texCoords;
}
