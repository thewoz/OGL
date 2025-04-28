#version 330 core

/*****************************************************************************/
// Material
/*****************************************************************************/
struct Material {
  
    sampler2D diffuseTexture;
    sampler2D specularTexture;
    sampler2D ambientTexture;
    sampler2D emissiveTexture;
    sampler2D normalsTexture;
    sampler2D shininessTexture;
    sampler2D opacityTexture;
    sampler2D lightTexture;
    sampler2D reflectionTexture;
    
    bool haveDiffuseTexture;
    bool haveSpecularTexture;
    bool haveAmbientTexture;
    bool haveEmissiveTexture;
    bool haveNormalsTexture;
    bool haveShininessTexture;
    bool haveOpacityTexture;
    bool haveLightTexture;
    bool haveReflectionTexture;
    
    vec3 emissiveColor;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 reflectiveColor;
    vec3 transparentColor;
    
    float shininess;
    float opacity;
};

/*****************************************************************************/
// Light
/*****************************************************************************/
struct Light {
    vec3 direction;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

/*****************************************************************************/
// Uniforms
/*****************************************************************************/
uniform Material material;
uniform Light light;
uniform sampler2D shadowMap;
uniform bool withShadow;

/*****************************************************************************/
// Inputs
/*****************************************************************************/
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

/*****************************************************************************/
// Outputs
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

    // Normal mapping
    if (material.haveNormalsTexture) {
        norm = texture(material.normalsTexture, fragTexCoord).rgb;
        norm = normalize(norm * 2.0 - 1.0);
    }

    vec3 viewDir = normalize(-fragPos);

    vec2 texCoords = fragTexCoord;

    // Light direction logic
    vec3 lightDir = vec3(0.0);
    if (length(light.direction) > 0.001) {
        lightDir = normalize(-light.direction);
    } else {
        lightDir = normalize(light.position - fragPos);
    }

    // Shading terms
    float diff = max(dot(norm, lightDir), 0.0);
    float specularStrength = material.haveShininessTexture
                            ? texture(material.shininessTexture, texCoords).r
                            : material.shininess;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularStrength);

    // Sample textures
    vec3 ambientColor    = material.haveAmbientTexture  ? vec3(texture(material.ambientTexture, texCoords)) : material.ambientColor;
    vec3 diffuseColor    = material.haveDiffuseTexture  ? vec3(texture(material.diffuseTexture, texCoords)) : material.diffuseColor;
    vec3 specularColor   = material.haveSpecularTexture ? vec3(texture(material.specularTexture, texCoords)) : material.specularColor;
    vec3 emissiveColor   = material.haveEmissiveTexture ? vec3(texture(material.emissiveTexture, texCoords)) : material.emissiveColor;
    vec3 reflectionColor = material.haveReflectionTexture ? vec3(texture(material.reflectionTexture, texCoords)) : material.reflectiveColor;

    // Lighting
    vec3 ambient  = light.ambient * ambientColor;
    vec3 diffuse  = light.diffuse * diff * diffuseColor;
    vec3 specular = light.specular * spec * specularColor;
    vec3 reflection = reflectionColor;

    float shadow = withShadow ? shadowCalculation(fragPosLightSpace) : 0.0;

    vec3 lighting = emissiveColor + ambient + (1.0 - shadow) * (diffuse + specular + reflection);

    // Apply lightmap if present
    if (material.haveLightTexture) {
        lighting *= texture(material.lightTexture, texCoords).rgb;
    }

    // Transparency color (kt)
    lighting = mix(lighting, material.transparentColor, 1.0 - material.opacity);

    // Opacity
    float finalOpacity = material.haveOpacityTexture
                        ? texture(material.opacityTexture, texCoords).r
                        : material.opacity;

    // Gamma correction
    vec3 gammaCorrected = pow(lighting, vec3(1.0 / gamma));
    color = vec4(gammaCorrected, finalOpacity);
}

/*****************************************************************************/
// Shadow calculation (Bias dinamico + PCF 5x5 + Soft Shadows)
/*****************************************************************************/
float shadowCalculation(vec4 fragPosLightSpace) {
    // Trasformo da clip space a texture space [0,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Se il frammento è fuori dallo shadow map range (es. dietro), niente ombra
    if(projCoords.z > 1.0)
        return 0.0;

    // Bias dinamico in base all'angolo tra normale e luce
    float bias = max(0.05 * (1.0 - dot(normalize(fragNormal), normalize(light.direction))), 0.005);

    // Texture size (risoluzione shadow map)
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    // Filtering: 5x5 campioni
    float shadow = 0.0;
    int samples = 0;
    int range = 2; // Raggio = 2 → 5x5 grid (da -2 a +2)

    for(int x = -range; x <= range; ++x) {
        for(int y = -range; y <= range; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (projCoords.z - bias) > pcfDepth ? 1.0 : 0.0;
            samples++;
        }
    }
    shadow /= float(samples); // Media

    return shadow;
}
