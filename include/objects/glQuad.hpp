/*
 * GNU GENERAL PUBLIC LICENSE
 *
 * Copyright (C) 2019
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _H_OGL_GLQUAD_H_
#define _H_OGL_GLQUAD_H_

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glQuad
  //****************************************************************************/
  class glQuad : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo = -1;
    GLuint ebo = -1;

    float vertices[18] = {
       // first triangle
       0.5f,  0.5f, 0.0f,  // top right
       0.5f, -0.5f, 0.0f,  // bottom right
      -0.5f,  0.5f, 0.0f,  // top left
       // second triangle
       0.5f, -0.5f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f,  // bottom left
      -0.5f,  0.5f, 0.0f   // top left
    };

    glm::vec3 color = glm::vec3(1.0);
    
  public:
    
    //****************************************************************************/
    // glQuad - Constructor
    //****************************************************************************/
    glQuad(const std::string & _name = "") {
      
      name = _name;
      
      isInited = true;
      
      shader.setName(name);
      shader.initPlain();

    }
    
    //****************************************************************************/
    // ~glQuad
    //****************************************************************************/
    ~glQuad() { cleanInGpu(); }
    
    //****************************************************************************/
    // render
    //****************************************************************************/
    void render(const glCamera * camera) {
      
      DEBUG_LOG("glQuad::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glQuad must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);
      
      glEnable(GL_DEPTH_TEST);

      glDisable(GL_CULL_FACE);

      glBindVertexArray(vao);
      
      glEnableVertexAttribArray(0);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindVertexArray(0);

      glDisable(GL_DEPTH_TEST);
            
    }
    
  private:
        
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glQuad::setInGpu(" + name + ")");
      
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
      
      glEnableVertexAttribArray(0);

      glBindVertexArray(0);
      
    }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
        
      }

    }
    
  };
  
} /* namspace ogl */

#endif /* _H_OGL_GLQUAD_H_ */

