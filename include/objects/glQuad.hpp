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

#ifndef _H_OGL_GLQUAD_H_
#define _H_OGL_GLQUAD_H_

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glQuad
  //****************************************************************************/
  class glQuad : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo = -1;
    GLuint ebo = -1;

    std::vector<glm::vec3> vertices;

    glm::vec3 color;
    
  public:
    
    //****************************************************************************/
    // glQuad - Constructor
    //****************************************************************************/
    glQuad(const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      std::vector<glm::vec3> _vertices {// first triangle
        glm::vec3( 0.5f,  0.5f, 0.0f),  // top right
        glm::vec3( 0.5f, -0.5f, 0.0f),  // bottom right
        glm::vec3(-0.5f,  0.5f, 0.0f),  // top left
                                        // second triangle
        glm::vec3( 0.5f, -0.5f, 0.0f),  // bottom right
        glm::vec3(-0.5f, -0.5f, 0.0f),  // bottom left
        glm::vec3(-0.5f,  0.5f, 0.0f)}; // top left
      init(_vertices, _color);
    }
    
    //****************************************************************************/
    // glQuad - Constructor
    //****************************************************************************/
    glQuad(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_vertices, _color);
    }
 
    //****************************************************************************/
    // ~glQuad
    //****************************************************************************/
    ~glQuad() { cleanInGpu(); }
    
    //****************************************************************************/
    // setColor()
    //****************************************************************************/
    void setColor(const glm::vec3 & _color) { color = _color; }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0)) {
      
      DEBUG_LOG("glQuad::init(" + name + ")");
      
      vertices = _vertices;
      
      shader.setName(name);
      
      shader.initPlain();
      
      color = _color;
      
      isInited = true;
      
      printf("%f %f %f\n", color[0], color[1], color[2]);
      printf("%f %f %f\n", vertices[0][0], vertices[0][1], vertices[0][2]);

    }
    
    //****************************************************************************/
    // render
    //****************************************************************************/
    void render(const glCamera * camera) {
      
      DEBUG_LOG("glQuad::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glQuad must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);
      
      glDisable(GL_CULL_FACE);

      glBindVertexArray(vao);
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindVertexArray(0);
            
    }
    
  private:
        
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glQuad::setInGpu(" + name + ")");
      
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
      glEnableVertexAttribArray(0);

      glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

      glCheckError();

      glBindVertexArray(0);
      
    }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
        
      }

    }
    
  };
  
} /* namspace ogl */

#endif /* _H_OGL_GLQUAD_H_ */

