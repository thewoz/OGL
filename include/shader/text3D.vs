#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {

  //gl_Position = projection * vec4(vertex.xyz, 1.0);
  gl_Position = projection * view * model * vec4(vertex.xyz, 1.0f);

  if(vertex.w == 0) TexCoords = vec2(0,0);
  if(vertex.w == 1) TexCoords = vec2(0,1);
  if(vertex.w == 2) TexCoords = vec2(1,1);
  if(vertex.w == 3) TexCoords = vec2(1,0);
   
}
