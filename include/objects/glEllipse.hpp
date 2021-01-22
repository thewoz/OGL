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

#ifndef _H_OGL_ELLIPSE_H_
#define _H_OGL_ELLIPSE_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // Class glEllipse
  //****************************************************************************/
  class glEllipse : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo[4];

    int stacks;
    int slices;
    
    float a;
    float b;
    float c;
    
    glm::vec3 color;
    
  public:
        
    //****************************************************************************/
    // glEllipse
    //****************************************************************************/
    glEllipse(const std::string & _name = "") { name = _name; }

    //****************************************************************************/
    // glEllipse()
    //****************************************************************************/
    glEllipse(float _a, float _b, float _c, int _stacks, int _slices, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      
      name = _name;
      
      init(_a, _b, _c, _stacks, _slices, _style, _color); }
    
    //****************************************************************************/
    // ~glEllipse()
    //****************************************************************************/
    ~glEllipse() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(float _a, float _b, float _c, int _stacks, int _slices, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0)) {
      
      DEBUG_LOG("glEllipse::init(" + name + ")");

      shader.setName(name);
      
      shader.initPlain();
      
      stacks = _stacks;
      slices = _slices;
      
      a = _a;
      b = _b;
      c = _c;

      style = _style;
      
      color = _color;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera) {
            
      DEBUG_LOG("glEllipse::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glEllipse must be inited before render\n");
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

      if(style == glObject::STYLE::WIREFRAME) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      if(style == glObject::STYLE::SOLID)     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
      glDrawElements(GL_TRIANGLES, (slices * stacks + slices) * 6, GL_UNSIGNED_INT, nullptr);
      
      glBindVertexArray(0);
      
    }
    
    private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glEllipse::setInGpu(" + name + ")");
          
      if(!isInitedInGpu) {

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> textureCoords;
        
        for(int i=0; i<=stacks; ++i) {

          // V texture coordinate
          float V   = i / (float)stacks;
          float phi = V * M_PI - M_PI/2.0;

          for(int j=0; j<=slices; ++j) {
            
            // U texture coordinate
            float U     = j / (float)slices;
            float theta = U * 2.0f * M_PI;
            
            float X = a * cos(phi) * cos(theta);
            float Y = b * cos(phi) * sin(theta);
            float Z = c * sin(phi);
                      
            positions.push_back( glm::vec3( X, Y, Z) );
            normals.push_back( glm::vec3(X, Y, Z) );
            textureCoords.push_back( glm::vec2(U, V) );
            
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

        glCheckError();
        
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

#endif /* _H_OGL_ELLIPSE_H_ */


