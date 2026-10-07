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
  // Intermediate base for the lit objects (glSphere, glEllipse, glCuboid,
  // glQuad and the glPoints impostors). On top of glObject (transform, shader,
  // style, lineWidth, color) it is the common type for Phong-shaded shapes.
  // Lighting is no longer owned per object: the scene light is supplied through
  // glScene at render time (render(camera, &scene)). Still abstract: subclasses
  // implement setInGpu()/cleanInGpu().
  //
  // What IS owned per object is the specular response, because it describes the
  // surface and not the light: 'shininess' is the Phong exponent (small = broad
  // dull highlight, large = small sharp one) and 'specularStrength' scales the
  // highlight, with 0 removing it for a fully matte surface. The defaults, 32
  // and 1, are the values these were hardcoded to before they were exposed.
  //
  // The highlight is deliberately NOT tinted by the object color: on the
  // dielectrics these shapes usually stand for (paper, cardboard, plastic,
  // painted surfaces) the specular reflection does not take the body color.
  // Tinting it is right only for metals, which are the business of glModel,
  // where the material carries a real specular color.
  //****************************************************************************/
  class glShape : public glObject {

  public:

    glShape(const std::string & _name = "") : glObject(_name) { }

    //****************************************************************************/
    // setShininess() - Phong exponent; values below 1 make no physical sense
    //****************************************************************************/
    inline void setShininess(float _shininess) { shininess = (_shininess > 1.0f) ? _shininess : 1.0f; }

    //****************************************************************************/
    // setSpecularStrength() - highlight intensity, 0 = fully matte
    //****************************************************************************/
    inline void setSpecularStrength(float _strength) { specularStrength = (_strength > 0.0f) ? _strength : 0.0f; }

    inline float getShininess()        const { return shininess;        }
    inline float getSpecularStrength() const { return specularStrength; }

    //****************************************************************************/
    // setUnlit() - draw the shape with its own color and no lighting at all, so
    // that it reads exactly as the given color from any side and with any light
    // (white comes out as full white). Only the SOLID style is affected.
    //****************************************************************************/
    inline void setUnlit(bool _unlit) { unlit = _unlit; }
    inline bool isUnlit() const { return unlit; }

  protected:

    float shininess        = 32.0f;
    float specularStrength = 1.0f;
    bool  unlit            = false;

  }; /* class glShape */

} /* namespace ogl */

#endif /* _H_OGL_SHAPE_H_ */
