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

#ifndef _H_OGL_LINES_H_
#define _H_OGL_LINES_H_

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>

//#include <opencv2/opencv.hpp>

//#include <ogl/core/glObject.hpp>

/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {
  
  /*****************************************************************************/
  // Class glLines
  /*****************************************************************************/
  class glLines : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo[2];
    
    float lineWidth = 1;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec4> colors;
        
  public:
    
    //****************************************************************************/
    // glLines
    //****************************************************************************/
    glLines(const std::string & _name = "") : glObject(_name) { }
    glLines(const std::vector<glm::vec3> & _vertices, const glm::vec4 & color = glm::vec4(0.0), float _lineWidth = 1, const std::string & _name = "") : glObject(_name) { init(_vertices, color, _lineWidth); }
    glLines(const std::vector<glm::vec3> & _vertices, const std::vector<glm::vec4> & _color, float _lineWidth = 1, const std::string & _name = "") : glObject(_name) { init(_vertices, _color, _lineWidth); }
    
    
    //****************************************************************************/
    // ~glLines
    //****************************************************************************/
    ~glLines() { cleanInGpu(); }
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec4 & color = glm::vec4(0.0), float _lineWidth = 1) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      glObject::initAdvanced();
      
      vertices = _vertices;
      
      lineWidth = _lineWidth;
      
      colors.resize(vertices.size(), color);
            
      isInited = true;
      
    }
    
    /* ****************************************************************************/
    // init
    /* ****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const std::vector<glm::vec4> & _color, float _lineWidth = 1) {
      
      DEBUG_LOG("glLines::init(" + name + ")");
      
      glObject::initAdvanced();
      
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
    // render
    //****************************************************************************/
    void render(const glm::mat4 & projection, const glm::mat4 & view, int from = 0, int to = -1, int strip = -1, int stripOffset = -1) {
      
      if(to == -1) to = (int) vertices.size();
      
      DEBUG_LOG("glLines::render(" + name + ")");
          
      glObject::renderBegin(projection, view);
                  
      //glEnable(GL_BLEND);
      
      glEnable(GL_LINE_SMOOTH);
      
      glShadeModel(GL_SMOOTH);
      
      // https://vitaliburkov.wordpress.com/2016/09/17/simple-and-fast-high-quality-antialiased-lines-with-opengl/
      //glLineWidth(lineWidth);
      
      glBindVertexArray(vao);
      
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      
      if(strip == -1) {
        
        glDrawArrays(GL_LINE_STRIP, from, to);

      } else {
        
        for(int i=0; i<=strip; ++i)
          glDrawArrays(GL_LINE_STRIP, (i*stripOffset)+from, to);

      }


      //glDisable(GL_BLEND);
            
      glDisable(GL_LINE_SMOOTH);

      glBindVertexArray(0);
      
      glObject::renderEnd();
      
    }
    
    
  private:
    
    //****************************************************************************/
    // setInGpu
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

#endif /* _H_OGL_LINES_H_ */
