/*
 * GNU GENERAL PUBLIC LICENSE
 *
 * Copyright (C) 2017-2026
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


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>
#include <algorithm>

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

    glLines(glLines &&) noexcept = default;
    glLines & operator = (glLines &&) noexcept = default;
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec4 & _color = glm::vec4(0.0)) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initLine();
      
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
      
      shader.initLine();
      
      vertices = _vertices;

      colors = _color;

      // the color buffer must cover every vertex, otherwise the GPU reads past
      // the end of the color VBO during rendering
      if(colors.size() < vertices.size())
        colors.resize(vertices.size(), colors.empty() ? glm::vec4(1.0f) : colors.back());

      isInited = true;

    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, int from = 0, int to = -1, int strip = -1, int stripOffset = -1, int index = -1) {
      
      DEBUG_LOG("glLines::render(" + name + ")");
          
      if(!isInited){
        fprintf(stderr, "ERROR [glLines]: must be initialized before rendering\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection",   camera.getProjection());
      shader.setUniform("view",         camera.getView());
      shader.setUniform("model",        modelMatrix);
      shader.setUniform("lineWidth",    lineWidth);
      shader.setUniform("viewport",     camera.getViewport());
      shader.setUniform("uniformColor", glm::vec4(1.0f));
                        
      glBindVertexArray(vao);

      glDisable(GL_CULL_FACE);

      int n = (int) vertices.size();

      if(strip == -1) {

        // Single strip: 'to' is an end index (exclusive); -1 means "up to the
        // last vertex". Clamp so bad arguments cannot read past the buffer.
        if(to == -1 || to > n) to = n;
        if(from < 0) from = 0;

        if(to - from > 1) glDrawArrays(GL_LINE_STRIP, from, to - from);

      } else {

        // Multiple strips: strip i starts at from + i*stripOffset and 'to' is
        // the vertex count of each strip (-1 = up to the end of the buffer).
        // 'index' (when != -1) draws only that strip. Each strip is clamped to
        // the end of the buffer.
        for(int i=0; i<=strip; ++i) {
          if(index != -1 && i != index) continue;
          int first = (i * stripOffset) + from;
          if(first < 0 || first >= n) continue;
          int count = (to == -1) ? n - first : std::min(to, n - first);
          if(count > 1) glDrawArrays(GL_LINE_STRIP, first, count);
        }

      }

      glBindVertexArray(0);
      
      glCheckError();
      
    }
    
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {
      
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
    void cleanInGpu() override {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(2, vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;
        
      }
      
    }
    
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_LINES_H_ */
