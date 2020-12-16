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

#ifndef _H_OGL_GLLIGHT_H_
#define _H_OGL_GLLIGHT_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {
  
  //****************************************************************************//
  // glLight
  //****************************************************************************//
  class glLight {
    
  private:
    
    // contatore del numero di luci create
    static GLuint counter;
    
  public:
    
    // posizione della luce
    glm::vec3 position;
    
    // intensita della luce (colore)
    glm::vec3 intensities;
    
    // direzione della luce
    glm::vec3 direction;
    
    // attenuazione
    float attenuation;
    
    // ?
    float ambientCoefficient;
    
    // grandezza del cono
    float coneAngle;
    
    // direzione del cono
    glm::vec3 coneDirection;
    
    //****************************************************************************//
    // glLight
    //****************************************************************************//
    glLight() { counter++; }
    
    //****************************************************************************//
    // setPosition
    //****************************************************************************//
    void setPosition(const glm::vec3 & _position) { position = _position; }
    
    //****************************************************************************//
    // setDirection
    //****************************************************************************//
    void setDirection(const glm::vec3 & _direction) { direction = _direction; }
    
    //****************************************************************************//
    // setInShader
    //****************************************************************************//
    void setInShader(const ogl::glShader & shader, const glm::mat4 & view) const {
      
      glm::vec3 _position = glm::vec3(view * glm::vec4(position, 1));
      
      shader.setUniform("light.position", _position);
      shader.setUniform("light.direction", _position);
      shader.setUniform("light.ambient", glm::vec3(0.0f));
      shader.setUniform("light.diffuse", glm::vec3(1.0f));
      shader.setUniform("light.specular", glm::vec3(1.0f));
      
    }
    
  };
  
  GLuint glLight::counter = 0;
  
} /* namespace ogl */

#endif /* _H_OGL_GLLIGHT_H_ */
