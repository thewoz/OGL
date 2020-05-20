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

#ifndef _H_OGL_POINTS_H_
#define _H_OGL_POINTS_H_

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
  // Class glPoints
  /*****************************************************************************/
  class glPoints : public glObject {
    
  private:
        
    GLuint vao = -1;
    GLuint vbo[2];
    
    std::vector<glm::vec3> points;
    std::vector<glm::vec4> colors;

    float radius;
    
  public:
    
    /*****************************************************************************/
    // glPoints
    /*****************************************************************************/
    glPoints(const std::string & _name = "") : glObject(_name) { }
    glPoints(const std::vector<glm::vec3> & _points, const glm::vec4 & color = glm::vec4(0.0), float _radius = 1, const std::string & _name = "") : glObject(_name) { init(_points, color, _radius); }
    glPoints(const std::vector<glm::vec3> & _points, const std::vector<glm::vec4> & _color, float _radius = 1, const std::string & _name = "") : glObject(_name) { init(_points, _color, _radius); }

    
    /*****************************************************************************/
    // ~glPoints
    /*****************************************************************************/
    ~glPoints() { cleanInGpu(); }
 
    /* ****************************************************************************/
    // init
    /* ****************************************************************************/
    void init(const std::vector<glm::vec3> & _points, const glm::vec4 & color = glm::vec4(0.0), float _radius = 1) {
      
      DEBUG_LOG("glPoints::init(" + name + ")");
         
      glObject::initSphere();

      points = _points;

      colors.resize(points.size(), color);
              
      radius = _radius;

      isInited = true;
      
    }
   
    /* ****************************************************************************/
    // init
    /* ****************************************************************************/
    void init(const std::vector<glm::vec3> & _points, const std::vector<glm::vec4> & _color, float _radius = 1) {
      
      DEBUG_LOG("glPoints::init(" + name + ")");
      
      glObject::initSphere();
      
      points = _points;
      
      colors = _color;
      
      radius = _radius;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // setRadius
    //****************************************************************************/
    void setRadius(float _radius) { radius = _radius; }
    
    //****************************************************************************/
    // render
    //****************************************************************************/
    void render(const glm::mat4 & projection, const glm::mat4 & view, int from = 0, int to = -1) {
      
      if(to == -1) to = (int) points.size();
      
      DEBUG_LOG("glPoints::render(" + name + ")");

      glEnable(GL_PROGRAM_POINT_SIZE);

      glObject::renderBegin(projection, view);
      
      shader.setUniform("pointSize", radius);

      //glEnable(GL_BLEND);

      glBindVertexArray(vao);

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      
      glDrawArrays(GL_POINTS, from, to);

      //glDisable(GL_BLEND);

      glDisable(GL_PROGRAM_POINT_SIZE);

      glBindVertexArray(0);

      glObject::renderEnd();
      
    }
    
    
  private:
    
    /*****************************************************************************/
    // setInGpu
    /*****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glPoints::setInGpu(" + name + ")");

      if(!isInitedInGpu) {
              
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(2, vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
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

#endif /* _H_OGL_POINTS_H_ */