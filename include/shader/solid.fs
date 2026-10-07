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

uniform float shininess;         // specular exponent
uniform float specularStrength;  // specular intensity, 0 disables the highlight
uniform bool  unlit;             // no lighting: the color as given, see glShape::setUnlit()

//
// srgbToLinear - exact sRGB electro-optical transfer function (the piecewise
// one, with the linear segment near black; not the pow 2.2 approximation).
// Colors coming in as uniforms are authored in sRGB and must be linearized
// before any lighting math. The re-encoding on the way out is NOT done here:
// the default framebuffer is sRGB and the GPU encodes the write (glWindow
// enables GL_FRAMEBUFFER_SRGB), so using the exact curve here makes the
// round trip exact for the colors that are written through unchanged.
//
vec3 srgbToLinear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

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

    // Unlit: the surface shows its own color exactly, whatever the light and
    // the point of view (a printed pattern that must read as true white and
    // black). Linearized like the lit path, the framebuffer encodes it back
    if(unlit) {
        outColor = vec4(srgbToLinear(color), 1.0);
        return;
    }

    vec3 norm = normalize(fragNormal);

    // In view space the eye is at the origin, so -fragPos points to the camera.
    vec3 viewDir = normalize(-fragPos);

    // Two-sided shading. On a surface seen from its back the normal points away
    // from the eye, so the diffuse and specular terms collapse to zero and the
    // face comes out flat ambient: flipping the normal shades that side
    // properly. This matters for the objects drawn with back face culling off
    // (glSheet, and glQuad with setCullFaceEnabled(false)), which are the only
    // ones whose far side is ever rasterized.
    //
    // The test is on the view direction and NOT on gl_FrontFacing, which is
    // unreliable here: on this driver a shader reading it twice gets two
    // different answers, and using it flipped the normals of the culled closed
    // objects too (the sphere came out visibly darker). dot(norm, viewDir) is
    // the geometric condition we actually care about and does not depend on the
    // winding of the triangles.
    if(dot(norm, viewDir) < 0.0) norm = -norm;

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

    // The base color arrives as an sRGB-authored value and the light
    // intensities are already linear, so only the former is converted.
    vec3 base = srgbToLinear(color);

    vec3 ambient  = light.ambient  * base;
    vec3 diffuse  = light.diffuse  * diff * base;

    // The highlight is NOT tinted by the base color: on a dielectric (paper,
    // cardboard, painted surfaces) the specular reflection does not take the
    // body color. Metals do tint it, and those belong to model.fs, which has a
    // real specular color in the material.
    vec3 specular = light.specular * spec * specularStrength;

    // Shadow only attenuates the direct (diffuse + specular) terms.
    float shadow = useShadow ? shadowCalc(norm, lightDir) : 0.0;

    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    // Linear out: GL_FRAMEBUFFER_SRGB is on, the GPU does the sRGB encoding.
    outColor = vec4(lighting, 1.0);
}
