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

#ifndef _H_OGL_POINTS_H_
#define _H_OGL_POINTS_H_

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glPoints
  //****************************************************************************/
  class glPoints : public glObject {
    
  private:
        
    GLuint vao = -1;
    GLuint vbo[2];
    
    std::vector<glm::vec3> points;
    std::vector<glm::vec4> colors;

    float radius;
    
  public:
    
    //****************************************************************************/
    // glPoints()
    //****************************************************************************/
    glPoints(const std::string & _name = "") { name = _name; }
    glPoints(const std::vector<glm::vec3> & _points, const glm::vec4 & _color = glm::vec4(0.0), float _radius = 1, const std::string & _name = "") {
      name = _name;
      init(_points, _color, _radius);
    }
    glPoints(const std::vector<glm::vec3> & _points, const std::vector<glm::vec4> & _color, float _radius = 1, const std::string & _name = "") {
      name = _name;
      init(_points, _color, _radius);
    }

    //****************************************************************************/
    // ~glPoints()
    //****************************************************************************/
    ~glPoints() { cleanInGpu(); }
 
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _points, const glm::vec4 & _color = glm::vec4(0.0), float _radius = 1) {
      
      DEBUG_LOG("glPoints::init(" + name + ")");
         
      shader.setName(name);
      shader.initSphere();
      
      points = _points;

      colors.resize(points.size(), _color);
              
      radius = _radius;

      isInited = true;
      
    }
   
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _points, const std::vector<glm::vec4> & _color, float _radius = 1) {
      
      DEBUG_LOG("glPoints::init(" + name + ")");
      
      shader.setName(name);
      shader.initSphere();
            
      points = _points;
      
      colors = _color;
      
      radius = _radius;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // setRadius()
    //****************************************************************************/
    void setRadius(float _radius) { radius = _radius; }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera, int from = 0, int to = -1, int index = -1) {
            
      DEBUG_LOG("glPoints::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "glPoints must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("pointSize",  radius);
            
      if(to == -1) to = (int) points.size();
      
      if(index != -1) { from += index; to = 1; }
      
      glEnable(GL_PROGRAM_POINT_SIZE);

      glBindVertexArray(vao);
      
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      glDrawArrays(GL_POINTS, from, to);

      glDisable(GL_PROGRAM_POINT_SIZE);

      glBindVertexArray(0);
      
      glCheckError();

    }
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glPoints::setInGpu(" + name + ")");

      if(!isInitedInGpu) {
              
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(2, vbo);
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
        
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

#endif /* _H_OGL_POINTS_H_ */
