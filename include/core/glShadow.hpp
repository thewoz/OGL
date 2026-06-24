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

#ifndef _H_OGL_GLSHADOW_H_
#define _H_OGL_GLSHADOW_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {

  //****************************************************************************//
  // glShadow
  //****************************************************************************//
  // Single 2D shadow map (depth-only framebuffer) plus the depth-only shader
  // used to fill it. It is a low-level helper owned by glScene; user code talks
  // to glScene, not to glShadow directly.
  //
  // The same 2D map serves both light types through a single 'lightSpaceMatrix':
  //   - directional light: orthographic projection along the light direction;
  //   - point light:       perspective projection aimed at the scene center.
  // A point light therefore casts shadows inside the cone that looks at the
  // scene center, not omnidirectionally (that would need a depth cubemap). This
  // is intentionally kept simple and is adequate for a scene viewer.
  //
  // GPU resources (FBO + depth texture) are created lazily on the first pass and
  // re-created if the GL context (window) changes, mirroring the rest of OGL.
  //****************************************************************************//
  class glShadow {

  private:

    GLuint fbo      = 0;
    GLuint depthTex = 0;

    GLsizei resolution;

    uint32_t windowID = 0;
    bool isInitedInGpu = false;

    glShader depthShader;

    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);

    // Viewport saved on beginPass() and restored on endPass().
    GLint savedViewport[4] = {0, 0, 0, 0};

  public:

    //****************************************************************************//
    // glShadow
    //****************************************************************************//
    glShadow(GLsizei _resolution = 2048) : resolution(_resolution) {
      depthShader.setName("shadow");
      depthShader.initShadow();
    }

    //****************************************************************************//
    // ~glShadow
    //****************************************************************************//
    ~glShadow() { cleanInGpu(); }

    glShadow(const glShadow &) = delete;
    glShadow & operator = (const glShadow &) = delete;

    //****************************************************************************//
    // setDirectional - frame the scene with an orthographic light box
    //****************************************************************************//
    void setDirectional(const glm::vec3 & direction, const glm::vec3 & center, float radius) {

      if(radius <= 0.0f) radius = 1.0f;

      glm::vec3 dir = glm::length(direction) > 0.0f ? glm::normalize(direction) : glm::vec3(0.0f, -1.0f, 0.0f);

      // Place the virtual light eye outside the scene, looking at its center.
      glm::vec3 eye = center - dir * (radius * 2.0f);

      glm::vec3 up = (fabs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

      glm::mat4 view = glm::lookAt(eye, center, up);
      glm::mat4 proj = glm::ortho(-radius, radius, -radius, radius, 0.05f, radius * 4.0f);

      lightSpaceMatrix = proj * view;

    }

    //****************************************************************************//
    // setPoint - perspective light frustum aimed at the scene center
    //****************************************************************************//
    void setPoint(const glm::vec3 & position, const glm::vec3 & center, float radius) {

      if(radius <= 0.0f) radius = 1.0f;

      float dist = glm::length(center - position);
      if(dist < 1e-4f) dist = radius * 2.0f;

      glm::vec3 dir = (center - position) / glm::max(dist, 1e-4f);
      glm::vec3 up  = (fabs(dir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

      // Field of view that comfortably contains the scene sphere.
      float fov = 2.0f * atanf(radius / glm::max(dist, 1e-4f)) * 1.2f;
      fov = glm::clamp(fov, glm::radians(10.0f), glm::radians(120.0f));

      glm::mat4 view = glm::lookAt(position, center, up);
      glm::mat4 proj = glm::perspective(fov, 1.0f, glm::max(0.05f, dist - radius), dist + radius);

      lightSpaceMatrix = proj * view;

    }

    //****************************************************************************//
    // beginPass - bind the shadow framebuffer and start the depth-only pass
    //****************************************************************************//
    void beginPass() {

      if(isToInitInGpu()) setInGpu();

      glGetIntegerv(GL_VIEWPORT, savedViewport);

      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glViewport(0, 0, resolution, resolution);
      glClear(GL_DEPTH_BUFFER_BIT);

      // Cull front faces while filling the map to reduce shadow acne (peter-panning
      // is negligible for thin objects and is preferable to surface acne here).
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);

      depthShader.use();
      depthShader.setUniform("lightSpaceMatrix", lightSpaceMatrix);

    }

    //****************************************************************************//
    // endPass - restore the default framebuffer and viewport
    //****************************************************************************//
    void endPass() {

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);
      glCullFace(GL_BACK);

    }

    //****************************************************************************//
    // bind - upload the shadow map + light-space matrix to a color shader
    //****************************************************************************//
    void bind(const glShader & shader, int unit = 8) const {

      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(GL_TEXTURE_2D, depthTex);

      shader.setUniform("shadowMap", unit);
      shader.setUniform("lightSpaceMatrix", lightSpaceMatrix);
      shader.setUniform("useShadow", true);

      glActiveTexture(GL_TEXTURE0);

    }

    //****************************************************************************//
    // getDepthShader / getLightSpaceMatrix
    //****************************************************************************//
    glShader & getDepthShader() { return depthShader; }
    const glm::mat4 & getLightSpaceMatrix() const { return lightSpaceMatrix; }

  private:

    //****************************************************************************//
    // isToInitInGpu - (re)create the FBO on first use or after a context change
    //****************************************************************************//
    inline bool isToInitInGpu() {
      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) {
        isInitedInGpu = false;
        return true;
      }
      return false;
    }

    //****************************************************************************//
    // setInGpu - allocate the depth texture and the depth-only framebuffer
    //****************************************************************************//
    void setInGpu() {

      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;

      glGenTextures(1, &depthTex);
      glBindTexture(GL_TEXTURE_2D, depthTex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      // Outside the light frustum depth is 1.0 (fully lit), so nothing is in shadow there.
      GLfloat border[4] = {1.0f, 1.0f, 1.0f, 1.0f};
      glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

      glGenFramebuffers(1, &fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
      glDrawBuffer(GL_NONE);
      glReadBuffer(GL_NONE);

      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "ERROR [glShadow]: shadow framebuffer is not complete\n");
        abort();
      }

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      isInitedInGpu = true;

      glCheckError();

    }

    //****************************************************************************//
    // cleanInGpu
    //****************************************************************************//
    void cleanInGpu() {
      if(isInitedInGpu) {
        glDeleteTextures(1, &depthTex);
        glDeleteFramebuffers(1, &fbo);
        isInitedInGpu = false;
      }
    }

  }; /* class glShadow */

} /* namespace ogl */

#endif /* _H_OGL_GLSHADOW_H_ */
