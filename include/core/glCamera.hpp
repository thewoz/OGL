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
  // glCamera - abstract camera
  //****************************************************************************/
  // A camera produces the projection and view matrices used to render a scene.
  //
  // The class is split in two parts:
  //   - everything that is mode-independent (the perspective projection, the
  //     position/orientation state, the viewport, and all the getters/setters)
  //     lives here in the base class;
  //   - the bits that actually differ between camera modes — how the view
  //     matrix is built and how mouse/keyboard input is interpreted — are
  //     virtual and implemented by the concrete subclasses:
  //
  //       glCameraFly      free-fly first-person camera (WASD/arrows + mouse look)
  //       glCameraOrbit    always looks at a fixed target point
  //       glCameraPan pans on the screen plane and zooms with the wheel
  //
  // The mode is therefore chosen by picking a subclass at construction time.
  // glWindow owns the cameras and can swap the active one's mode through
  // glWindow::updateCurrentCamera(). All the setters are always active: a setter
  // never silently does nothing because of the current mode — the mode only
  // decides how the stored state is interpreted by getView().
  //****************************************************************************/
  class glCamera {

  public:

    enum MODE     { FLY, ORBIT, PAN };
    enum MOVEMENT { FORWARD, BACKWARD, LEFT, RIGHT };

  protected:

    MODE mode;

    GLfloat speed       = 3.0f;
    GLfloat sensitivity = 0.3f;

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

    //****************************************************************************/
    // glCamera() - protected: only subclasses (which fix the mode) construct it
    //****************************************************************************/
    glCamera(MODE _mode, GLsizei _width, GLsizei _height, GLfloat _fov, const glm::vec3 & _position, const glm::vec3 & _target)
      : mode(_mode), position(_position), target(_target), width(_width), height(_height), fov(_fov) {
      updateBasis();
      updateProjection();
    }

  public:

    virtual ~glCamera() = default;

    //==========================================================================
    // Per-mode behavior (implemented by the subclasses)
    //==========================================================================

    //****************************************************************************/
    // getView() - the view matrix for this camera mode
    //****************************************************************************/
    virtual glm::mat4 getView() const = 0;

    //****************************************************************************/
    // input handlers - default to "do nothing"; modes override what they need
    //****************************************************************************/
    virtual void processKeyboard(MOVEMENT /*direction*/, GLfloat /*deltaTime*/) { }
    virtual void processMouseMovement(GLfloat /*xOffset*/, GLfloat /*yOffset*/, bool /*mods*/, GLboolean /*constrainPitch*/ = true) { }
    virtual void processMouseScroll(GLfloat /*yOffset*/) { }

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

    //****************************************************************************/
    // viewport (the camera "sensor" size, in pixels)
    //****************************************************************************/
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

    //****************************************************************************/
    // near/far planes and field of view
    //****************************************************************************/
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

    //****************************************************************************/
    // getAspect()
    //****************************************************************************/
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

    inline glm::vec3 getTarget() const { return target; }

    //****************************************************************************/
    // setFront() - override the look direction (keeps the basis consistent)
    //****************************************************************************/
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

    inline MODE getMode() const { return mode; }

    inline void setKeybordSpeed(GLfloat _speed)          { speed = _speed; }
    inline void setMouseSensitivity(GLfloat _sensitivity) { sensitivity = _sensitivity; }

    //****************************************************************************/
    // getLookAt() - view matrix that looks at an arbitrary point from here
    //****************************************************************************/
    inline glm::mat4 getLookAt(const glm::vec3 & _target) const { return glm::lookAt(position, _target, up); }

    //****************************************************************************/
    // screenPosition() - project a world point to window pixels.
    // Returns false (and leaves 'screen' at the projected xy) when the point is
    // behind the camera or outside the depth range.
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
    // computeCameraDistanceToFitObject() - distance at which a bounding sphere
    // of the given radius exactly fills the frustum.
    //****************************************************************************/
    float computeCameraDistanceToFitObject(float boundingRadius) const {

      std::pair<float,float> f = getFOV();

      float distVertical   = boundingRadius / sin(f.first  / 2.0f);
      float distHorizontal = boundingRadius / sin(f.second / 2.0f);

      return std::max(distVertical, distHorizontal);

    }

  protected:

    //****************************************************************************/
    // updateBasis() - rebuild front/right/up from the Euler angles (degrees)
    //****************************************************************************/
    void updateBasis() {

      glm::vec3 f;
      f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      f.y = sin(glm::radians(pitch));
      f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

      front = glm::normalize(f);
      right = glm::normalize(glm::cross(front, worldUp));
      up    = glm::normalize(glm::cross(right, front));

    }

    //****************************************************************************/
    // updateProjection() - rebuild the perspective matrix
    //****************************************************************************/
    void updateProjection() {
      float aspect = (height != 0) ? width / static_cast<float>(height) : 1.0f;
      projection = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
    }

    //****************************************************************************/
    // eulerRotation() - rotation matrix from pitch/yaw/roll (degrees)
    //****************************************************************************/
    glm::mat4 eulerRotation(float _pitch, float _yaw, float _roll) const {
      glm::mat4 RX = glm::rotate(glm::mat4(1.0f), glm::radians(_pitch), glm::vec3(1, 0, 0));
      glm::mat4 RY = glm::rotate(glm::mat4(1.0f), glm::radians(_yaw),   glm::vec3(0, 1, 0));
      glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), glm::radians(_roll),  glm::vec3(0, 0, 1));
      return RX * RY * RZ;
    }

  };

  //****************************************************************************/
  // glCameraFly - free-fly first-person camera
  //****************************************************************************/
  // Moves along its own axes (WASD / arrow keys) and looks around with the
  // mouse. Holding the modifier key pans the camera on the screen plane instead
  // of rotating it.
  //****************************************************************************/
  class glCameraFly : public glCamera {

  public:

    glCameraFly(GLsizei _width, GLsizei _height, GLfloat _fov = 45.0f, const glm::vec3 & _position = glm::vec3(0.0f))
      : glCamera(FLY, _width, _height, _fov, _position, glm::vec3(0.0f)) { }

    glm::mat4 getView() const override {
      return glm::lookAt(position, position + front, up);
    }

    void processKeyboard(MOVEMENT direction, GLfloat deltaTime) override {
      GLfloat velocity = speed * deltaTime;
      if(direction == FORWARD)  position += front * velocity;
      if(direction == BACKWARD) position -= front * velocity;
      if(direction == LEFT)     position -= right * velocity;
      if(direction == RIGHT)    position += right * velocity;
    }

    void processMouseMovement(GLfloat xOffset, GLfloat yOffset, bool mods, GLboolean constrainPitch = true) override {

      if(!mods) {

        yaw   += xOffset * sensitivity;
        pitch += yOffset * sensitivity;

        // Keep the camera from flipping over at the poles.
        if(constrainPitch) {
          if(pitch >  89.0f) pitch =  89.0f;
          if(pitch < -89.0f) pitch = -89.0f;
        }

        updateBasis();

      } else {
        // Modifier held: pan on the screen plane instead of rotating.
        updatePosition(xOffset, yOffset, 0.0f);
      }

    }

  };

  //****************************************************************************/
  // glCameraOrbit - always looks at a fixed target point
  //****************************************************************************/
  // The view direction is determined entirely by position and target, so this
  // camera ignores mouse/keyboard input (move it through setPosition/setTarget).
  //****************************************************************************/
  class glCameraOrbit : public glCamera {

  public:

    glCameraOrbit(GLsizei _width, GLsizei _height, GLfloat _fov = 45.0f, const glm::vec3 & _position = glm::vec3(0.0f), const glm::vec3 & _target = glm::vec3(0.0f))
      : glCamera(ORBIT, _width, _height, _fov, _position, _target) { }

    glm::mat4 getView() const override {
      return glm::lookAt(position, target, up);
    }

  };

  //****************************************************************************/
  // glCameraPan - screen-plane pan + wheel zoom
  //****************************************************************************/
  // The view orbits a target: the mouse rotates it (or pans with the modifier),
  // the arrow keys pan it on the XY plane and the scroll wheel dollies along Z.
  //****************************************************************************/
  class glCameraPan : public glCamera {

  public:

    glCameraPan(GLsizei _width, GLsizei _height, GLfloat _fov = 45.0f, const glm::vec3 & _position = glm::vec3(0.0f), const glm::vec3 & _target = glm::vec3(0.0f))
      : glCamera(PAN, _width, _height, _fov, _position, _target) { }

    glm::mat4 getView() const override {
      glm::mat4 t1(1.0f); t1[3] = glm::vec4(target - position, 1.0f);
      glm::mat4 t2(1.0f); t2[3] = glm::vec4(-target, 1.0f);
      glm::mat4 r = eulerRotation(pitch, yaw, 0.0f);
      return t1 * r * t2;
    }

    void processKeyboard(MOVEMENT direction, GLfloat deltaTime) override {
      GLfloat velocity = speed * deltaTime;
      if(direction == FORWARD)  position.y += velocity;
      if(direction == BACKWARD) position.y -= velocity;
      if(direction == LEFT)     position.x -= velocity;
      if(direction == RIGHT)    position.x += velocity;
    }

    void processMouseMovement(GLfloat xOffset, GLfloat yOffset, bool mods, GLboolean constrainPitch = true) override {

      if(!mods) {

        yaw   += xOffset * sensitivity;
        pitch += yOffset * sensitivity;

        if(constrainPitch) {
          if(pitch >  89.0f) pitch =  89.0f;
          if(pitch < -89.0f) pitch = -89.0f;
        }

        updateBasis();

      } else {
        updatePosition(xOffset, yOffset, 0.0f);
      }

    }

    void processMouseScroll(GLfloat yOffset) override {
      updatePosition(0.0f, 0.0f, yOffset);
    }

  };

} /* namespace ogl */

#endif /* _H_OGL_GLCAMERA_H_ */
