/*
 * MIT License
 *
 * Copyright © 2017
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _H_MPL_OPENGL_BOX_H_
#define _H_MPL_OPENGL_BOX_H_

#include <cstdio>
#include <cstdlib>

#include "glObject.hpp"

/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {
  
  /*****************************************************************************/
  // Class glBox
  /*****************************************************************************/
  class glBox: public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo[2];

  public:
        
    /*****************************************************************************/
    // glBox
    /*****************************************************************************/
    glBox(const std::string & _name = "") : glObject(_name) { }
    
    /*****************************************************************************/
    // glBox
    /*****************************************************************************/
    glBox(const glm::vec3 & scale, const glm::vec3 & _color = glm::vec3(0.0), const std::string & _name = "") : glObject(_name) { init(scale, _color); }
    
    /*****************************************************************************/
    // ~glBox
    /*****************************************************************************/
    ~glBox() { cleanInGpu(); }
    
    /*****************************************************************************/
    // init
    /*****************************************************************************/
    void init(const glm::vec3 & scale, const glm::vec3 & _color = glm::vec3(0.0)) {

      DEBUG_LOG("glBox::init(" + name + ")");

      glObject::initPlain(glm::vec3(0.0), glm::vec3(0.0), glm::vec3(scale));

      style = glObject::STYLE::WIREFRAME;
      
      color = _color;
      
      isInited = true;
      
    }
    
    /*****************************************************************************/
    // render
    /*****************************************************************************/
    void render(const glm::mat4 & projection, const glm::mat4 & view) {
            
      DEBUG_LOG("glBox::render(" + name + ")");

      glObject::renderBegin(projection, view);
      
      glDisable(GL_CULL_FACE); //NOTE: non sono sicuro che serva

      glBindVertexArray(vao);
        
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
     
      glBindVertexArray(0);
      
      glObject::renderEnd();

    }
    
    private:

    /*****************************************************************************/
    // setInGpu
    /*****************************************************************************/
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
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      
    }
    
  private:
    
    /* ****************************************************************************/
    // cleanInGpu() -
    /* ****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(2, vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_MPL_OPENGL_BOX_H_ */


