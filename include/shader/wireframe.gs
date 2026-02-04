#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 12) out;

uniform float lineWidth;
uniform vec2 viewport;
uniform vec3 color;

out vec3 fragColor;

void emitEdge(vec4 a, vec4 b) {
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

  fragColor = color;
  gl_Position = aPos;
  EmitVertex();

  fragColor = color;
  gl_Position = bPos;
  EmitVertex();

  fragColor = color;
  gl_Position = aNeg;
  EmitVertex();

  fragColor = color;
  gl_Position = bNeg;
  EmitVertex();

  EndPrimitive();
}

void main() {
  emitEdge(gl_in[0].gl_Position, gl_in[1].gl_Position);
  emitEdge(gl_in[1].gl_Position, gl_in[2].gl_Position);
  emitEdge(gl_in[2].gl_Position, gl_in[0].gl_Position);
}
