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

#include <ogl/model/glLight.hpp>

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
      GLuint vbo[2];
      glm::vec3 color;
      glm::vec2 size;
      ogl::glLight light;
      bool cullFaceEnabled = false;
    
      std::vector<glm::vec3> vertices;

    public:
    
      //****************************************************************************/
      // glQuad()
      //****************************************************************************/
      glQuad(const glm::vec2& _size = glm::vec2(1.0f), const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID, const std::string & _name = "") {
        name = _name;
        init(_size, _color, _style);
      }
    
      //****************************************************************************/
      // glQuad()
      //****************************************************************************/
      glQuad(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID, const std::string & _name = "") {
        name = _name;
        init(_vertices, _color, _style);
      }
    
      //****************************************************************************/
      // ~glQuad()
      //****************************************************************************/
      ~glQuad() { cleanInGpu(); }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(const glm::vec2 & _size, const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID) {
          
        DEBUG_LOG("glQuad::init(" + name + ")");

        shader.setName(name);
        if(_style == glShader::STYLE::WIREFRAME) {
          shader.initWireframe();
        } else {
          shader.initSolid();
        }

        size = _size;
        style = _style;
        color = _color;

        isInited = true;
        
      }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID) {
        
        DEBUG_LOG("glQuad::init(" + name + ")");

        if(_vertices.size() != 6) {
          fprintf(stderr, "glQuad init error vertices must be six\n");
          abort();
        }
        
        shader.setName(name);
        if(_style == glShader::STYLE::WIREFRAME) {
          shader.initWireframe();
        } else {
          shader.initSolid();
        }

        size = glm::vec2(1.0f);
        style = _style;
        color = _color;
      
        vertices = _vertices;

        isInited = true;
        
      }
    
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera * camera) {
          
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
        if(style == glShader::STYLE::SOLID) {
          light.setInShader(shader, camera->getView());
        }

        glBindVertexArray(vao);

        if(style == glShader::STYLE::WIREFRAME) {
          shader.setUniform("lineWidth", lineWidth);
          shader.setUniform("viewport",  camera->getViewport());
          glDisable(GL_CULL_FACE);
        } else if(cullFaceEnabled) {
          glEnable(GL_CULL_FACE);
          glCullFace(GL_BACK);
        } else {
          glDisable(GL_CULL_FACE);
        }

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindVertexArray(0);

        glCheckError();
        
      }

      //****************************************************************************/
      // setLight() - Set the light
      //****************************************************************************/
      void setLight(const glm::vec3 & _position, const glm::vec3 & _direction) {

        light.setPosition(_position);
        light.setDirection(_direction);

      }

      //****************************************************************************/
      // setCullFaceEnabled() - Toggle back-face culling
      //****************************************************************************/
      void setCullFaceEnabled(bool _enabled) { cullFaceEnabled = _enabled; }

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
        
        glGenBuffers(2, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);

        glm::vec3 normal(0.0f, 0.0f, 1.0f);
        if(vertices.size() == 6) {
          glm::vec3 edgeA = vertices[1] - vertices[0];
          glm::vec3 edgeB = vertices[2] - vertices[0];
          glm::vec3 computed = glm::normalize(glm::cross(edgeA, edgeB));
          if(glm::length(computed) > 0.0f) {
            normal = computed;
          }
        }

        glm::vec3 normals[6] = { normal, normal, normal, normal, normal, normal };

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, nullptr);
        glEnableVertexAttribArray(1);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        
        glCheckError();
        
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

} // namespace ogl

#endif /* _H_OGL_GLQUAD_H_ */
