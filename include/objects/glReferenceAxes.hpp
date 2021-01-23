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

#ifndef _H_OGL_REFERENCE_AXES_H_
#define _H_OGL_REFERENCE_AXES_H_

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glReferenceAxes
  //****************************************************************************/
  class glReferenceAxes : public glObject {
    
    private:
      
      GLuint vao;
      GLuint vbo;
      
      std::vector<glm::vec3> colors;
      std::vector<glm::vec3> vertices;

      //glPrint3D xlabel;
      //glPrint3D ylabel;
      //glPrint3D zlabel;

    public:
        
      //****************************************************************************/
      // glReferenceAxes()
      //****************************************************************************/
      glReferenceAxes(float _scale = 1.0, const std::string & _name = " "){
        name = _name;
        init(_scale);
      }
      
      //****************************************************************************/
      // ~glReferenceAxes()
      //****************************************************************************/
      ~glReferenceAxes() { cleanInGpu(); }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(float _scale = 1.0) {
        
        DEBUG_LOG("glReferenceAxes::init(" + name + ")");
        
        shader.setName(name);
        
        shader.initPlain();
        
        scale(glm::vec3(_scale));

        vertices.resize(6);
        
        vertices[0] = glm::vec3(0.0f,0.0f,0.0f);
        vertices[1] = glm::vec3(1.0f,0.0f,0.0f);
        
        vertices[2] = glm::vec3(0.0f,0.0f,0.0f);
        vertices[3] = glm::vec3(0.0f,1.0f,0.0f);
        
        vertices[4] = glm::vec3(0.0f,0.0f,0.0f);
        vertices[5] = glm::vec3(0.0f,0.0f,1.0f);
        
        colors.resize(3);
        
        colors[0] = glm::vec3(1.0f,0.0f,0.0f);
        colors[1] = glm::vec3(0.0f,1.0f,0.0f);
        colors[2] = glm::vec3(0.0f,0.0f,1.0f);
        
//        xAxeLabel.init(glm::vec3(1.01f,0.01f,0.01f), glm::vec3(1.0f), 0.5, "x");
//        yAxeLabel.init(glm::vec3(0.01f,1.01f,0.01f), glm::vec3(1.0f), 0.5, "y");
//        zAxeLabel.init(glm::vec3(0.01f,0.01f,1.01f), glm::vec3(1.0f), 0.5, "z");
        
        isInited = true;
        
      }
    
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera * camera) {

        DEBUG_LOG("glReferenceAxes::render(" + name + ")");
        
        if(!isInited){
          fprintf(stderr, "glReferenceAxes must be inited before render\n");
          abort();
        }
        
        if(isToInitInGpu()) initInGpu();
        
        shader.use();
        
        GLint viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport);
        
        glViewport(viewport[2] - 160, 10, 160, 160);

        shader.setUniform("projection", camera->get3DOrthoProjection());
        shader.setUniform("view",       camera->getLookAt(glm::vec3(0.0f)));
        shader.setUniform("model",      modelMatrix);
                        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glBindVertexArray(vao);
        
        for(int i=0; i<3; ++i) {
          
          shader.setUniform("color", colors[i]);
          
          glDrawArrays(GL_LINES, i*2, 2);
          
        }
        
        glBindVertexArray(0);
                      
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

      }
      
      
    private:
      
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() {
        
        DEBUG_LOG("glReferenceAxes::setInGpu(" + name + ")");
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);
        
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
        
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
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_REFERENCE_AXES_H_ */
