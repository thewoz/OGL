/*
 * MIT License
 *
 * Copyright © 2017
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
      
      GLuint vao[3];
      GLuint vbo[3];
      
      glm::vec3 colors[3] = { glm::vec3(1.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f) };
      
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
        
        for(size_t i=0; i<3; ++i) {
          
          shader.setUniform("color", colors[i]);
          
          glBindVertexArray(vao[i]);
          
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
          
          glEnableVertexAttribArray(0);
          
          glDrawArrays(GL_LINES, 0, 2);

          glBindVertexArray(0);
          
        }
        
        glDisable(GL_DEPTH_TEST);

      }
  
    private:
      
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() {
        
        DEBUG_LOG("glAxes::setInGpu(" + name + ")");
        
        std::vector<std::vector<glm::vec3>> vertices(3, std::vector<glm::vec3>(2, glm::vec3(0.0f)));
        
        vertices[0][1] = glm::vec3(1.0f,0.0f,0.0f);
        vertices[1][1] = glm::vec3(0.0f,1.0f,0.0f);
        vertices[2][1] = glm::vec3(0.0f,0.0f,1.0f);
        
        for(size_t i=0; i<3; ++i) {
          
          glGenVertexArrays(1, &vao[i]);
          glBindVertexArray(vao[i]);
          
          glGenBuffers(1, &vbo[i]);
          glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
          glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), &vertices[i][0], GL_STATIC_DRAW);
          
          glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
          
        }
        
        glBindBuffer(GL_ARRAY_BUFFER,0);
        
        glBindVertexArray(0);
        
      }
    
  private:
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(3, vbo);
        glDeleteVertexArrays(3, vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_AXES_H_ */
