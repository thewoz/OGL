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

#ifndef _H_OGL_LINE_H_
#define _H_OGL_LINE_H_


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
  // Class glLine
  //****************************************************************************/
  class glLine : public glObject {
    
  private:
    

    std::vector<glm::vec3> vertices;
    
  public:
    
    //****************************************************************************/
    // glLine()
    //****************************************************************************/
    glLine(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glLine()
    //****************************************************************************/
    glLine(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name  = _name;
      color = _color;
      init(_vertices, _color);
    }
    
    //****************************************************************************/
    // ~glLine()
    //****************************************************************************/
    ~glLine() { releaseInGpu(); }

    glLine(glLine &&) noexcept = default;
    glLine & operator = (glLine &&) noexcept = default;
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0)) {
 
      DEBUG_LOG("glLine::init(" + name + ")");

      shader.setName(name);
     
      shader.initLine();
      
      color = _color;

      vertices = _vertices;

      isInited = true;
      isToUpdateInGpu = true; // re-init after a render must re-upload

    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {
      
      DEBUG_LOG("glLine::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "ERROR [glLine]: must be initialized before rendering\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       camera.getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("lineWidth",  lineWidth);
      shader.setUniform("viewport",   camera.getViewport());
      shader.setUniform("uniformColor", glm::vec4(color, 1.0f));
      
      buffers.bind();

      glDisable(GL_CULL_FACE);
      glDisableVertexAttribArray(1);
      glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

      glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)vertices.size());
     
      glBindVertexArray(0);
      
      glCheckError();
            
    }
    
    //****************************************************************************/
    // setInGpu
    //****************************************************************************/
    void setInGpu() override {
      
      DEBUG_LOG("glLine::setInGpu(" + name + ")");

      prepareBuffers(1);

      buffers.attribute(0, 0, 3, vertices.data(), vertices.size() * sizeof(glm::vec3));

      glBuffers::unbind();
      
      glCheckError();
          
    }
    
    //****************************************************************************/
    // update() -
    //****************************************************************************/
    void update(const std::vector<glm::vec3> & _vertices) {
      
      DEBUG_LOG("glLine::update(" + name + ")");

      if(!isInited) init(_vertices, color);
      else vertices = _vertices;
      
      isToUpdateInGpu = true;
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_LINE_H_ */
