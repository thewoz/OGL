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

#include <cstdio>
#include <cstdlib>

#include <vector>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // class glQuad
  //****************************************************************************/
  class glQuad : public glObject {
      
    private:
    
      GLuint vao = 0;
      GLuint vbo = 0;
      glm::vec3 color;
      glm::vec2 size;
    
      std::vector<glm::vec3> vertices;

    public:
    
      //****************************************************************************/
      // glQuad()
      //****************************************************************************/
      glQuad(const glm::vec2& _size = glm::vec2(1.0f), const glm::vec3 & _color = glm::vec3(1.0f), const std::string & _name = "") {
        name = _name;
        init(_size, _color);
      }
    
      //****************************************************************************/
      // glQuad()
      //****************************************************************************/
      glQuad(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0f), const std::string & _name = "") {
        name = _name;
        init(_vertices, _color);
      }
    
      //****************************************************************************/
      // ~glQuad()
      //****************************************************************************/
      ~glQuad() { cleanInGpu(); }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(const glm::vec2 & _size, const glm::vec3 & _color = glm::vec3(1.0f)) {
          
        DEBUG_LOG("glQuad::init(" + name + ")");

        shader.setName(name);
        shader.initPlain();

        size = _size;
        color = _color;

        isInited = true;
        
      }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0f)) {
        
        DEBUG_LOG("glQuad::init(" + name + ")");

        if(_vertices.size() != 6) {
          fprintf(stderr, "glQuad init error vertices must be six\n");
          abort();
        }
        
        shader.setName(name);
        shader.initPlain();

        size = glm::vec2(1.0f);
        color = _color;
      
        vertices = _vertices;

        isInited = true;
        
      }
    
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera* camera) {
          
        DEBUG_LOG("glQuad::render(" + name + ")");

        if(!isInited) {
          fprintf(stderr, "glQuad must be inited before render\n");
          abort();
        }

        if(isToInitInGpu()) initInGpu();

        shader.use();

        shader.setUniform("projection", camera->getProjection());
        shader.setUniform("view", camera->getView());
        shader.setUniform("model", modelMatrix);
        shader.setUniform("color", color);

        glBindVertexArray(vao);

        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);

        glCheckError();
        
      }

    private:
    
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() {
        
        DEBUG_LOG("glQuad::setInGpu(" + name + ")");
        
        GLfloat _vertices[18]; // 6 vertices * 3 coords
        
        if(vertices.size() != 0) {
          
          for(int i=0; i<6; ++i) {
            _vertices[i * 3 + 0] = vertices[i].x;
            _vertices[i * 3 + 1] = vertices[i].y;
            _vertices[i * 3 + 2] = vertices[i].z;
          }
          
        } else {
          
          GLfloat defaultVertices[] = { -0.5f * size.x,  0.5f * size.y, 0.0f,
            -0.5f * size.x, -0.5f * size.y, 0.0f,
            0.5f * size.x, -0.5f * size.y, 0.0f,
            -0.5f * size.x,  0.5f * size.y, 0.0f,
            0.5f * size.x, -0.5f * size.y, 0.0f,
            0.5f * size.x,  0.5f * size.y, 0.0f };
          
          memcpy(_vertices, defaultVertices, sizeof(defaultVertices));
          
        }
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        glCheckError();
        
      }
    
      //****************************************************************************/
      // cleanInGpu()
      //****************************************************************************/
      void cleanInGpu() {
        
        if(isInitedInGpu) {
          glDeleteBuffers(1, &vbo);
          glDeleteVertexArrays(1, &vao);
          isInitedInGpu = false;
        }
        
      }
    
  };

} // namespace ogl

#endif /* _H_OGL_GLQUAD_H_ */
