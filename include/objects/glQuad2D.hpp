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

#ifndef _H_OGL_GLQUAD2D_H_
#define _H_OGL_GLQUAD2D_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glQuad
  //****************************************************************************/
  class glQuad2D : public glObject {
    
  private:
    
    GLuint vao = 0;
    GLuint vbo = 0;

    glm::vec2 position;

    glm::vec2 size;

    // RGBA fill color. Named differently from glObject::color (vec3) so it does
    // not shadow it: the inherited setColor(vec3)/getColor() stay meaningful.
    glm::vec4 fillColor;

    std::vector<glm::vec2> vertices;
    
  public:
    
    //****************************************************************************/
    // glQuad2D - Constructor
    //****************************************************************************/
    glQuad2D(const std::string & _name = "", glm::vec4 _color = glm::vec4(1.0)) { name = _name; fillColor = _color; }
    
    //****************************************************************************/
    // ~glQuad2D
    //****************************************************************************/
    ~glQuad2D() { cleanInGpu(); }

    glQuad2D(glQuad2D &&) noexcept = default;
    glQuad2D & operator = (glQuad2D &&) noexcept = default;
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(glm::vec2 _pos, glm::vec2 _size, const glm::vec4 & _color = glm::vec4(1.0)) {

      fillColor = _color;
      
      position = _pos;
      
      size = _size;
      
      vertices.resize(6);

      float x = position.x;
      float y = position.y;
      
      float w = size.x;
      float h = size.y;
                                         // first triangle
      vertices[0] = glm::vec2(x+w, y);   // top right
      vertices[1] = glm::vec2(x+w, y+h); // bottom right
      vertices[2] = glm::vec2(x,   y);   // top left
                                         // second triangle
      vertices[3] = glm::vec2(x+w, y+h); // bottom right
      vertices[4] = glm::vec2(x,   y+h); // bottom left
      vertices[5] = glm::vec2(x,   y);   // top left
      
      shader.setName(name);

      shader.initPlain2D();
      
      isInited = true;

    }
    
    //****************************************************************************/
    // render
    //****************************************************************************/
    void render(const glCamera & camera) {
      
      DEBUG_LOG("glQuad2D::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "ERROR [glQuad2D]: must be initialized before rendering\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("color", fillColor);
      shader.setUniform("viewport", camera.getViewport());
      
      glDisable(GL_CULL_FACE);

      glEnable(GL_BLEND);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glBindVertexArray(vao);
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindVertexArray(0);
            
      glCheckError();

    }

    //****************************************************************************/
    // setColor() - RGBA fill color (the vec3 overload keeps the current alpha)
    //****************************************************************************/
    inline void setColor(const glm::vec4 & _color) { fillColor = _color; }
    inline void setColor(const glm::vec3 & _color) { fillColor = glm::vec4(_color, fillColor.a); }

  private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {
      
      DEBUG_LOG("glQuad2D::setInGpu(" + name + ")");

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
      
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      glCheckError();
      
    }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() override {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
        
      }

    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLQUAD2D_H_ */
