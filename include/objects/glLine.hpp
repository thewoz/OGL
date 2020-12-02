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

#ifndef _H_OGL_LINE_H_
#define _H_OGL_LINE_H_

#include <cstdlib>
#include <cstdio>


//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // Class glLine
  //****************************************************************************/
  class glLine : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo;

    glm::vec3 color;
    
    std::vector<glm::vec3> vertices;
    
  public:
    
    //****************************************************************************/
    // glLine()
    //****************************************************************************/
    glLine(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glLine()
    //****************************************************************************/
    glLine(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name  = _name;
      color = _color;
      init(_vertices, _color);
    }
    
    //****************************************************************************/
    // ~glLine()
    //****************************************************************************/
    ~glLine() { cleanInGpu(); }
    
    //****************************************************************************/
    // setColor()
    //****************************************************************************/
    void setColor(const glm::vec3 & _color) { color = _color; }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0)) {
 
      DEBUG_LOG("glLine::init(" + name + ")");

      shader.setName(name);
     
      shader.initPlain();
      
      color = _color;
      
      vertices = _vertices;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // _render()
    //****************************************************************************/
    void render(const glCamera * camera) {
      
      DEBUG_LOG("glLine::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glLine must be inited before render\n");
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

      glEnableVertexAttribArray(0);
      
      glDrawArrays(GL_LINE_STRIP, 0, (GLuint)vertices.size());
      
      glDisableVertexAttribArray(0);
      
      glBindVertexArray(0);
            
      glDisable(GL_DEPTH_TEST);

    }
    
    //****************************************************************************/
    // setInGpu
    //****************************************************************************/
    void setInGpu()  {
      
      DEBUG_LOG("glLine::setInGpu(" + name + ")");

      if(isToUpdateInGpu && vao != -1) {

        DEBUG_LOG("glLine::updateInGpu(" + name + ")");
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(glm::vec3), glm::value_ptr(vertices[0]));
                
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        isToUpdateInGpu = false;
        
      } else {
        
        DEBUG_LOG("glLine::setInGpu(" + name + ")");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        //glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        glBindVertexArray(0);
              
        isInitedInGpu = true;

      }
          
    }
    
    //****************************************************************************/
    // update() -
    //****************************************************************************/
    void update(const std::vector<glm::vec3> & _vertices) {
      
      DEBUG_LOG("glLine::update(" + name + ")");

      if(!isInited) init(_vertices, color);
      else vertices = _vertices;

      isToUpdateInGpu = true;
      isInitedInGpu   = false;
      
    }
    
  private:
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      DEBUG_LOG("glLine::cleanInGpu(" + name + ")");

      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
              
        isInitedInGpu = false;

      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_LINE_H_ */
