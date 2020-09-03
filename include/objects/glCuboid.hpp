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

#ifndef _H_OGL_CUBOID_H_
#define _H_OGL_CUBOID_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // Class glCuboid
  //****************************************************************************/
  class glCuboid : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo[2];
    
    glm::vec3 color;

  public:
        
    //****************************************************************************/
    // glCuboid()
    //****************************************************************************/
    glCuboid(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glCuboid()
    //****************************************************************************/
    glCuboid(const glm::vec3 & _scale, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_scale, _style, _color);
    }
    
    //****************************************************************************/
    // ~glCuboid()
    //****************************************************************************/
    ~glCuboid() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const glm::vec3 & _scale, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0)) {

      DEBUG_LOG("glCuboid::init(" + name + ")");

      shader.setName(name);
      
      shader.initPlain();
      
      scale(_scale);
      
      style = _style;
      
      color = _color;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera) {
            
      DEBUG_LOG("glCuboid::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "glCuboid must be inited before render\n");
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
      
      if(style == glObject::STYLE::WIREFRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      if(style == glObject::STYLE::SOLID)     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, nullptr);
            
      glDisableVertexAttribArray(0);

      glBindVertexArray(0);
      
      glDisable(GL_DEPTH_TEST);

    }
    
    private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glCuboid::setInGpu(" + name + ")");
      
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
        0, 1, 3,
        1, 2, 3,
        2, 3, 7,
        2, 7, 6,
        1, 5, 2,
        2, 5, 6,
        0, 7, 3,
        0, 4, 7,
        5, 7, 6,
        5, 4, 7,
        0, 4, 1,
        4, 5, 2
      };
      
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(2, vbo);

      glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      //glEnableVertexAttribArray(0);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      
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

#endif /* _H_OGL_CUBOID_H_ */


