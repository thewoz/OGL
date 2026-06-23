#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float pointSize;
uniform vec2  viewport;   // framebuffer size in pixels, to size the impostor in view space

out vec4  fragColor;
out vec3  fragPosView;    // point centre in view space, for lighting in the fragment
out float fragRadiusView; // sphere radius in view space, for the per-fragment depth

void main() {

  vec4 posView = view * model * vec4(position, 1.0f);

  gl_Position = projection * posView;

  // Scale the sprite by distance so points keep a consistent apparent size.
  float Z = length(posView.xyz);
  float pixelDiameter = pointSize / Z;
  gl_PointSize = pixelDiameter;

  // View-space radius of the sphere that projects to 'pixelDiameter' pixels, so
  // the fragment shader can write the real spherical surface depth. Inverting the
  // perspective vertical scale: pixels = projection[1][1] * (2R)/w * viewport.y/2.
  float w = max(-posView.z, 1e-6);
  fragRadiusView = pixelDiameter * w / (projection[1][1] * viewport.y);

  fragColor   = color;
  fragPosView = posView.xyz;

}
