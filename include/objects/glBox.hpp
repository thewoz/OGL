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

#ifndef _H_OGL_BOX_H_
#define _H_OGL_BOX_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // Class glBox
  //****************************************************************************/
  class glBox: public glObject {
    
  private:
    
    GLuint vao;
    GLuint vbo[2];
    
    glm::vec3 color;
    
    constexpr static const GLfloat vertices[] = {
      -0.5,  0.5,  0.5,
       0.5,  0.5,  0.5,
       0.5, -0.5,  0.5,
      -0.5, -0.5,  0.5,
      -0.5,  0.5, -0.5,
       0.5,  0.5, -0.5,
       0.5, -0.5, -0.5,
      -0.5, -0.5, -0.5,
    };
    
  public:
        
    //****************************************************************************/
    // glBox()
    //****************************************************************************/
    glBox(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glBox()
    //****************************************************************************/
    glBox(const glm::vec3 & _scale, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_scale, _color);
    }
    
    //****************************************************************************/
    // ~glBox()
    //****************************************************************************/
    ~glBox() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const glm::vec3 & _scale, const glm::vec3 & _color = glm::vec3(1.0)) {

      DEBUG_LOG("glBox::init(" + name + ")");

      shader.setName(name);
      
      shader.initPlain();
      
      scale(_scale);

      style = glObject::STYLE::WIREFRAME;
      
      color = _color;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera) {
            
      DEBUG_LOG("glBox::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "glBox must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection",  camera->getProjection());
      shader.setUniform("view",        camera->getView());
      shader.setUniform("model",       modelMatrix);
      shader.setUniform("color",       color);

      glBindVertexArray(vao);
        
      glDisable(GL_CULL_FACE);

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
     
      glBindVertexArray(0);
      
      glCheckError();
            
    }
    
    //****************************************************************************/
    // getBoundingRadius()
    //****************************************************************************/
    float getBoundingRadius() const {
      
      glm::vec3 center = glm::vec3(modelMatrix * glm::vec4(0, 0, 0, 1));
      
      float radius = 0.0f;
      
      for(int i=0; i<8; ++i) {
        glm::vec3 v(vertices[i*3], vertices[i*3 + 1], vertices[i*3 + 2]);
        glm::vec3 worldV = glm::vec3(modelMatrix * glm::vec4(v, 1.0));
        float dist = glm::distance(worldV, center);
        if(dist > radius) radius = dist;
      }
      
      return radius;
      
    }
    
    private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glBox::setInGpu(" + name + ")");

      static const GLushort indices[] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        0, 4,
        7, 4,
        4, 5,
        5, 6,
        6, 7,
        1, 5,
        2, 6,
        3, 7
      };
      
      // Crea e bind del VAO
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      // Crea i VBO
      glGenBuffers(2, vbo);

      // VBO per i vertici
      glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      // VBO per gli indici
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

      // Unbind del VAO (buona pratica)
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      glCheckError();
      
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

#endif /* _H_OGL_BOX_H_ */


