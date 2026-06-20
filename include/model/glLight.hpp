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

#ifndef _H_OGL_GLLIGHT_H_
#define _H_OGL_GLLIGHT_H_


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
  // glLight
  //****************************************************************************//
  // A single light source described by the classic Phong triple
  // (ambient/diffuse/specular) plus a geometry part.
  //
  // The geometry can describe two kinds of light, and the shaders pick which
  // one to use at runtime:
  //   - directional light: 'direction' is non-zero (e.g. the sun);
  //   - point light:       'direction' is zero and 'position' is used instead.
  // If both are zero the shaders fall back to a "head light" coming from the
  // camera, so an object is always visibly shaded even without an explicit
  // call to setLight().
  //****************************************************************************//
  class glLight {

  private:

    // Number of lights created so far (handy for debugging / future use).
    static GLuint counter;

    glm::vec3 position;   // world-space position  (used by point lights)
    glm::vec3 direction;  // world-space direction (used by directional lights)

    glm::vec3 ambient;    // ambient  contribution
    glm::vec3 diffuse;    // diffuse  contribution
    glm::vec3 specular;   // specular contribution

  public:

    //****************************************************************************//
    // glLight - default values give a soft white light
    //****************************************************************************//
    glLight() : position(0.0f), direction(0.0f), ambient(0.2f), diffuse(0.5f), specular(1.0f) {
      counter++;
    }

    //****************************************************************************//
    // setters
    //****************************************************************************//
    void setPosition(const glm::vec3& _position)   { position  = _position; }
    void setDirection(const glm::vec3& _direction) { direction = _direction; }
    void setAmbient(const glm::vec3& _ambient)     { ambient   = _ambient; }
    void setDiffuse(const glm::vec3& _diffuse)     { diffuse   = _diffuse; }
    void setSpecular(const glm::vec3& _specular)   { specular  = _specular; }

    //****************************************************************************//
    // setInShader - upload the light to the given shader
    //****************************************************************************//
    void setInShader(const ogl::glShader & shader, const glm::mat4 & view) const {

      // The shaders work in view space (the camera is at the origin there), so
      // the light position/direction are transformed by the view matrix to match
      // fragPos and fragNormal.
      glm::vec3 viewPos = glm::vec3(view * glm::vec4(position, 1.0f));
      glm::vec3 viewDir = glm::mat3(view) * direction;

      // Keep a null direction as zero: it tells the shader to use the point
      // light position (or the head-light fallback) instead of a directional one.
      if(glm::length(viewDir) > 0.0f) viewDir = glm::normalize(viewDir);

      shader.setUniform("light.position",  viewPos);
      shader.setUniform("light.direction", viewDir);
      shader.setUniform("light.ambient",   ambient);
      shader.setUniform("light.diffuse",   diffuse);
      shader.setUniform("light.specular",  specular);

    }

  };

  inline GLuint glLight::counter = 0;

} /* namespace ogl */

#endif /* _H_OGL_GLLIGHT_H_ */
