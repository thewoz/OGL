#version 330 core

layout (location = 0) in vec3 position;

// uniform variable
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform mat4 mvp;

void main() {
  
  mat4 tmp = projection * view;// * model;
  
  mat4 new = transpose(tmp);
  
  new[3][0] = position.x;
  new[3][1] = position.y;
  new[3][2] = position.z;
  new[3][3] = 1.0;

  new[0][3] = 0.0;
  new[1][3] = 0.0;
  new[2][3] = 0.0;

  gl_Position = new * vec4(position, 1.0f);

  
  // coordinata proiettiva del punto
  //gl_Position = projection * view * model * vec4(position, 1.0f);

}

