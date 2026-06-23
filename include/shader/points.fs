#version 330 core

//
// Point-sprite sphere impostors: every GL_POINTS vertex is shaded as if it were
// a little 3D sphere, reconstructing a view-space normal from gl_PointCoord.
// Lighting uses the same model as solid.fs (scene light + head-light fallback),
// so glPoints now reacts to setLight() instead of a hard-coded light.
//

struct Light {
    vec3 direction;  // directional light (view space); zero means "not used"
    vec3 position;   // point light position (view space)
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform mat4  projection;   // to turn the spherical surface point into a depth value

// Shading mode for the impostors:
//   0 = FLAT    : uniform matte colour, no lighting, view independent
//                 (the assigned colour is shown exactly, no gamma)
//   1 = DIFFUSE : matte sphere (ambient + Lambert diffuse, no specular)
//   2 = PHONG   : full Phong with specular highlight (shiny, the old default)
uniform int shadingMode;

in vec4  fragColor;
in vec3  fragPosView;
in float fragRadiusView;

out vec4 outColor;

const float shininess = 32.0;
const float gamma     = 2.2;

void main() {

  // Hide-by-alpha: a vertex colour with alpha < 1 means "don't draw this point".
  if(fragColor.w < 0.999) discard;

  // Reconstruct the sphere impostor: drop fragments outside the unit disc.
  vec2 cxy = 2.0 * gl_PointCoord - 1.0;
  float r2 = dot(cxy, cxy);
  if(r2 > 1.0) discard;

  // Height of the sphere cap above the disc plane (1 at centre, 0 at the rim).
  float zCap = sqrt(1.0 - r2);

  // Write the real spherical surface depth instead of the flat point-centre
  // depth, so overlapping points occlude each other like true 3D spheres and
  // their order stays stable while the camera moves (no popping front/back).
  vec3 surfView = vec3(fragPosView.xy, fragPosView.z + fragRadiusView * zCap);
  vec4 surfClip = projection * vec4(surfView, 1.0);
  gl_FragDepth = 0.5 + 0.5 * (surfClip.z / surfClip.w);

  vec3 outRGB;

  if(shadingMode == 0) {

    // FLAT: uniform matte colour, view independent. No lighting and no gamma so
    // the colour assigned to the point is reproduced exactly.
    outRGB = fragColor.rgb;

  } else {

    // Turn the disc into a view-space normal (z points toward the camera).
    vec3 norm = normalize(vec3(cxy.x, cxy.y, zCap));

    // In view space the eye is at the origin, so -fragPos points to the camera.
    vec3 viewDir = normalize(-fragPosView);

    // Choose the light direction: explicit direction, else point light,
    // else a head light coming straight from the camera.
    vec3 lightDir;
    if(length(light.direction) > 0.001)      lightDir = normalize(-light.direction);
    else if(length(light.position) > 0.001)  lightDir = normalize(light.position - fragPosView);
    else                                     lightDir = viewDir;

    float diff = max(dot(norm, lightDir), 0.0);

    vec3 ambient = light.ambient * fragColor.rgb;
    vec3 diffuse = light.diffuse * diff * fragColor.rgb;

    vec3 lighting = ambient + diffuse;

    // PHONG: add the specular highlight on top of the matte base.
    if(shadingMode == 2) {
      vec3  reflectDir = reflect(-lightDir, norm);
      float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
      lighting += light.specular * spec;
    }

    // Gamma-correct the final color for display (consistent with solid/model).
    outRGB = pow(lighting, vec3(1.0 / gamma));

  }

  // Antialias the disc edge.
  float delta = fwidth(r2);
  float alpha = 1.0 - smoothstep(1.0 - delta, 1.0 + delta, r2);

  outColor = vec4(outRGB, 1.0) * alpha;

}
