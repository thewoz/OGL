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
  vec2 dir = ndcB - ndcA;
  float len = length(dir);
  if(len < 1e-6) {
    dir = vec2(0.0, 1.0);
  } else {
    dir /= len;
  }
  vec2 normal = vec2(-dir.y, dir.x);
  vec2 offset = normal * (lineWidth / viewport);

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
