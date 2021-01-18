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
    
    int slices;
    int lenght;
    
    glm::vec3 color;
    
  public:
    
    //****************************************************************************/
    // glGrid()
    //****************************************************************************/
    glGrid(const std::string & _name = "") { name = _name; }
    glGrid(int _slices, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_slices, _color);
    }
    
    //****************************************************************************/
    // ~glGrid()
    //****************************************************************************/
    ~glGrid() { cleanInGpu(); }
 
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(int _slices, const glm::vec3 & _color = glm::vec3(0.0)) {
      
      DEBUG_LOG("glGrid::init(" + name + ")");

      shader.setName(name);
      
      shader.initPlain();
      
      slices = _slices;

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
      
      glEnable(GL_DEPTH_TEST);
      
      glBindVertexArray(vao);
          
      glDrawElements(GL_LINES, lenght, GL_UNSIGNED_INT, nullptr);

      glBindVertexArray(0);
            
      glDisable(GL_DEPTH_TEST);

    }
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glGrid::setInGpu(" + name + ")");

      std::vector<glm::vec3> vertices;
      std::vector<glm::uvec4> indices;
         
      for(int j=0; j<=slices; ++j) {
        for(int i=0; i<=slices; ++i) {
          float x = (float)i/(float)slices;
          float y = 0;
          float z = (float)j/(float)slices;
          vertices.push_back(glm::vec3(x, y, z));
        }
      }
           
      for(int j=0; j<slices; ++j) {
        for(int i=0; i<slices; ++i) {
          
          int row1 =  j    * (slices+1);
          int row2 = (j+1) * (slices+1);
          
          indices.push_back(glm::uvec4(row1+i, row1+i+1, row1+i+1, row2+i+1));
          indices.push_back(glm::uvec4(row2+i+1, row2+i, row2+i, row1+i));

        }
      }

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), glm::value_ptr(vertices[0]), GL_STATIC_DRAW);
         
      glGenBuffers(1, &ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(glm::uvec4), glm::value_ptr(indices[0]), GL_STATIC_DRAW);

      glBindVertexArray(0);
      //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      //glBindBuffer(GL_ARRAY_BUFFER, 0);
      
      lenght = (GLuint)indices.size()*4;
      
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
