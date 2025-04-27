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

#ifndef _H_OGL_GRID_H_
#define _H_OGL_GRID_H_

#include <cstdio>
#include <cstdlib>

//#include <ogl/core/glObject.hpp>

/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {

  /*****************************************************************************/
  // Class glGrid
  /*****************************************************************************/
  class glGrid : public glObject {
    
  private:
        
    GLuint vao = -1;
    GLuint vbo = -1;
    GLuint ibo = -1;
    
    int rows;
    int cols;
    
    float cellSize;
    
    glm::vec3 color;
    
    std::vector<GLuint> indices;
    
  public:
    
    //****************************************************************************/
    // glGrid()
    //****************************************************************************/
    glGrid(const std::string & _name = "") { name = _name; }
    glGrid(int _rows, int _cols, float _cellSize, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_rows, _cols, _cellSize, _color);
    }
    
    //****************************************************************************/
    // ~glGrid()
    //****************************************************************************/
    ~glGrid() { cleanInGpu(); }
 
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(int _rows, int _cols, float _cellSize, const glm::vec3 & _color = glm::vec3(0.0)) {
      
      DEBUG_LOG("glGrid::init(" + name + ")");

      shader.setName(name);
      
      shader.initPlain();
      
      rows = _rows;
      
      cols = _cols;
      
      cellSize = _cellSize;

      color = _color;
              
      isInited = true;
      
    }
   
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera) {
      
      DEBUG_LOG("glGrid::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "glGrid must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);

      glBindVertexArray(vao);
          
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      glDisable(GL_CULL_FACE);

      glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
      
      glBindVertexArray(0);
      
      glCheckError();
            
    }
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glGrid::setInGpu(" + name + ")");

      indices.clear();

      float halfWidth  = (cols * cellSize) * 0.5f;
      float halfHeight = (rows * cellSize) * 0.5f;

      std::vector<glm::vec3> vertices;

      // Linee verticali
      for(int i = 0; i <= cols; ++i) {
          float x = -halfWidth + i * cellSize;
          vertices.push_back(glm::vec3(x, 0.0f, -halfHeight));
          vertices.push_back(glm::vec3(x, 0.0f,  halfHeight));
          indices.push_back((GLuint)vertices.size() - 2);
          indices.push_back((GLuint)vertices.size() - 1);
      }

      // Linee orizzontali
      for(int j = 0; j <= rows; ++j) {
          float z = -halfHeight + j * cellSize;
          vertices.push_back(glm::vec3(-halfWidth, 0, z));
          vertices.push_back(glm::vec3( halfWidth, 0, z));
          indices.push_back((GLuint)vertices.size() - 2);
          indices.push_back((GLuint)vertices.size() - 1);
      }

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      glGenBuffers(1, &ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

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
        
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GRID_H_ */
