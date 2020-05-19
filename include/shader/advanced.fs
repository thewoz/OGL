#version 330 core

in vec4 frag_color;

out vec4 out_color;

void main() {
  
  if(frag_color.w<0.999) discard;
  
  out_color = frag_color;
  
}
