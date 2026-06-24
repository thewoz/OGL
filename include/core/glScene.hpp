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

#ifndef _H_OGL_GLSCENE_H_
#define _H_OGL_GLSCENE_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

#include <vector>

//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {

  //****************************************************************************//
  // glScene
  //****************************************************************************//
  // Holds the lighting shared by every drawable and the shadow map cast by the
  // primary light. Objects no longer own a light: they receive a glScene at
  // render time (render(camera, &scene)) and read the scene light from it.
  //
  // For now a single light is used (lights[0]); the std::vector is there so the
  // class can grow to several lights later without changing its public shape.
  // Shadows are produced only by the primary light (see glShadow).
  //****************************************************************************//
  class glScene {

  private:

    std::vector<glLight> lights;

    glShadow shadow;

    bool shadowsEnabled = false;

    // Bounding sphere of the shadow casters/receivers, used to frame the light.
    glm::vec3 sceneCenter = glm::vec3(0.0f);
    float     sceneRadius = 5.0f;

  public:

    //****************************************************************************//
    // glScene - starts with one default light (soft white)
    //****************************************************************************//
    glScene(GLsizei shadowResolution = 2048) : shadow(shadowResolution) {
      lights.emplace_back();
    }

    //****************************************************************************//
    // light - access a scene light (index 0 is the primary light)
    //****************************************************************************//
    glLight & light(std::size_t i = 0) { return lights[i]; }
    const glLight & light(std::size_t i = 0) const { return lights[i]; }

    std::size_t numLights() const { return lights.size(); }

    //****************************************************************************//
    // primary-light convenience setters
    //****************************************************************************//
    void setDirectionalLight(const glm::vec3 & direction) { lights[0].setDirection(direction); }
    void setPointLight(const glm::vec3 & position)        { lights[0].setPosition(position);   }

    void setAmbient (const glm::vec3 & a) { lights[0].setAmbient(a);  }
    void setDiffuse (const glm::vec3 & d) { lights[0].setDiffuse(d);  }
    void setSpecular(const glm::vec3 & s) { lights[0].setSpecular(s); }

    //****************************************************************************//
    // shadow controls
    //****************************************************************************//
    void enableShadows(bool on = true) { shadowsEnabled = on; }
    bool areShadowsEnabled() const     { return shadowsEnabled; }

    //****************************************************************************//
    // setBounds - bounding sphere used to frame the light for shadow mapping
    //****************************************************************************//
    void setBounds(const glm::vec3 & center, float radius) {
      sceneCenter = center;
      sceneRadius = radius;
    }

    //****************************************************************************//
    // beginShadowPass - update the light-space matrix and bind the shadow FBO.
    // Draw every caster with getShadowShader() between this and endShadowPass().
    //****************************************************************************//
    void beginShadowPass() {

      const glLight & L = lights[0];

      // A directional light wins over a positional one when both are set.
      if(L.hasDirectionSet() && glm::length(L.getDirection()) > 0.0f)
        shadow.setDirectional(L.getDirection(), sceneCenter, sceneRadius);
      else if(L.hasPositionSet())
        shadow.setPoint(L.getPosition(), sceneCenter, sceneRadius);
      else
        shadow.setDirectional(glm::vec3(-1.0f), sceneCenter, sceneRadius);

      shadow.beginPass();

    }

    //****************************************************************************//
    // getShadowShader / endShadowPass
    //****************************************************************************//
    glShader & getShadowShader() { return shadow.getDepthShader(); }

    void endShadowPass() { shadow.endPass(); }

    //****************************************************************************//
    // setInShader - upload the scene light (and the shadow map when enabled) to a
    // color shader. Called by each lit object's render() after shader.use().
    //****************************************************************************//
    void setInShader(const glShader & shader, const glm::mat4 & view) const {

      lights[0].setInShader(shader, view);

      if(shadowsEnabled) shadow.bind(shader);
      else               shader.setUniform("useShadow", false);

    }

  }; /* class glScene */

} /* namespace ogl */

#endif /* _H_OGL_GLSCENE_H_ */
