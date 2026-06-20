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

#ifndef _H_OGL_SPHERE_H_
#define _H_OGL_SPHERE_H_

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
  // Class glSphere
  //****************************************************************************/
  // A UV-sphere is just an ellipsoid with three equal semi-axes, so glSphere is
  // a thin wrapper over glEllipse: it reuses the same geometry generation,
  // shading, SOLID/WIREFRAME handling and setLight() instead of duplicating
  // them. Supports two rendering styles:
  //   SOLID     — filled triangles with Phong shading
  //   WIREFRAME — triangle edges drawn as thick lines
  //****************************************************************************/
  class glSphere : public glEllipse {

  public:

    //****************************************************************************/
    // glSphere()
    //****************************************************************************/
    glSphere(const std::string & _name = "") : glEllipse(_name) { }

    //****************************************************************************/
    // glSphere()
    //****************************************************************************/
    glSphere(float _radius, int _slices, int _stacks, int _style = glShader::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_radius, _slices, _stacks, _style, _color);
    }

    //****************************************************************************/
    // init() - a sphere is an ellipsoid with equal semi-axes (a = b = c = radius)
    //****************************************************************************/
    void init(float _radius, int _slices, int _stacks, int _style = glShader::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0)) {

      DEBUG_LOG("glSphere::init(" + name + ")");

      glEllipse::init(_radius, _radius, _radius, _stacks, _slices, _style, _color);

    }

  };

} /* namespace ogl */

#endif /* _H_OGL_SPHERE_H_ */
