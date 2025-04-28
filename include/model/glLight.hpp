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
    
    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
  public:
    
    //****************************************************************************//
    // glLight
    //****************************************************************************//
    glLight() : position(0.0f), direction(0.0f, -1.0f, 0.0f), ambient(0.2f), diffuse(0.5f), specular(1.0f) {
      counter++;
    }
    
    //****************************************************************************//
    // set()
    //****************************************************************************//
    void setPosition(const glm::vec3& _position)   { position = _position; }
    void setDirection(const glm::vec3& _direction) { direction = _direction; }
    void setAmbient(const glm::vec3& _ambient)     { ambient = _ambient; }
    void setDiffuse(const glm::vec3& _diffuse)     { diffuse = _diffuse; }
    void setSpecular(const glm::vec3& _specular)   { specular = _specular; }
    
    //****************************************************************************//
    // setInShader
    //****************************************************************************//
    void setInShader(const ogl::glShader & shader, const glm::mat4 & view) const {
            
      glm::vec3 transformedPos = glm::vec3(view * glm::vec4(position, 1.0f));
      glm::vec3 transformedDir = glm::normalize(glm::mat3(view) * direction);

      shader.setUniform("light.position", transformedPos);
      shader.setUniform("light.direction", transformedDir);
      shader.setUniform("light.ambient", ambient);
      shader.setUniform("light.diffuse", diffuse);
      shader.setUniform("light.specular", specular);
      
    }
    
  };
  
  GLuint glLight::counter = 0;
  
} /* namespace ogl */

#endif /* _H_OGL_GLLIGHT_H_ */
