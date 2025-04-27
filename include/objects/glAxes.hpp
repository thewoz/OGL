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
      
      std::vector<glm::vec3> vertices;
      std::vector<glm::vec3> colors;
      
      glPrint3D xAxeLabel;
      glPrint3D yAxeLabel;
      glPrint3D zAxeLabel;

    public:
        
      //****************************************************************************/
      // glAxes()
      //****************************************************************************/
      glAxes(GLfloat _scale = 1.0, const std::string & _name = " ") {
                
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
    
        xAxeLabel.init("x", glm::vec3(1.01f,0.01f,0.01f), glm::vec3(1.0f), 0.5);
        yAxeLabel.init("y", glm::vec3(0.01f,1.01f,0.01f), glm::vec3(1.0f), 0.5);
        zAxeLabel.init("z", glm::vec3(0.01f,0.01f,1.01f), glm::vec3(1.0f), 0.5);

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
                
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        glBindVertexArray(vao);

        glDisable(GL_CULL_FACE);

        for(int i=0; i<3; ++i) {
          
          shader.setUniform("color", colors[i]);
 	 
          glDrawArrays(GL_LINES, i*2, 2);

        }
        
        glBindVertexArray(0);

        glCheckError();

        xAxeLabel.render(camera);
        yAxeLabel.render(camera);
        zAxeLabel.render(camera);

      }
   
      //****************************************************************************/
      // translate()
      //****************************************************************************/
      inline void translate(const glm::vec3 & value) {
        
        _position = value;
        
        xAxeLabel.translate(_position);
        yAxeLabel.translate(_position);
        zAxeLabel.translate(_position);
        
        updateModelMatrix();
        
      }

    
    private:
      
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() {
   
        DEBUG_LOG("glAxes::setInGpu(" + name + ")");
                  
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
        glEnableVertexAttribArray(0); 
        
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
       
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
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_AXES_H_ */
