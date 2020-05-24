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

#ifndef _H_OGL_SMALL_AXES_H_
#define _H_OGL_SMALL_AXES_H_

#include <cstdlib>
#include <cstdio>

//#include <ogl/core/glObject.hpp>


/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {

  /*****************************************************************************/
  // Class glSmallAxes
  /*****************************************************************************/
  class glSmallAxes : public glObject {
    
    private:
      
      GLuint vao[3];
      GLuint vbo[3];
      
      glm::vec3 colors[3] = { glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f) };
      
    public:
        
      /*****************************************************************************/
      // glSmallAxes() -
      /*****************************************************************************/
      glSmallAxes(GLfloat scale = 1.0, const std::string & _name = " ") : glObject(_name) {
        
        glObject::initOrto();
        
        init(scale);
        
      }
      
      /*****************************************************************************/
      // ~glSmallAxes() -
      /*****************************************************************************/
      ~glSmallAxes() { cleanInGpu(); }
      
    
    /*****************************************************************************/
    // init() -
    /*****************************************************************************/
    void init(GLfloat scale = 1.0) {
      
      DEBUG_LOG("glSmallAxes::init(" + name + ")");

      glObject::scale(glm::vec3(scale));

      isInited = true;
      
    }
    
    /*****************************************************************************/
    // render() -
    /*****************************************************************************/
    //void render(const glm::mat4 & projection, float yaw, float pitch, const glm::vec3 & position) {
    void render(const glm::mat4 & projection, const glm::mat4 & view) {

      DEBUG_LOG("glSmallAxes::render(" + name + ")");
      
      GLint data[4];
      glGetIntegerv(GL_VIEWPORT, data);
//
//      //printf("%d %d %d %d\n\n\n", data[0], data[1], data[2], data[3]);
//
//      glm::vec3 _front;
//
//      _front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//      _front.y = sin(glm::radians(pitch));
//      _front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//
//      glm::vec3 worldUp = glm::vec3(0.0f, 1.0f,  0.0f);
//
//      glm::vec3 front = glm::normalize(_front);
//
//      //printf("%f %f %f\n", front.x, front.y, front.z);
//
//      // Also re-calculate the Right and Up vector
//      glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
//
//      //printf("%f %f %f\n", right.x, right.y, right.z);
//
//      // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//      glm::vec3 up = glm::normalize(glm::cross(right, front));
//
////      printf("%f %f %f\n\n\n", up.x, up.y, up.z);
//
//      glm::mat4 view = glm::lookAt(position, glm::vec3(0), up);
      
      glObject::renderBegin(projection, view);
      
      //glOrtho(-1, 1, -1, 1, 1, -1);
      
      glViewport(data[2] - 160, 10, 160, 160);
      
      //glEnable(GL_LINE_SMOOTH);
      
      //glShadeModel(GL_SMOOTH);
      
      //glLineWidth(2);

      for(size_t i=0; i<3; ++i) {
        
        shader.setUniform("color", colors[i]);
        
        glBindVertexArray(vao[i]);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // AGGIUNTA
        
        glEnableVertexAttribArray(0);
        
        //glDrawArrays(GL_LINE_STRIP, 0, 2);
        glDrawArrays(GL_LINES, 0, 2); // AGGIUNTA
        
        glBindVertexArray(0);
        
      }
      
      //glDisable(GL_LINE_SMOOTH);
      
      glViewport(data[0], data[1], data[2], data[3]);

      glObject::renderEnd();
      
    }
    
      
    private:
      
      /*****************************************************************************/
      // setInGpu() -
      /*****************************************************************************/
      void setInGpu() {
        
        DEBUG_LOG("glSmallAxes::setInGpu(" + name + ")");
        
        std::vector<std::vector<glm::vec3>> vertices(3, std::vector<glm::vec3>(2, glm::vec3(0.0f)));
        
        vertices[0][1] = glm::vec3(1.0f,0.0f,0.0f);
        vertices[1][1] = glm::vec3(0.0f,1.0f,0.0f);
        vertices[2][1] = glm::vec3(0.0f,0.0f,1.0f);
        
        for(size_t i=0; i<3; ++i) {
          
          glGenVertexArrays(1, &vao[i]);
          glBindVertexArray(vao[i]);
          
          glGenBuffers(1, &vbo[i]);
          glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
          glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), &vertices[i][0], GL_STATIC_DRAW);
          
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
          
        }
        
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        glBindVertexArray(0);
        
      }
    
  private:
    
    /* ****************************************************************************/
    // cleanInGpu() -
    /* ****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(3, vbo);
        glDeleteVertexArrays(3, vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_SMALL_AXES_H_ */
