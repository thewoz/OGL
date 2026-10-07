#version 330 core

//
// Infinite floor grid: one big horizontal quad, built here from gl_VertexID so
// the draw needs no vertex buffer. The quad is a square of half side 'halfSize'
// centred (in X and Z) on 'gridCenter', which glInfiniteGrid sets to the eye
// position: it follows the camera, so the plane never has an edge to reach. The
// grid pattern itself is not tied to the quad, it is computed in world space by
// the fragment shader, so moving the quad around is invisible.
//

uniform mat4 view;
uniform mat4 projection;

uniform vec3  gridCenter;   // X and Z of the quad centre, Y of the floor
uniform float halfSize;     // half side of the quad, in world units

out vec3 worldPos;

// two triangles covering the square
const vec2 corners[6] = vec2[6](
  vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0),
  vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)
);

void main() {

  vec2 corner = corners[gl_VertexID];

  worldPos = vec3(gridCenter.x + corner.x * halfSize,
                  gridCenter.y,
                  gridCenter.z + corner.y * halfSize);

  gl_Position = projection * view * vec4(worldPos, 1.0);

}
