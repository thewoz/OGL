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

#ifndef _H_MPL_GLSHADOW_H_
#define _H_MPL_GLSHADOW_H_

#include <cstdlib>
#include <cstdio>

#include <glm/glm.hpp>

#include "glShader.hpp"
#include "glQuad.hpp"

//class glModel;

/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {
  
  /*****************************************************************************/
  // glShadow
  /*****************************************************************************/
  class glShadow {
    
  public:
    
    glTextureDepthMap depthMap;
    
  private:
    
    glShader shader;
    
    glShader shaderDepth;
    
    glQuad quad;
    
    GLuint fbo;
    
    GLuint width;
    GLuint height;
    
    glm::mat4 lightSpaceMatrix;
    
  public:
    
    /*****************************************************************************/
    // glShadow - Empty constructor
    /*****************************************************************************/
    glShadow() { }
    
    /*****************************************************************************/
    // init
    /*****************************************************************************/
    void init(GLuint _width = 1024, GLuint _height = 1024) {
      
      width  = _width;
      height = _height;

      // load and compile the shadow shader
      shader.init("/usr/local/include/ogl/opengl/shader/shadow/shadowMap.vs", "/usr/local/include/ogl/opengl/shader/shadow/shadowMap.fs");
      
      //quad.init();
      
      shaderDepth.init("/usr/local/include/ogl/opengl/shader/depth.vs", "/usr/local/include/ogl/opengl/shader/depth.fs");
      
      // configure depth map FBO
      glGenFramebuffers(1, &fbo);
      
      // create depth cubemap texture
      depthMap.init(width,height);
      
      // attach depth texture as FBO's depth buffer
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
      
      //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap.getId(), 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap.getId(), 0);
      
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);
      
      // Always check that our framebuffer is ok
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("DIOCANE ME\n");
      };
      
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      
      //fprintf(stderr, "DEBUG shadow shader: %d FBO: %d texture id: %d\n", shader.get(), FBO, depthMap.getId());
      
    }
    
    
    /*****************************************************************************/
    // renderBegin
    /*****************************************************************************/
    void renderBegin(const glm::vec3 & lightDir) {
      
      glViewport(0,0,width,height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
      
      // We don't use bias in the shader, but instead we draw back faces,
      // which are already separated from the front faces by a small distance
      // (if your geometry is made this way)
      glEnable(GL_CULL_FACE);
      
      // Cull back-facing triangles -> draw only front-facing triangles
      glCullFace(GL_BACK);
      
      // Clear the screen
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      // Use our shader
      shaderDepth.use();

      // Render to our framebuffer
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      
      // Compute the MVP matrix from the light's point of view
      glm::mat4 depthProjectionMatrix = glm::ortho<float>(-10,10,-10,10,-10,20);
      glm::mat4 depthViewMatrix = glm::lookAt(lightDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
      
      glm::mat4 depthModelMatrix = glm::mat4(1.0);
      glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
      
     // shaderDepth.setUniform(<#const std::string &name#>, depthMVP);
      
      
      
    }
      
    /*****************************************************************************/
    // renderEnd
    /*****************************************************************************/
    void renderBegin() {
      
      
    }
      
      
      
      
      
      
      
      
      
      /*****************************************************************************/
      // compute
      /*****************************************************************************/
      void compute(const glm::vec3 & lightPosition) {
      
      GLint viewport[4];
      
      glGetIntegerv(GL_VIEWPORT, viewport);
      
      float lightAngleX = glm::acos(lightPosition.z);
      float lightAngleY = glm::asin(lightPosition.x);
      float lightAngleZ = 0;
      
      float factor = -10, near = 0.001, far = 10;
      
      glm::mat4 projection = glm::ortho(-1.5f, 1.5f, -1.5f, 1.5f, near, far);
      glm::mat4 view       = glm::translate(glm::mat4(), lightPosition * factor) * glm::mat4_cast(glm::quat(glm::vec3(lightAngleX, lightAngleY, lightAngleZ)));
      
      //lightSpaceMatrix = projection * view * model.getModelMatrix();
      
      ////glEnable(GL_POLYGON_OFFSET_FILL);
      
      ////glPolygonOffset(1.0f, 1.0f);
      
      glViewport(0, 0, width, height);
      
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
      
      glClearDepth(1.0);
      
      glClear(GL_DEPTH_BUFFER_BIT);
      
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      
      shader.use();
      
      shader.setUniform("lightSpaceMatrix", lightSpaceMatrix);
      
      //model.render(projection, view, false);
      
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      
      //glDisable(GL_POLYGON_OFFSET_FILL);
      
      glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
      
      
    }
    
    /*****************************************************************************/
    // render
    /*****************************************************************************/
    void render() {
      
      GLint viewport[4];
      
      glGetIntegerv(GL_VIEWPORT, viewport);
      
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
      
      glViewport(0, 0, width, height);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      //glDisable(GL_CULL_FACE);
      
      //glCullFace(GL_BACK);
      
      shaderDepth.use();
      
      depthMap.activate(0);
      
      quad.render();
      
      glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
      
    }
    
    /*****************************************************************************/
    // setInShader
    /*****************************************************************************/
    void setInShader(const glShader & program) const {


      //depthMap.activate(0);

      //    glm::mat4 biasMatrix(
      //                         0.5, 0.0, 0.0, 0.0,
      //                         0.0, 0.5, 0.0, 0.0,
      //                         0.0, 0.0, 0.5, 0.0,
      //                         0.5, 0.5, 0.5, 1.0
      //                         );

      //program.setUniform("shadowMap", 0);

      shader.setUniform("lightSpaceMatrix", lightSpaceMatrix);

      //program.setUniform("depthBiasMVP", depthBiasMVP);

    }
    
    
  };
  
} /* namespace ogl */


#endif /* _H_MPL_GLSHADOW_H_ */
