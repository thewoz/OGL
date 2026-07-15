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

#ifndef _H_OGL_AXES_H_
#define _H_OGL_AXES_H_


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
  // Class glAxes
  //****************************************************************************/
  class glAxes : public glObject {
    
    private:
      
      GLuint vao = 0;
      GLuint vbo = 0;
      
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

      glAxes(glAxes &&) noexcept = default;
      glAxes & operator = (glAxes &&) noexcept = default;
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(GLfloat _scale = 1.0) {
        
        DEBUG_LOG("gAxes::init(" + name + ")");

        shader.setName(name);

        shader.initLine();

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

        // The labels are independent objects anchored at the unit tips: their
        // own model matrix (kept in sync by the translate/rotate/scale
        // overrides below) carries them to the actual axis tips, so they keep
        // following the axes when the object is transformed after init().
        xAxeLabel.init("x", glm::vec3(1.01f,0.01f,0.01f), glm::vec3(1.0f), 0.5);
        yAxeLabel.init("y", glm::vec3(0.01f,1.01f,0.01f), glm::vec3(1.0f), 0.5);
        zAxeLabel.init("z", glm::vec3(0.01f,0.01f,1.01f), glm::vec3(1.0f), 0.5);

        // After the labels exist: the scale() override below forwards the
        // transform to them, so one call scales both axes and labels.
        scale(glm::vec3(_scale));

        isInited = true;
        isToUpdateInGpu = true; // re-init after a render must re-upload

      }
    
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera & camera) {
        
        DEBUG_LOG("glAxes::render(" + name + ")");
        
        if(!isInited){
          fprintf(stderr, "ERROR [glAxes]: must be initialized before rendering\n");
          abort();
        }
        
        if(isToInitInGpu()) initInGpu();

        shader.use();
        
        shader.setUniform("projection", camera.getProjection());
        shader.setUniform("view",       camera.getView());
        shader.setUniform("model",      modelMatrix);
        shader.setUniform("lineWidth",  lineWidth);
        shader.setUniform("viewport",   camera.getViewport());
                
        glBindVertexArray(vao);

        glDisable(GL_CULL_FACE);
        glDisableVertexAttribArray(1);
        glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

        for(int i=0; i<3; ++i) {
          
          shader.setUniform("uniformColor", glm::vec4(colors[i], 1.0f));
 	 
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
      void translate(const glm::vec3 & value) override {

        _position = value;

        xAxeLabel.translate(_position);
        yAxeLabel.translate(_position);
        zAxeLabel.translate(_position);

        updateModelMatrix();

      }

      //****************************************************************************/
      // rotate() / scale() - keep the label anchors on the axis tips
      //****************************************************************************/
      void rotate(const glm::vec3 & value) override {

        glObject::rotate(value);

        xAxeLabel.rotate(value);
        yAxeLabel.rotate(value);
        zAxeLabel.rotate(value);

      }

      void scale(const glm::vec3 & value) override {

        glObject::scale(value);

        xAxeLabel.scale(value);
        yAxeLabel.scale(value);
        zAxeLabel.scale(value);

      }

    
    private:
      
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() override {

        DEBUG_LOG("glAxes::setInGpu(" + name + ")");

        // Same-context re-upload (init() called again): drop the old buffers
        // first. After a context change isInitedInGpu is already false, so this
        // is a no-op and the old-context handles are (deliberately) left alone.
        cleanInGpu();

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
    void cleanInGpu() override {

      if(isInitedInGpu) {

        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_AXES_H_ */
