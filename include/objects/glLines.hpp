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
    
    GLuint vao = -1;
    GLuint vbo[2];
    
    float lineWidth = 1;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
        
  public:
    
    //****************************************************************************/
    // glLines()
    //****************************************************************************/
    glLines(const std::string & _name = "") { name = _name; }
    glLines(const std::vector<glm::vec3> & _vertices, const glm::vec4 & _color = glm::vec4(0.0), float _lineWidth = 1, const std::string & _name = "") {
      name = _name;
      init(_vertices, _color, _lineWidth);
    }
    glLines(const std::vector<glm::vec3> & _vertices, const std::vector<glm::vec4> & _color, float _lineWidth = 1, const std::string & _name = "") {
      name = _name;
      init(_vertices, _color, _lineWidth);
    }
    
    
    //****************************************************************************/
    // ~glLines()
    //****************************************************************************/
    ~glLines() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec4 & _color = glm::vec4(0.0), float _lineWidth = 1) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initAdvanced();
      
      vertices = _vertices;
      
      lineWidth = _lineWidth;
      
      colors.resize(vertices.size(), _color);
            
      isInited = true;
      
    }
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const std::vector<glm::vec4> & _color, float _lineWidth = 1) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initAdvanced();
      
      vertices = _vertices;
      
      lineWidth = _lineWidth;

      colors = _color;
            
      isInited = true;
      
    }
    
    //****************************************************************************/
    // setLineWidth
    //****************************************************************************/
    //void setLineWidth(float _lineWidth) { lineWidth = _lineWidth; }
  
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera, int from = 0, int to = -1, int strip = -1, int stripOffset = -1) {
      
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
      
      glEnable(GL_DEPTH_TEST);
                  
      // https://vitaliburkov.wordpress.com/2016/09/17/simple-and-fast-high-quality-antialiased-lines-with-opengl/
      //glLineWidth(lineWidth);
      
      glBindVertexArray(vao);
      
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      
      if(to == -1) to = (int) vertices.size();
      
      if(strip == -1) {
        
        glDrawArrays(GL_LINE_STRIP, from, to);

      } else {
        
        for(int i=0; i<=strip; ++i)
          glDrawArrays(GL_LINE_STRIP, (i*stripOffset)+from, to);

      }

      glBindVertexArray(0);
      
      glDisable(GL_DEPTH_TEST);

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
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        glBindVertexArray(0);
        
      }
      
      isInitedInGpu = true;
      
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

#endif /* _H_OGL_LINES_H_ */
