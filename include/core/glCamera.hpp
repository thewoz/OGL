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

#ifndef _H_OGL_GLCAMERA_H_
#define _H_OGL_GLCAMERA_H_

#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

#include <cmath>
#include <utility>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // glCamera
  //****************************************************************************/
  // A single camera class that changes behaviour according to its current mode.
  // The mode is set at construction time and can be changed at any time via
  // setMode(). All setters are always active: the mode only decides how the
  // stored state is interpreted when building the view matrix or processing input.
  //
  // ── FLY ─────────────────────────────────────────────────────────────────────
  //   First-person free-fly. The camera moves through the world independently,
  //   like a drone. 'position' is the eye; 'front/right/up' are derived from
  //   yaw/pitch and change as you look around.
  //
  //   Mouse drag            → look around (yaw / pitch)
  //   Ctrl + mouse drag     → strafe (translate position)
  //   Arrow keys            → move forward / back / left / right
  //   Scroll wheel          → (no effect in FLY)
  //
  //   Best for: free exploration of a 3-D scene, first-person navigation.
  //
  // ── FIXED ────────────────────────────────────────────────────────────────────
  //   Locked look-at. The camera always points at 'target' using glm::lookAt;
  //   yaw/pitch/keyboard/mouse are all ignored. The view is entirely controlled
  //   by setting 'position' and 'target' explicitly in code.
  //
  //   All input devices       → ignored
  //
  //   Best for: cinematic shots, fixed inspection angles, offscreen rendering
  //             where the view must be set programmatically and must never drift.
  //
  // ── ORBIT ────────────────────────────────────────────────────────────────────
  //   Turntable orbit. The camera revolves around 'target' at a fixed radius
  //   (= |position − target|). Think of it as spinning a turntable under an
  //   object while the camera watches from above/beside.
  //
  //   Mouse drag            → orbit around target (yaw / pitch)
  //   Ctrl + mouse drag     → translate both camera and target (pan the scene)
  //   Shift + Arrow keys    → pan the scene (translate camera + target together)
  //   Scroll wheel          → dolly in/out (move camera along its radius)
  //
  //   IMPORTANT: position must differ from target, otherwise the orbit radius
  //   is zero and the scene appears frozen. A typical setup:
  //     camera.setTarget(0, 0, 0);
  //     camera.setPosition(0, 0, 5);  // radius = 5
  //
  //   Best for: inspecting a 3-D object from all sides (like a model viewer).
  //****************************************************************************/
  class glCamera {

  public:

    enum MODE     { FLY, FIXED, ORBIT };
    enum MOVEMENT { FORWARD, BACKWARD, LEFT, RIGHT };

  private:

    MODE mode;

    GLfloat speed          = 3.0f;  // keyboard translation speed (world units/s)
    GLfloat sensitivity    = 0.3f;  // mouse-look rotation (degrees per pixel)
    GLfloat panSensitivity = 1.0f;  // mouse pan translation (world units per pixel)

    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 front    = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right    = glm::vec3(1.0f, 0.0f,  0.0f);
    glm::vec3 up       = glm::vec3(0.0f, 1.0f,  0.0f);
    glm::vec3 worldUp  = glm::vec3(0.0f, 1.0f,  0.0f);

    // Euler angles, in degrees.
    GLfloat yaw   = 0.0f;
    GLfloat pitch = 0.0f;

    glm::vec3 target = glm::vec3(0.0f);

    glm::mat4 projection = glm::mat4(1.0f);

    GLsizei width  = 1;
    GLsizei height = 1;

    GLfloat fov   = 45.0f;
    GLfloat zNear = 0.1f;
    GLfloat zFar  = 10.0f;

  public:

    //****************************************************************************/
    // glCamera() - default: FLY mode, 1x1 viewport
    //****************************************************************************/
    glCamera() : mode(FLY) { updateBasis(); updateProjection(); }

    //****************************************************************************/
    // glCamera() - explicit construction
    //****************************************************************************/
    glCamera(MODE _mode, GLsizei _width, GLsizei _height, GLfloat _fov = 45.0f,
             const glm::vec3 & _position = glm::vec3(0.0f),
             const glm::vec3 & _target   = glm::vec3(0.0f))
      : mode(_mode), position(_position), target(_target),
        width(_width), height(_height), fov(_fov) {
      updateBasis();
      updateProjection();
    }

    //==========================================================================
    // Mode
    //==========================================================================

    inline MODE getMode() const { return mode; }

    //****************************************************************************/
    // setMode() - switch behaviour at runtime; all other state is preserved
    //****************************************************************************/
    inline void setMode(MODE _mode) { mode = _mode; }

    //==========================================================================
    // View matrix — built according to the current mode
    //==========================================================================

    glm::mat4 getView() const {
      switch(mode) {
        case FIXED:
          return glm::lookAt(position, target, up);
        case ORBIT: {
          glm::mat4 t1(1.0f); t1[3] = glm::vec4(target - position, 1.0f);
          glm::mat4 t2(1.0f); t2[3] = glm::vec4(-target, 1.0f);
          return t1 * eulerRotation(pitch, yaw, 0.0f) * t2;
        }
        case FLY:
        default:
          return glm::lookAt(position, position + front, up);
      }
    }

    //==========================================================================
    // Input handlers — FIXED ignores all input
    //==========================================================================

    void processKeyboard(MOVEMENT direction, GLfloat deltaTime) {
      GLfloat velocity = speed * deltaTime;
      switch(mode) {
        case FLY:
          if(direction == FORWARD)  position += front * velocity;
          if(direction == BACKWARD) position -= front * velocity;
          if(direction == LEFT)     position -= right * velocity;
          if(direction == RIGHT)    position += right * velocity;
          break;
        case ORBIT:
          if(direction == FORWARD)  position.y += velocity;
          if(direction == BACKWARD) position.y -= velocity;
          if(direction == LEFT)     position.x -= velocity;
          if(direction == RIGHT)    position.x += velocity;
          break;
        default: break;
      }
    }

    void processMouseMovement(GLfloat xOffset, GLfloat yOffset, bool mods, GLboolean constrainPitch = true) {
      if(mode == FIXED) return;
      if(!mods) {
        yaw   += xOffset * sensitivity;
        pitch += yOffset * sensitivity;
        if(constrainPitch) {
          if(pitch >  89.0f) pitch =  89.0f;
          if(pitch < -89.0f) pitch = -89.0f;
        }
        updateBasis();
      } else {
        updatePosition(xOffset * panSensitivity, yOffset * panSensitivity, 0.0f);
      }
    }

    void processMouseScroll(GLfloat yOffset) {
      if(mode == ORBIT) updatePosition(0.0f, 0.0f, yOffset);
    }

    //==========================================================================
    // Projection / viewport
    //==========================================================================

    inline glm::mat4 getProjection() const { return projection; }

    //****************************************************************************/
    // getOrthoProjection() - pixel-space ortho (0,0)-(width,height), for 2D overlays
    //****************************************************************************/
    inline glm::mat4 getOrthoProjection() const {
      return glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    }

    //****************************************************************************/
    // get3DOrthoProjection() - aspect-correct ortho in the [-1,1] cube
    //****************************************************************************/
    inline glm::mat4 get3DOrthoProjection() const {
      float aspect = static_cast<float>(width) / static_cast<float>(height);
      return glm::ortho(-aspect, aspect, -1.0f, 1.0f, zNear, zFar);
    }

    inline void setSensorSize(GLsizei _width, GLsizei _height) { setViewport(_width, _height); }

    inline void setViewport(GLsizei _width, GLsizei _height) {
      width  = _width;
      height = _height;
      updateProjection();
    }

    inline GLsizei getWidth()  const { return width;  }
    inline GLsizei getHeight() const { return height; }

    inline glm::vec2 getViewport() const { return glm::vec2(width, height); }
    inline void getViewport(int & _width, int & _height) const { _width = width; _height = height; }

    inline void setzNearFar(float _zNear, float _zFar) {
      zNear = _zNear;
      zFar  = _zFar;
      updateProjection();
    }

    inline float getzNear() const { return zNear; }
    inline float getzFar()  const { return zFar;  }

    inline void setFov(float _fov) { fov = _fov; updateProjection(); }

    //****************************************************************************/
    // getFOV() - vertical and horizontal field of view recovered from the matrix
    //****************************************************************************/
    std::pair<float,float> getFOV(bool inDegrees = false) const {

      float m00 = projection[0][0];
      float m11 = projection[1][1];

      float aspect = m11 / m00;

      float fovY = 2.0f * atan(1.0f / m11);
      float fovX = 2.0f * atan(tan(fovY / 2.0f) * aspect);

      if(inDegrees) {
        fovY = glm::degrees(fovY);
        fovX = glm::degrees(fovX);
      }

      return { fovY, fovX };

    }

    float getAspect() const {
      return projection[1][1] / projection[0][0];
    }

    //==========================================================================
    // Pose: position / orientation / target
    //==========================================================================

    inline void setPosition(const glm::vec3 & _position) { position = _position; }
    inline void setPosition(float x, float y, float z)   { position = glm::vec3(x, y, z); }

    inline void updatePosition(float dx, float dy, float dz) { position += glm::vec3(dx, dy, dz); }

    inline glm::vec3 getPosition() const { return position; }

    inline void setTarget(const glm::vec3 & _target) { target = _target; }
    inline void setTarget(float x, float y, float z)  { target = glm::vec3(x, y, z); }

    // Point the camera at _target from the current position, updating yaw/pitch.
    // In FLY mode this is the only way to aim the camera at a specific point,
    // because getView() uses position+front (not target). Calling setTarget()
    // alone has no effect on the view direction in FLY mode.
    void lookAt(const glm::vec3 & _target) {
      target = _target;
      glm::vec3 dir = glm::normalize(_target - position);
      pitch = glm::degrees(glm::asin(glm::clamp(dir.y, -1.0f, 1.0f)));
      yaw   = glm::degrees(glm::atan(dir.z, dir.x));
      updateBasis();
    }
    void lookAt(float x, float y, float z) { lookAt(glm::vec3(x, y, z)); }

    inline glm::vec3 getTarget() const { return target; }

    inline void setFront(const glm::vec3 & _front) {
      front = glm::normalize(_front);
      right = glm::normalize(glm::cross(front, worldUp));
      up    = glm::normalize(glm::cross(right, front));
    }

    inline glm::vec3 getFront() const { return front; }

    inline void setPitch(float _pitch) { pitch = _pitch; updateBasis(); }
    inline void setYaw(float _yaw)     { yaw   = _yaw;   updateBasis(); }

    inline float getPitch() const { return pitch; }
    inline float getYaw()   const { return yaw;   }

    //==========================================================================
    // Misc
    //==========================================================================

    inline void setKeybordSpeed(GLfloat _speed)           { speed = _speed; }
    inline void setMouseSensitivity(GLfloat _sensitivity) { sensitivity = _sensitivity; }
    inline void setPanSensitivity(GLfloat _sensitivity)   { panSensitivity = _sensitivity; }

    inline glm::mat4 getLookAt(const glm::vec3 & _target) const { return glm::lookAt(position, _target, up); }

    //****************************************************************************/
    // screenPosition() - project a world point to window pixels
    //****************************************************************************/
    bool screenPosition(const glm::vec3 & _coord, const glm::mat4 & model, glm::vec2 & screen) const {

      glm::vec4 coord = projection * getView() * model * glm::vec4(_coord, 1.0f);

      if(coord.w <= 0) return false;

      coord.x /= coord.w;
      coord.y /= coord.w;
      coord.z /= coord.w;

      screen.x = (coord.x + 1.0f) * width  * 0.5f;
      screen.y = (coord.y + 1.0f) * height * 0.5f;

      if(coord.z < -1.0f || coord.z > 1.0f) return false;

      return true;

    }

    //****************************************************************************/
    // computeCameraDistanceToFitObject()
    //****************************************************************************/
    float computeCameraDistanceToFitObject(float boundingRadius) const {

      std::pair<float,float> f = getFOV();

      float distVertical   = boundingRadius / sin(f.first  / 2.0f);
      float distHorizontal = boundingRadius / sin(f.second / 2.0f);

      return std::max(distVertical, distHorizontal);

    }

  private:

    void updateBasis() {

      glm::vec3 f;
      f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      f.y = sin(glm::radians(pitch));
      f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

      front = glm::normalize(f);
      right = glm::normalize(glm::cross(front, worldUp));
      up    = glm::normalize(glm::cross(right, front));

    }

    void updateProjection() {
      float aspect = (height != 0) ? width / static_cast<float>(height) : 1.0f;
      projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
    }

    glm::mat4 eulerRotation(float _pitch, float _yaw, float _roll) const {
      glm::mat4 RX = glm::rotate(glm::mat4(1.0f), glm::radians(_pitch), glm::vec3(1, 0, 0));
      glm::mat4 RY = glm::rotate(glm::mat4(1.0f), glm::radians(_yaw),   glm::vec3(0, 1, 0));
      glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), glm::radians(_roll),  glm::vec3(0, 0, 1));
      return RX * RY * RZ;
    }

  };

} /* namespace ogl */

#endif /* _H_OGL_GLCAMERA_H_ */
