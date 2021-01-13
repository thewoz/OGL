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

#ifndef _H_OGL_AXES_H_
#define _H_OGL_AXES_H_

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glAxes
  //****************************************************************************/
  class glAxes : public glObject {
    
    private:
      
      GLuint vao;
      GLuint vbo;
      
      glm::vec3 colors[3] = { glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f) };
      
      std::vector<glm::vec3> vertices;
      
    public:
        
      //****************************************************************************/
      // glAxes()
      //****************************************************************************/
      glAxes(GLfloat _scale = 1.0, const std::string & _name = " ") {
                
        vertices.resize(6);
                
        vertices[0] = glm::vec3(0.0f,0.0f,0.0f);
        vertices[1] = glm::vec3(1.0f,0.0f,0.0f);

        vertices[2] = glm::vec3(0.0f,0.0f,0.0f);
        vertices[3] = glm::vec3(0.0f,1.0f,0.0f);

        vertices[4] = glm::vec3(0.0f,0.0f,0.0f);
        vertices[5] = glm::vec3(0.0f,0.0f,1.0f);
                
        name = _name;
        
        init(_scale);
        
      }
      
      //****************************************************************************/
      // ~glAxes()
      //****************************************************************************/
      ~glAxes() { cleanInGpu(); }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(GLfloat _scale = 1.0) {
        
        DEBUG_LOG("gAxes::init(" + name + ")");

        shader.setName(name);
        
        shader.initPlain();
        
        scale(glm::vec3(_scale));

        isInited = true;
        
      }
    
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera * camera) {
        
        DEBUG_LOG("glAxes::render(" + name + ")");
        
        if(!isInited){
          fprintf(stderr, "glAxes must be inited before render\n");
          abort();
        }
        
        if(isToInitInGpu()) initInGpu();
        
        shader.use();
        
        shader.setUniform("projection", camera->getProjection());
        shader.setUniform("view",       camera->getView());
        shader.setUniform("model",      modelMatrix);
        
        glEnable(GL_DEPTH_TEST);
        
        glBindVertexArray(vao);

        glCheckError();

        glEnableVertexAttribArray(0);

        glCheckError();

        for(int i=0; i<3; ++i) {
          
          shader.setUniform("color", colors[i]);
 	  
          //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
          
          glDrawArrays(GL_LINES, i, (i+1)*2);

        }
        
        glBindVertexArray(0);

        // glDisableVertexAttribArray(0);

        glDisable(GL_DEPTH_TEST);

      }
  
    private:
      
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() {
        
        printf("AAA %p\n", this);
        
        DEBUG_LOG("glAxes::setInGpu(" + name + ")");
                  
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
          
         glCheckError();
          
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                  
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        glBindVertexArray(0);
        
        glCheckError();
                
      }
    
  private:
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        printf("pulisco %p\n", this);
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_AXES_H_ */
