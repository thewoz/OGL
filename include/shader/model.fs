#version 330 core

//
// Phong shading for imported 3D models (glModel / glMesh).
// Everything is computed in view space, where the camera sits at the origin.
// Each texture map, when present, overrides the matching material color.
//

/*****************************************************************************/
// Material
/*****************************************************************************/
struct Material {

    sampler2D diffuseTexture;
    sampler2D specularTexture;
    sampler2D ambientTexture;
    sampler2D emissiveTexture;
    sampler2D normalsTexture;
    sampler2D opacityTexture;

    bool haveDiffuseTexture;
    bool haveSpecularTexture;
    bool haveAmbientTexture;
    bool haveEmissiveTexture;
    bool haveNormalsTexture;
    bool haveOpacityTexture;

    vec3 emissiveColor;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    float shininess;
    float opacity;
};

/*****************************************************************************/
// Light
/*****************************************************************************/
struct Light {
    vec3 direction;  // directional light (view space); zero means "not used"
    vec3 position;   // point light position (view space)
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

/*****************************************************************************/
// Uniforms
/*****************************************************************************/
uniform Material material;
uniform Light    light;

uniform bool      useShadow;   // sample the shadow map only when enabled
uniform sampler2D shadowMap;

/*****************************************************************************/
// Inputs (from the vertex shader, all in view space)
/*****************************************************************************/
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;
in mat3 TBN;        // tangent-space → view-space, built in model.vs
in vec4 fragPosLightSpace; // fragment position in light clip space

/*****************************************************************************/
// Output
/*****************************************************************************/
out vec4 outColor;

/*****************************************************************************/
// Constants
/*****************************************************************************/
const float gamma = 2.2;

/*****************************************************************************/
// shadowCalc - fraction of this fragment in shadow (0 = lit, 1 = shadow).
// 3x3 PCF with a slope-scaled depth bias to avoid shadow acne.
/*****************************************************************************/
float shadowCalc(vec3 norm, vec3 lightDir) {

    vec3 proj = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj = proj * 0.5 + 0.5;

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

/*****************************************************************************/
// Main
/*****************************************************************************/
void main() {

    vec3 norm = normalize(fragNormal);

    // Normal map: sample in tangent space, then transform to view space via TBN.
    if(material.haveNormalsTexture) {
      vec3 tsNorm = texture(material.normalsTexture, fragTexCoord).rgb;
      tsNorm = normalize(tsNorm * 2.0 - 1.0);
      norm   = normalize(TBN * tsNorm);
    }

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
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Material colors (a texture, when present, replaces the flat color).
    vec3 ambientColor  = material.haveAmbientTexture  ? texture(material.ambientTexture,  fragTexCoord).rgb : material.ambientColor;
    vec3 diffuseColor  = material.haveDiffuseTexture  ? texture(material.diffuseTexture,  fragTexCoord).rgb : material.diffuseColor;
    vec3 specularColor = material.haveSpecularTexture ? texture(material.specularTexture, fragTexCoord).rgb : material.specularColor;
    vec3 emissiveColor = material.haveEmissiveTexture ? texture(material.emissiveTexture, fragTexCoord).rgb : material.emissiveColor;

    // Lighting.
    vec3 ambient  = light.ambient  * ambientColor;
    vec3 diffuse  = light.diffuse  * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;

    // Shadow only attenuates the direct (diffuse + specular) terms.
    float shadow = useShadow ? shadowCalc(norm, lightDir) : 0.0;

    vec3 lighting = emissiveColor + ambient + (1.0 - shadow) * (diffuse + specular);

    // Opacity (from the opacity map when available).
    float opacity = material.haveOpacityTexture ? texture(material.opacityTexture, fragTexCoord).r : material.opacity;

    // Gamma correction.
    vec3 gammaCorrected = pow(lighting, vec3(1.0 / gamma));

    outColor = vec4(gammaCorrected, opacity);

}
