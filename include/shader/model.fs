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

/*****************************************************************************/
// Inputs (from the vertex shader, all in view space)
/*****************************************************************************/
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;

/*****************************************************************************/
// Output
/*****************************************************************************/
out vec4 color;

/*****************************************************************************/
// Constants
/*****************************************************************************/
const float gamma = 2.2;

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

    vec3 lighting = emissiveColor + ambient + diffuse + specular;

    // Opacity (from the opacity map when available).
    float opacity = material.haveOpacityTexture ? texture(material.opacityTexture, fragTexCoord).r : material.opacity;

    // Gamma correction.
    vec3 gammaCorrected = pow(lighting, vec3(1.0 / gamma));

    color = vec4(gammaCorrected, opacity);

}
