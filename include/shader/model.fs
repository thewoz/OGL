#version 330 core

//
// Phong shading for imported 3D models (glModel / glMesh).
// Everything is computed in view space, where the camera sits at the origin.
// Each texture map, when present, overrides the matching material color.
// Optional shadow mapping is applied when 'withShadow' is enabled.
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
uniform Material  material;
uniform Light     light;
uniform sampler2D shadowMap;     // depth map rendered from the light
uniform bool      withShadow;    // enable/disable shadow mapping

/*****************************************************************************/
// Inputs (from the vertex shader, all in view space unless noted)
/*****************************************************************************/
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;       // fragment position in light space

/*****************************************************************************/
// Output
/*****************************************************************************/
out vec4 color;

/*****************************************************************************/
// Constants
/*****************************************************************************/
const float gamma = 2.2;

/*****************************************************************************/
// Functions
/*****************************************************************************/
float shadowCalculation(vec4 fragPosLightSpace);

/*****************************************************************************/
// Main
/*****************************************************************************/
void main() {

    vec3 norm = normalize(fragNormal);

    // Optional tangent-space normal map.
    if(material.haveNormalsTexture) {
      norm = texture(material.normalsTexture, fragTexCoord).rgb;
      norm = normalize(norm * 2.0 - 1.0);
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

    // Shadowing: shadowed fragments keep only the ambient/emissive terms.
    float shadow = withShadow ? shadowCalculation(fragPosLightSpace) : 0.0;

    vec3 lighting = emissiveColor + ambient + (1.0 - shadow) * (diffuse + specular);

    // Opacity (from the opacity map when available).
    float opacity = material.haveOpacityTexture ? texture(material.opacityTexture, fragTexCoord).r : material.opacity;

    // Gamma correction.
    vec3 gammaCorrected = pow(lighting, vec3(1.0 / gamma));

    color = vec4(gammaCorrected, opacity);

}

/*****************************************************************************/
// shadowCalculation - dynamic bias + 5x5 PCF soft shadows
/*****************************************************************************/
float shadowCalculation(vec4 fragPosLightSpace) {

    // From light clip space to texture space [0,1].
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Outside the shadow map range (e.g. behind the light): no shadow.
    if(projCoords.z > 1.0) return 0.0;

    // Bias based on the angle between the normal and the light.
    float bias = max(0.05 * (1.0 - dot(normalize(fragNormal), normalize(light.direction))), 0.005);

    // Texel size of the shadow map.
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    // 5x5 PCF filtering (radius 2).
    float shadow  = 0.0;
    int   samples = 0;
    int   range   = 2;

    for(int x = -range; x <= range; ++x) {
      for(int y = -range; y <= range; ++y) {
        float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
        shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
        samples++;
      }
    }

    shadow /= float(samples);

    return shadow;
}
