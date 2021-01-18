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

#ifndef _H_OGL_SPHERE_H_
#define _H_OGL_SPHERE_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glSphere
  //****************************************************************************/
  class glSphere : public glObject {
    
    private:
      
      GLuint vao = -1;
      GLuint vbo[4];
      
      float radius;
      int   slices;
      int   stacks;
    
      glm::vec3 color;
      
    public:
      
      //****************************************************************************/
      // glSphere()
      //****************************************************************************/
      glSphere(const std::string & _name = "") { name = _name; }
      
      //****************************************************************************/
      // glSphere()
      //****************************************************************************/
      glSphere(float _radius, int _slices, int _stacks, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
        name = _name;
        init(_radius, _slices, _stacks, _style, _color);
      }
      
      //****************************************************************************/
      // ~glSphere()
      //****************************************************************************/
      ~glSphere() { cleanInGpu(); }
      
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(float _radius, int _slices, int _stacks, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0)) {
        
        DEBUG_LOG("glSphere::init(" + name + ")");

        shader.setName(name);
        
        shader.initPlain();
        
        radius = _radius;
        
        slices = _slices;
        stacks = _stacks;
        
        style = _style;
        
        color = _color;
        
        isInited = true;
        
      }
      
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera * camera) {
        
        DEBUG_LOG("gSphere::render(" + name + ")");
        
        if(!isInited){
          fprintf(stderr, "glSphere must be inited before render\n");
          abort();
        }
        
        if(isToInitInGpu()) initInGpu();
        
        shader.use();

        shader.setUniform("projection", camera->getProjection());
        shader.setUniform("view",       camera->getView());
        shader.setUniform("model",      modelMatrix);
        shader.setUniform("color",      color);
        
        glEnable(GL_DEPTH_TEST);
        
        glDisable(GL_CULL_FACE);
        
        glBindVertexArray(vao);
        
//        glEnableVertexAttribArray(0);
//        glEnableVertexAttribArray(1);
//        glEnableVertexAttribArray(2);
        
        if(style == glObject::STYLE::WIREFRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if(style == glObject::STYLE::SOLID)     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
        glDrawElements(GL_TRIANGLES, (slices * stacks + slices) * 6, GL_UNSIGNED_INT, nullptr);
        
//        glDisableVertexAttribArray(2);
//        glDisableVertexAttribArray(1);
//        glDisableVertexAttribArray(0);
        
        glBindVertexArray(0);
        
        glDisable(GL_DEPTH_TEST);

      }
      
    private:
      
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() {
        
        DEBUG_LOG("glSphere::setInGpu(" + name + ")");
        
        if(!isInitedInGpu) {
          
          std::vector<glm::vec3> positions;
          std::vector<glm::vec3> normals;
          std::vector<glm::vec2> textureCoords;
          
          for(int i = 0; i <= stacks; ++i) {
            
            // V texture coordinate.
            float V = i / (float)stacks;
            float phi = V * M_PI;
            
            for( int j = 0; j <= slices; ++j) {
              
              // U texture coordinate.
              float U = j / (float)slices;
              float theta = U * 2.0f * M_PI;
              
              float X = cos(theta) * sin(phi);
              float Y = cos(phi);
              float Z = sin(theta) * sin(phi);
              
              positions.push_back(glm::vec3( X, Y, Z) * radius);
              normals.push_back(glm::vec3(X, Y, Z));
              textureCoords.push_back(glm::vec2(U, V));
              
            }
            
          }
          
          // Now generate the index buffer
          std::vector<GLuint> indicies;
          
          int noPerSlice = slices + 1;
          
          for(int i=0; i < stacks; ++i) {
            
            for (int j=0; j < slices; ++j) {
              
              int start_i = (i * noPerSlice) + j;
              
              indicies.push_back( start_i );
              indicies.push_back( start_i + noPerSlice + 1 );
              indicies.push_back( start_i + noPerSlice );
              
              indicies.push_back( start_i + noPerSlice + 1 );
              indicies.push_back( start_i );
              indicies.push_back( start_i + 1 );
              
            }
            
          }
          
          glGenVertexArrays(1, &vao);
          glBindVertexArray(vao);
          
          glGenBuffers(4, vbo);
          
          glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
          
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
          glEnableVertexAttribArray(0);
          
          glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
          
          glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
          
          glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, nullptr);
          glEnableVertexAttribArray(1);
          
          glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
          
          glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
          
          glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
          glEnableVertexAttribArray(2);
          
          glBufferData(GL_ARRAY_BUFFER, textureCoords.size() * sizeof(glm::vec2), textureCoords.data(), GL_STATIC_DRAW);
          
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(GLuint), indicies.data(), GL_STATIC_DRAW);
          
          glBindVertexArray(0);
//          glBindBuffer(GL_ARRAY_BUFFER, 0);
//          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
       
        }
        
        isInitedInGpu = true;
        
      }
    
    
  private:
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(4, vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_SPHERE_H_ */
