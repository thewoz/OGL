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

#ifndef _H_OGL_BOX_H_
#define _H_OGL_BOX_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // Class glBox
  //****************************************************************************/
  class glBox: public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo[2];
    
    glm::vec3 color;
    
  public:
        
    //****************************************************************************/
    // glBox()
    //****************************************************************************/
    glBox(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glBox()
    //****************************************************************************/
    glBox(const glm::vec3 & _scale, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_scale, _color);
    }
    
    //****************************************************************************/
    // ~glBox()
    //****************************************************************************/
    ~glBox() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const glm::vec3 & _scale, const glm::vec3 & _color = glm::vec3(1.0)) {

      DEBUG_LOG("glBox::init(" + name + ")");

      shader.setName(name);
      
      shader.initPlain();
      
      scale(_scale);

      style = glObject::STYLE::WIREFRAME;
      
      color = _color;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera) {
            
      DEBUG_LOG("glBox::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "glBox must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);
      
      glEnable(GL_DEPTH_TEST);
            
      glBindVertexArray(vao);
        
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
     
      glBindVertexArray(0);
      
      glDisable(GL_DEPTH_TEST);

    }
    
    private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glBox::setInGpu(" + name + ")");
      
      static const GLfloat vertices[] = {
        // front
        -1.0,  1.0,  1.0,
         1.0,  1.0,  1.0,
         1.0, -1.0,  1.0,
        -1.0, -1.0,  1.0,
        // back
        -1.0,  1.0,  -1.0,
        1.0,  1.0,  -1.0,
        1.0, -1.0,  -1.0,
        -1.0, -1.0, -1.0,
      };
      
      static const GLushort indicies[] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        0, 4,
        7, 4,
        4, 5,
        5, 6,
        6, 7,
        1, 5,
        2, 6,
        3, 7
      };
      
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(2, vbo);

      glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
      
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);
      
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

      glBindVertexArray(0);
//      glBindBuffer(GL_ARRAY_BUFFER, 0);
//      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      
    }
    
  private:
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(2, vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_BOX_H_ */


