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

#ifndef _H_OGL_GLQUAD_H_
#define _H_OGL_GLQUAD_H_

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glQuad
  //****************************************************************************/
  class glQuad : public glObject {
    
  private:
    
    GLuint vao = -1;
    GLuint vbo = -1;
    GLuint ebo = -1;

//    float vertices[12] = {
//      -0.5f, 0.5f, 0.0f,
//       0.5,  0.5f, 0.0f,
//       0.5, -0.5f, 0.0f,
//      -0.5, -0.5f, 0.0f };
//
    float vertices[18] = {

    0.5f,  0.5f, 0.0f,  // top right
    0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f,  0.5f, 0.0f,  // top left
                         // second triangle
    0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left
     };
//    unsigned int indices[6] = {
//      0, 1, 3,   // first triangle
//      1, 2, 3    // second triangle
//    };
    
    glm::vec3 color = glm::vec3(1.0);
    
  public:
    
    //****************************************************************************/
    // glQuad - Constructor
    //****************************************************************************/
    glQuad(const std::string & _name = "") {
      
      name = _name;
      
      isInited = true;
      
      shader.setName(name);
      shader.initPlain();
      
    }
    
    //****************************************************************************/
    // ~glQuad
    //****************************************************************************/
    ~glQuad() { cleanInGpu(); }
    
    //****************************************************************************/
    // render
    //****************************************************************************/
    void render(const glCamera * camera) {
      
      DEBUG_LOG("glQuad::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glQuad must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getLookAt(getTranslation()));
      //shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);
      
      glEnable(GL_DEPTH_TEST);

      glDisable(GL_CULL_FACE);

      glBindVertexArray(vao);
      
      glEnableVertexAttribArray(0);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindVertexArray(0);

      glDisable(GL_DEPTH_TEST);
            
    }
    
  private:
        
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glQuad::setInGpu(" + name + ")");
      
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
      
      glEnableVertexAttribArray(0);

      glBindVertexArray(0);
      
    }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
        
      }

    }
    
  };
  
} /* namspace ogl */

#endif /* _H_OGL_GLQUAD_H_ */

