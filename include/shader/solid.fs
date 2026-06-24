#version 330 core

//
// Simple Phong shading for the solid base objects (sphere, ellipse, cuboid, ...).
// Everything is computed in view space, where the camera sits at the origin.
// Optional shadow mapping darkens fragments occluded from the scene light.
//

struct Light {
    vec3 direction;  // directional light (view space); zero means "not used"
    vec3 position;   // point light position (view space)
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3  color;   // base color of the object
uniform Light light;

uniform bool      useShadow;   // sample the shadow map only when enabled
uniform sampler2D shadowMap;

in  vec3 fragPos;          // fragment position in view space
in  vec3 fragNormal;       // fragment normal   in view space
in  vec4 fragPosLightSpace; // fragment position in light clip space
out vec4 outColor;

const float shininess = 32.0;
const float gamma     = 2.2;

//
// shadowCalc - fraction of this fragment that is in shadow (0 = lit, 1 = shadow).
// 3x3 PCF with a slope-scaled depth bias to avoid shadow acne.
//
float shadowCalc(vec3 norm, vec3 lightDir) {

    // Perspective divide, then map from NDC [-1,1] to texture [0,1].
    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

    // Outside the light frustum (beyond far plane) => never in shadow.
    if(proj.z > 1.0) return 0.0;

    float bias = max(0.0025 * (1.0 - dot(norm, lightDir)), 0.0008);

    float shadow = 0.0;
    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float closest = texture(shadowMap, proj.xy + vec2(x, y) * texel).r;
            shadow += (proj.z - bias > closest) ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {

    vec3 norm = normalize(fragNormal);

    // In view space the eye is at the origin, so -fragPos points to the camera.
    vec3 viewDir = normalize(-fragPos);

    // Choose the light direction: explicit direction, else point light,
    // else a head light coming straight from the camera.
    vec3 lightDir;
    if(length(light.direction) > 0.001)      lightDir = normalize(-light.direction);
    else if(length(light.position) > 0.001)  lightDir = normalize(light.position - fragPos);
    else                                     lightDir = viewDir;

    // Phong terms.
    float diff = max(dot(norm, lightDir), 0.0);
    vec3  reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    vec3 ambient  = light.ambient  * color;
    vec3 diffuse  = light.diffuse  * diff * color;
    vec3 specular = light.specular * spec;

    // Shadow only attenuates the direct (diffuse + specular) terms.
    float shadow = useShadow ? shadowCalc(norm, lightDir) : 0.0;

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    // Gamma-correct the final color for display (consistent with model.fs).
    outColor = vec4(pow(lighting, vec3(1.0 / gamma)), 1.0);
}
