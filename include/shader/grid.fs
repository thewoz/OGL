#version 330 core

//
// Infinite floor grid, see grid.vs. The lines are not geometry: each fragment
// works out how far it is, in pixels, from the nearest line of the grid, using
// the screen space derivative of its world position, and turns that into an
// alpha. That gives lines of a constant width in pixels with a clean one pixel
// antialiasing at any distance and any viewing angle, which a mesh of lines
// cannot do.
//
// Two things keep the plane from looking wrong far away:
//   - when the cells get close to the size of a pixel the lines would alias into
//     moire, so they fade out instead (the grid thins out toward the horizon);
//   - the whole grid fades with the distance from the eye, so the far end of the
//     quad is never visible as an edge.
//

in vec3 worldPos;

uniform mat4  view;
uniform mat4  projection;
uniform float zNear;        // near plane of the camera, for the depth bias below

uniform vec3  eye;          // eye position in world space
uniform float cellSize;     // side of a cell, in world units
uniform float lineWidth;    // width of the lines, in pixels
uniform vec3  color;        // line color, sRGB
uniform float fadeStart;    // distance from the eye where the fade begins
uniform float fadeEnd;      // distance from the eye where the grid is gone

out vec4 outColor;

//
// srgbToLinear - exact sRGB transfer function. The framebuffer is sRGB and the
// GPU encodes every write (glWindow enables GL_FRAMEBUFFER_SRGB), so a color
// meant to appear as given has to be linearized first. The alpha blending then
// happens in linear space, like for the lit shaders.
//
vec3 srgbToLinear(vec3 c) {
    return mix(c / 12.92, pow((c + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), c));
}

void main() {

  // Depth: the grid is pushed a little away from the eye, so that a surface
  // lying on the floor (the bottom of a box, a target) always covers it. A fixed
  // polygon offset is not enough, for two reasons, and the push has a term for
  // each:
  //   - with a small zNear (the camera inside a large scene) the depth buffer
  //     loses precision, and the two coplanar surfaces differ by more than one
  //     or two steps just from interpolation. The first term is a fraction of
  //     the distance sized on the depth resolution there, which for a 24 bit
  //     perspective depth buffer is about distance / (zNear * 2^24) relative to
  //     the distance; 16 times that leaves room, and it is still millimetres at
  //     metres;
  //   - looking at the floor at a grazing angle its depth changes fast from a
  //     pixel to the next, and with multisampling the surface on the floor has a
  //     depth per sample while this shader writes one per pixel: a sample can be
  //     half a pixel away from the centre. The second term is the change of the
  //     depth across one pixel (fwidth), like the slope factor of a polygon
  //     offset.
  // NB: computed before any discard, because fwidth needs every pixel of the
  // quad to run it
  vec4  clipOnPlane = projection * view * vec4(worldPos, 1.0);
  float slope = fwidth(0.5 * clipOnPlane.z / clipOnPlane.w);

  float dist = distance(worldPos, eye);
  float bias = max(1e-5, 16.0 * dist / (zNear * 16777216.0));

  vec4  clipPushed = projection * view * vec4(eye + (worldPos - eye) * (1.0 + bias), 1.0);
  float depth = 0.5 * clipPushed.z / clipPushed.w + 0.5 + slope;

  // position in cells, and how many cells one pixel covers along each axis
  vec2 coord = worldPos.xz / cellSize;
  vec2 cellsPerPixel = max(fwidth(coord), vec2(1e-6));

  // distance in pixels to the nearest line of each family (0 on a line)
  vec2 toLine = abs(fract(coord - 0.5) - 0.5) / cellsPerPixel;

  float nearest = min(toLine.x, toLine.y);

  // a line of 'lineWidth' pixels with a one pixel antialiased edge
  float alpha = clamp(0.5 * lineWidth + 0.5 - nearest, 0.0, 1.0);

  // cells smaller than a few pixels: fade the lines out instead of aliasing
  float density = max(cellsPerPixel.x, cellsPerPixel.y);

  alpha *= 1.0 - smoothstep(0.2, 0.5, density);

  // distance fade
  alpha *= 1.0 - smoothstep(fadeStart, fadeEnd, distance(worldPos, eye));

  if(alpha <= 0.002) discard;

  gl_FragDepth = clamp(depth, 0.0, 1.0);

  outColor = vec4(srgbToLinear(color), alpha);

}
