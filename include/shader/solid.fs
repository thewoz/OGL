#version 330 core

//
// Simple Phong shading for the solid base objects (sphere, ellipse, cuboid, ...).
// Everything is computed in view space, where the camera sits at the origin.
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

in  vec3 fragPos;      // fragment position in view space
in  vec3 fragNormal;   // fragment normal   in view space
out vec4 outColor;

const float shininess = 32.0;

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

    outColor = vec4(ambient + diffuse + specular, 1.0);
}
