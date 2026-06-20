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

#ifndef _H_OGL_SHAPE_H_
#define _H_OGL_SHAPE_H_

#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glShape
  //****************************************************************************/
  // Intermediate base for the solid/wireframe primitives (glSphere, glEllipse,
  // glCuboid, glQuad). On top of glObject (transform, shader, style, lineWidth,
  // color) it adds the per-object light used for Phong shading in SOLID mode,
  // so the light handling is written once instead of being copied into every
  // shape. Still abstract: subclasses implement setInGpu()/cleanInGpu().
  //****************************************************************************/
  class glShape : public glObject {

  protected:

    ogl::glLight light;

  public:

    glShape(const std::string & _name = "") : glObject(_name) { }

    //****************************************************************************/
    // setLight() - Set the light position and direction
    //****************************************************************************/
    void setLight(const glm::vec3 & _position, const glm::vec3 & _direction) {
      light.setPosition(_position);
      light.setDirection(_direction);
    }

  }; /* class glShape */

} /* namespace ogl */

#endif /* _H_OGL_SHAPE_H_ */
