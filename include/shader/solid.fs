#version 330 core

struct Light {
    vec3 direction;
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 color;
uniform Light light;

in vec3 fragPos;
in vec3 fragNormal;
out vec4 outColor;

void main() {
    vec3 norm = normalize(fragNormal);
    // fragPos is in view space (camera at origin), so -fragPos points to the eye.
    vec3 viewDir = normalize(-fragPos);

    float dirLen = length(light.direction);
    float posLen = length(light.position);
    bool hasLight = dirLen > 0.001 || posLen > 0.001;

    vec3 ambient = light.ambient * color;
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);

    if(hasLight) {
        vec3 lightDir = vec3(0.0);
        if(dirLen > 0.001) {
            lightDir = normalize(-light.direction);
        } else {
            lightDir = normalize(light.position - fragPos);
        }

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        diffuse = light.diffuse * diff * color;
        specular = light.specular * spec;
    }

    outColor = vec4(ambient + diffuse + specular, 1.0);
}
