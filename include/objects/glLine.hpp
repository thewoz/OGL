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
