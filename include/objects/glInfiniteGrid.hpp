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

#ifndef _H_OGL_INFINITEGRID_H_
#define _H_OGL_INFINITEGRID_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

//*****************************************************************************/
// namespace ogl
//*****************************************************************************/
namespace ogl {

  //*****************************************************************************/
  // Class glInfiniteGrid
  //*****************************************************************************/
  // A grid on a horizontal plane that has no edges: it keeps going to the
  // horizon in every direction, like the floor grid of a 3D editor, and fades
  // out with the distance. Unlike glGrid, which is a finite mesh of lines with
  // a fixed number of cells, this one is a single quad that follows the camera
  // and an antialiased pattern computed in the fragment shader, so:
  //
  //   - the cell size is a free parameter, changeable at any time and cheap
  //     (setCellSize() only sets a uniform, nothing is rebuilt);
  //   - the lines have a constant width in pixels (setLineWidth()) at any
  //     distance and viewing angle, with clean antialiasing;
  //   - cells that get smaller than a few pixels fade out instead of aliasing
  //     into moire, so the grid thins out toward the horizon.
  //
  // The lines are at the multiples of the cell size in world coordinates, so
  // they land on the same values as the ticks of a glPlot with the same step.
  //
  // The plane is horizontal (XZ) and its height is the Y of translate(): X and
  // Z are ignored, the plane is infinite. It is drawn translucent and does not
  // write depth, so it never hides what is behind it, and it is pushed slightly
  // back in depth so that a surface lying exactly on the plane (the bottom of a
  // box standing on it) wins over the lines instead of z-fighting with them. It
  // is drawn from both sides. Draw it before the objects that should appear in
  // front of it, and it will be occluded by the ones already drawn.
  //
  // The fade distance defaults to 90% of the camera far plane, so the grid
  // always reaches as far as the camera can see; setFadeDistance() overrides it.
  //*****************************************************************************/
  class glInfiniteGrid : public glObject {

  private:

    // an empty vertex array: the quad is built in the vertex shader from
    // gl_VertexID, but the core profile still needs one bound to draw
    GLuint vao = 0;

    float cellSize     = 1.0f;
    float fadeDistance = 0.0f;   // 0 = automatic, from the camera far plane

  public:

    //****************************************************************************/
    // glInfiniteGrid()
    //****************************************************************************/
    glInfiniteGrid(const std::string & _name = "") { name = _name; }

    glInfiniteGrid(float _cellSize, const glm::vec3 & _color = glm::vec3(0.5f), const std::string & _name = "") {
      name = _name;
      init(_cellSize, _color);
    }

    //****************************************************************************/
    // ~glInfiniteGrid()
    //****************************************************************************/
    ~glInfiniteGrid() { cleanInGpu(); }

    glInfiniteGrid(glInfiniteGrid &&) noexcept = default;
    glInfiniteGrid & operator = (glInfiniteGrid &&) noexcept = default;

    //****************************************************************************/
    // init() - compiles the shader, so it is done once. Everything else can be
    // changed later at no cost
    //****************************************************************************/
    void init(float _cellSize = 1.0f, const glm::vec3 & _color = glm::vec3(0.5f)) {

      DEBUG_LOG("glInfiniteGrid::init(" + name + ")");

      shader.setName(name);

      shader.initGrid();

      setCellSize(_cellSize);

      color = _color;

      isInited = true;
      isToUpdateInGpu = true; // re-init after a render must re-upload

    }

    //****************************************************************************/
    // setCellSize() - side of a cell, in world units. A value that is not
    // positive (or NaN) is ignored, since a zero or negative cell has no meaning
    // and the shader would divide by it
    //****************************************************************************/
    void setCellSize(float _cellSize) { if(_cellSize > 0.0f) cellSize = _cellSize; }

    float getCellSize() const { return cellSize; }

    //****************************************************************************/
    // setHeight() - the Y of the plane. Same as translate() with the current X
    // and Z, which do not matter
    //****************************************************************************/
    void setHeight(float _height) { translate(glm::vec3(_position.x, _height, _position.z)); }

    float getHeight() const { return _position.y; }

    //****************************************************************************/
    // setFadeDistance() - distance from the eye at which the grid is gone. 0, the
    // default, means 90% of the camera far plane
    //****************************************************************************/
    void setFadeDistance(float _fadeDistance) { fadeDistance = (_fadeDistance > 0.0f) ? _fadeDistance : 0.0f; }

    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {

      DEBUG_LOG("glInfiniteGrid::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "ERROR [glInfiniteGrid]: must be initialized before rendering\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      glm::mat4 view = camera.getView();

      // The eye position comes from the view matrix, not from camera.getPosition():
      // in ORBIT mode the position is not where the eye is (the view is built
      // around the target), while the inverse of the view always is.
      glm::vec3 eye = glm::vec3(glm::inverse(view) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

      float fadeEnd = (fadeDistance > 0.0f) ? fadeDistance : 0.9f * camera.getzFar();

      shader.use();

      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       view);
      shader.setUniform("gridCenter", glm::vec3(eye.x, _position.y, eye.z));
      // half side of the quad: every point within fadeEnd of the eye has a
      // horizontal offset below it, so the quad covers everything that is visible
      shader.setUniform("halfSize",   fadeEnd);
      shader.setUniform("eye",        eye);
      shader.setUniform("cellSize",   cellSize);
      shader.setUniform("lineWidth",  lineWidth);
      shader.setUniform("color",      color);
      shader.setUniform("fadeStart",  0.35f * fadeEnd);
      shader.setUniform("fadeEnd",    fadeEnd);
      shader.setUniform("zNear",      camera.getzNear());

      // the state this draw changes, to put it back as it was
      GLboolean blend = glIsEnabled(GL_BLEND);
      GLboolean cull  = glIsEnabled(GL_CULL_FACE);
      GLboolean depthMask = GL_TRUE;
      glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      // no depth write: the grid is a reference and must not hide what is drawn
      // after it. Its depth is still tested, and grid.fs pushes it slightly away
      // from the eye so that a surface on the floor wins over it
      glDepthMask(GL_FALSE);
      glDisable(GL_CULL_FACE);

      glBindVertexArray(vao);

      glDrawArrays(GL_TRIANGLES, 0, 6);

      glBindVertexArray(0);

      if(!blend) glDisable(GL_BLEND);
      if(cull)   glEnable(GL_CULL_FACE);
      glDepthMask(depthMask);

      glCheckError();

    }

  private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glInfiniteGrid::setInGpu(" + name + ")");

      // Same-context re-upload: drop the old array first (no-op after a context
      // change, where isInitedInGpu is already false).
      cleanInGpu();

      glGenVertexArrays(1, &vao);

      glCheckError();

    }

    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() override {

      if(isInitedInGpu) {

        glDeleteVertexArrays(1, &vao);

        isInitedInGpu = false;

      }

    }

  }; /* class glInfiniteGrid */

} /* namespace ogl */

#endif /* _H_OGL_INFINITEGRID_H_ */
