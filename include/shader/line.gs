#version 330 core

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

uniform float lineWidth;
uniform vec2 viewport;

in vec4 vertColor[];
out vec4 fragColor;

void emitLineQuad(vec4 a, vec4 b, vec4 colorA, vec4 colorB) {
  vec2 ndcA = a.xy / a.w;
  vec2 ndcB = b.xy / b.w;
  // Work in pixel space: NDC is anisotropic (x and y span [-1,1] regardless of
  // the window aspect), so a perpendicular computed there yields a thickness
  // that changes with the line orientation and the aspect ratio. In pixels the
  // perpendicular offset is truly isotropic. lineWidth is the full thickness
  // in pixels (half on each side), matching the previous axis-aligned result.
  vec2 halfViewport = viewport * 0.5;
  vec2 dir = (ndcB - ndcA) * halfViewport;
  float len = length(dir);
  if(len < 1e-6) {
    dir = vec2(0.0, 1.0);
  } else {
    dir /= len;
  }
  vec2 normal = vec2(-dir.y, dir.x);
  vec2 offset = (normal * (lineWidth * 0.5)) / halfViewport;

  vec4 aPos = vec4((ndcA + offset) * a.w, a.z, a.w);
  vec4 aNeg = vec4((ndcA - offset) * a.w, a.z, a.w);
  vec4 bPos = vec4((ndcB + offset) * b.w, b.z, b.w);
  vec4 bNeg = vec4((ndcB - offset) * b.w, b.z, b.w);

  fragColor = colorA;
  gl_Position = aPos;
  EmitVertex();

  fragColor = colorB;
  gl_Position = bPos;
  EmitVertex();

  fragColor = colorA;
  gl_Position = aNeg;
  EmitVertex();

  fragColor = colorB;
  gl_Position = bNeg;
  EmitVertex();

  EndPrimitive();
}

void main() {
  emitLineQuad(gl_in[0].gl_Position, gl_in[1].gl_Position, vertColor[0], vertColor[1]);
}
