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

#ifndef _H_OGL_LINES_H_
#define _H_OGL_LINES_H_

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // Class glLines
  //****************************************************************************/
  class glLines : public glObject {
    
  private:
    
    GLuint vao;
    GLuint vbo[2];
        
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
        
  public:
    
    //****************************************************************************/
    // glLines()
    //****************************************************************************/
    glLines(const std::string & _name = "") { name = _name; }
    glLines(const std::vector<glm::vec3> & _vertices, const glm::vec4 & _color = glm::vec4(0.0), const std::string & _name = "") {
      name = _name;
      init(_vertices, _color);
    }
    glLines(const std::vector<glm::vec3> & _vertices, const std::vector<glm::vec4> & _color, const std::string & _name = "") {
      name = _name;
      init(_vertices, _color);
    }
    
    
    //****************************************************************************/
    // ~glLines()
    //****************************************************************************/
    ~glLines() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec4 & _color = glm::vec4(0.0)) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initAdvanced();
      
      vertices = _vertices;
            
      colors.resize(vertices.size(), _color);
            
      isInited = true;
      
    }
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const std::vector<glm::vec4> & _color) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initAdvanced();
      
      vertices = _vertices;
      
      colors = _color;
            
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera, int from = 0, int to = -1, int strip = -1, int stripOffset = -1, int index = -1) {
      
      DEBUG_LOG("glLines::render(" + name + ")");
          
      if(!isInited){
        fprintf(stderr, "glLines must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
                        
      // https://vitaliburkov.wordpress.com/2016/09/17/simple-and-fast-high-quality-antialiased-lines-with-opengl/
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      glBindVertexArray(vao);
      
      glDisable(GL_CULL_FACE);

      if(to == -1) to = (int) vertices.size();
      
      if(strip == -1) {
        
        glDrawArrays(GL_LINE_STRIP, from, to - from);

      } else {
        
        for(int i=0; i<=strip; ++i) {
          if(index != -1 && i != index) continue;
          glDrawArrays(GL_LINE_STRIP, (i*stripOffset)+from, to);
        }
        
      }

      glBindVertexArray(0);
      
      glCheckError();
      
    }
    
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glLines::setInGpu(" + name + ")");
      
      if(!isInitedInGpu) {
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(2, vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
     
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
        
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        glCheckError();
        
      }
      
      isInitedInGpu = true;
      
    }
    
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

#endif /* _H_OGL_LINES_H_ */
