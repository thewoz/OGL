#version 330 core

layout (location = 0) in vec2 position;

// width and Height of the viewport
uniform vec2 viewport;

//out vec2 lineCenter;

void main() {
  
  // From pixels to 0-1
  vec2 coord = position / viewport;
  
  // Flip Y so that 0 is top
  coord.y = (1.0-coord.y);
  
  // Map to NDC -1,+1
  coord.xy = coord.xy * 2.0 - 1.0;
  
  // coordinata proiettiva del punto
  gl_Position = vec4(coord.xy, 0.0, 1.0);

  //lineCenter = 0.5 * (gl_Position.xy + vec2(1,1))*viewport;

}

