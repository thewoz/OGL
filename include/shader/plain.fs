#version 330 core

uniform vec3 color;

uniform float lineWidth;
uniform float blendFactor;

in vec2 lineCenter;

out vec4 outColor;

void main() {

  float dist = length(lineCenter-gl_PointCoord.xy);

  if(dist > lineWidth) discard;
  else outColor = vec4(color, 1.0) * pow(float((lineWidth-dist)/lineWidth), blendFactor);
 
  //outColor = vec4(color, 1.0);

}
