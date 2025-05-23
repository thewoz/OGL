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

#ifndef _H_OGL_GLCAMERA_H_
#define _H_OGL_GLCAMERA_H_

#include <cstdlib>
#include <cstdio>

#include <iostream>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glCamera
  //****************************************************************************/
  class glCamera {
    
  public:
    
    // Tipi di telecamera
    enum MODE { FREE, TARGET, BILLBOARD };
    
    // Tipi di movimento della camera
    enum MOVEMENT { FORWARD, BACKWARD, LEFT, RIGHT };

  private:
    
    // Tipologia della camera
    MODE mode;
    
    // Constanti
    GLfloat speed       = 10.0f;
    GLfloat sensitivity = 0.2f;

    // Camera Attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;
    
    // Eular Angles
    GLfloat yaw;
    GLfloat pitch;
    
    glm::vec3 target;
    
    glm::mat4 projection;
    
    GLsizei width;
    GLsizei height;
    
    GLfloat fov;
    GLfloat zNear;
    GLfloat zFar;
    
    
  public:
    
    //****************************************************************************/
    // glCamera() - Constructor of a easy camera
    //****************************************************************************/
    glCamera() {}
    
    //****************************************************************************/
    // glCamera() - Constructor of a easy camera
    //****************************************************************************/
    glCamera(GLsizei _width, GLsizei _height, glm::vec3 _position = glm::vec3(0.0f)) {
      init(_width, _height, 45.0f, _position);
    }
    
    //****************************************************************************/
    // glCamera() - Constructor of a full camera
    //****************************************************************************/
    glCamera(GLsizei _width, GLsizei _height, float _fov, glm::vec3 _position = glm::vec3(0.0f), MODE _mode = FREE, glm::vec3 _target = glm::vec3(0.0f)) {
      init(_width, _height, _fov, _position, _mode, _target);
    }
    
    //****************************************************************************/
    // init() - Inizializatore of a easy camera
    //****************************************************************************/
    void init(GLsizei _width, GLsizei _height, glm::vec3 _position = glm::vec3(0.0f)) { init(_width, _height, 45.0f, _position); }
    
    //****************************************************************************/
    // init() - Inizializatore di una camera
    //****************************************************************************/
    void init(GLsizei _width, GLsizei _height, float _fov, glm::vec3 _position = glm::vec3(0.0f), MODE _mode = FREE, glm::vec3 _target = glm::vec3(0.0f)) {
            
      position = _position;
      target   = _target;

      front   = glm::vec3(0.0f, 0.0f, -1.0f);
      worldUp = glm::vec3(0.0f, 1.0f,  0.0f);
      
      width  = _width;
      height = _height;
      
      yaw   = 0;
      pitch = 0;
      
      mode = _mode;
      
      fov = _fov;
      
      zNear = 0.1;
      zFar  = 10;
            
      projection = glm::perspective(glm::radians(fov), width/(float)height, zNear, zFar);
            
      updateCameraVectors();
      
    }
    
    //****************************************************************************/
    // processKeyboard() - Processa i tasti premuti
    //****************************************************************************/
    void processKeyboard(MOVEMENT direction, GLfloat deltaTime) {
      
      if(mode == FREE) {

        GLfloat velocity = speed * deltaTime;
        
        if(direction == FORWARD)  position += front * velocity;
        if(direction == BACKWARD) position -= front * velocity;
        if(direction == LEFT)     position -= right * velocity;
        if(direction == RIGHT)    position += right * velocity;
      
      }
      
      if(mode == BILLBOARD) {
        
        GLfloat velocity = speed * deltaTime;
        
        if(direction == FORWARD)  position[1] += velocity;
        if(direction == BACKWARD) position[1] -= velocity;
        if(direction == LEFT)     position[0] -= velocity;
        if(direction == RIGHT)    position[0] += velocity;
        
      }
            
    }
    
    //****************************************************************************/
    // processMouseMovement() - Processa il moviemento del mouse
    //****************************************************************************/
    void processMouseMovement(GLfloat xOffset, GLfloat yOffset, bool mods, GLboolean constrainPitch = true) {
            
      if(mode == FREE || mode == BILLBOARD) {

        if(!mods) {
        
          xOffset *= sensitivity;
          yOffset *= sensitivity;
        
          yaw   += xOffset;
          pitch += yOffset;
        
          // Make sure that when pitch is out of bounds, screen doesn't get flipped
          if(constrainPitch) {
            if(pitch >  89.0f) pitch =  89.0f;
            if(pitch < -89.0f) pitch = -89.0f;
          }
                
          // Update Front, Right and Up Vectors using the updated Eular angles
          updateCameraVectors();
                  
        } else {
          
          updatePosition(xOffset, yOffset, 0);
          
        }
        
      }
      
    }
    
    //****************************************************************************/
    // processMouseScroll() -
    //****************************************************************************/
    void processMouseScroll(GLfloat yOffset) {
      
      if(mode == BILLBOARD) updatePosition(0.0, 0.0, yOffset);
      
    }
    
    //****************************************************************************/
    // setSensorSize() - Setto la grandezza in pixel del sensore
    //****************************************************************************/
    void setSensorSize(GLsizei _width, GLsizei _height) { width = _width; height = _height; }
    
    //****************************************************************************/
    // getSensorSize() - Ritorno la grandezza in pixel del sensore
    //****************************************************************************/
    inline GLsizei getWidth()  const { return width;  }
    inline GLsizei getHeight() const { return height; }
    
    inline glm::vec2 getViewport() const { return glm::vec2(width,height); }
    
    inline void getViewport(int & _width, int & _height ) const { _width = width; _height = height; }

    inline void setViewport(int _width, int _height) {
      
      width = _width;
      height = _height;
      
      projection = glm::perspective(glm::radians(fov), width/(float)height, zNear, zFar);
      
    }

    //****************************************************************************/
    // setPosition() - Aggiorno la posizione della camera
    //****************************************************************************/
    void setPosition(const glm::vec3 & _position) { /*if(mode!=FREE)*/ position = _position; }
    void setPosition(float x, float y, float z)   { /*if(mode!=FREE)*/ position = glm::vec3(x,y,z); }

    //****************************************************************************/
    // updatePosition() - Aggiorno la posizione della camera
    //****************************************************************************/
    void updatePosition(float dx, float dy, float dz) { position.x += dx; position.y += dy; position.z += dz; }
    
    glm::vec3 getPosition() const { return position; }

    inline void setPitch(float _pitch) { pitch = _pitch; updateCameraVectors(); }
    inline void setYaw(float _yaw)     { yaw = _yaw;     updateCameraVectors(); }

    inline float getPitch() const { return pitch; }
    inline float getYaw()   const { return yaw;   }
    
    //****************************************************************************/
    // setTarget() - Aggiorno la posizione del target
    //****************************************************************************/
    void setTarget(const glm::vec3 & _target) { if(mode==TARGET || mode==BILLBOARD) target = _target; }
    void setTarget(float x, float y, float z) { if(mode==TARGET || mode==BILLBOARD) target = glm::vec3(x,y,z); }

    //****************************************************************************/
    // setFront() - Aggiorno la posizione del target
    //****************************************************************************/
    void setFront(const glm::vec3 & _front)   { if(mode==FREE) front = _front;   }
    
    //****************************************************************************/
    // Get projection matrixs
    //****************************************************************************/
    inline glm::mat4 getProjection() const { return projection; }
    
    //****************************************************************************/
    // Get view matrixs
    //****************************************************************************/
    inline glm::mat4 getView()       const {
      if(mode == FREE)   return glm::lookAt(position, position + front, up);
      if(mode == TARGET) return glm::lookAt(position, target          , up);
      if(mode == BILLBOARD) {
        glm::mat4 t1(1.0f); t1[3] = glm::vec4( target.x-position.x,  target.y-position.y,  target.z+position.z, 1.0f);
        glm::mat4 t2(1.0f); t2[3] = glm::vec4(-target.x, -target.y, -target.z           , 1.0f);
        glm::mat4 r = rotation(pitch, yaw, 0);
        return t1 * r * t2;
      }
      throw std::runtime_error("Invalid camera mode");
    }
    
    //****************************************************************************/
    // Get lookAt
    //****************************************************************************/
    inline glm::mat4 getLookAt(const glm::vec3 & _target) const { return glm::lookAt(position, _target , up); }
    
    //****************************************************************************/
    // getMode() - Camera mode
    //****************************************************************************/
    inline MODE getMode() const { return mode; }
    
    //****************************************************************************/
    // setMode() - Camera mode
    //****************************************************************************/
    inline void setMode(MODE _mode) { mode = _mode; }
    
    //****************************************************************************/
    // setSpeed() -
    //****************************************************************************/
    inline void setSpeed(GLfloat _speed) { speed = _speed; }
    
    //****************************************************************************/
    // setSensitivity() -
    //****************************************************************************/
    inline void setSensitivity(GLfloat _sensitivity) { sensitivity = _sensitivity; }
    
    //****************************************************************************/
    // setzNearFar() -
    //****************************************************************************/
    inline void setzNearFar(float _zNear, float _zFar) {
      
      zNear = _zNear;
      zFar  = _zFar;
      
      projection = glm::perspective(glm::radians(fov), width/(float)height, zNear, zFar);

    }
    
    //****************************************************************************/
    // setzNearFar()
    //****************************************************************************/
    inline float getzNear() { return zNear; }
    inline float getzFar()  { return zFar;  }

    //****************************************************************************/
    // get3DOrthoProjection()
    //****************************************************************************/
    inline glm::mat4 get3DOrthoProjection() const {
      float aspect = static_cast<float>(width) / static_cast<float>(height);
      return glm::ortho(-aspect, aspect, -1.0f, 1.0f, zNear, zFar);
    }
    
    //****************************************************************************/
    // getOrthoProjection()
    //****************************************************************************/
    inline glm::mat4 getOrthoProjection() const {
      return glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    }
    
    //****************************************************************************/
    // screenPosition()
    //****************************************************************************/
    bool screenPosition(const glm::vec3 & _coord, const glm::mat4 & model, glm::vec2 & screen) const {
            
      glm::vec4 coord = glm::vec4(_coord, 1);
      
      coord = projection * getView() * model * coord;
      
      if(coord.w <= 0) return false;
      
      coord.x /= coord.w;
      coord.y /= coord.w;
      coord.z /= coord.w;

      coord.x = (coord.x + 1) * width  * 0.5;
      coord.y = (coord.y + 1) * height * 0.5;
      
      screen.x = coord.x;
      screen.y = coord.y;

      if(coord.z < -1.0 || coord.z > 1.0) return false;
      
      return true;
      
    }
    
    //****************************************************************************/
    // getFOV()
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
      float m00 = projection[0][0];
      float m11 = projection[1][1];
      return m11 / m00;
    }
    
    //****************************************************************************/
    // computeCameraDistanceToFitObject()
    //****************************************************************************/
    // Calcola la distanza minima per far entrare l’oggetto nella vista
    float computeCameraDistanceToFitObject(float boundingRadius) const {
     
      std::pair<float,float> fov = getFOV();
      
      float distVertical   = boundingRadius / sin(fov.first  / 2.0f);
      float distHorizontal = boundingRadius / sin(fov.second / 2.0f);

      // Distanza finale
      return std::max(distVertical, distHorizontal);
      
    }
    
  private:
    
    //****************************************************************************/
    //updateCameraVectors
    //****************************************************************************/
    // Calculates the front vector from the Camera's (updated) Eular Angles
    void updateCameraVectors() {
      
      // Calculate the new Front vector
      glm::vec3 _front;
      
      _front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      _front.y = sin(glm::radians(pitch));
      _front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      
      front = glm::normalize(_front);
      
      // Also re-calculate the Right and Up vector
      right = glm::normalize(glm::cross(front, worldUp));
      
      // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
      up = glm::normalize(glm::cross(right, front));
      
    }
    
    //****************************************************************************//
    // rotation
    //****************************************************************************//
    glm::mat4 rotation(float _pitch, float _yaw, float _roll) const {
            
      glm::mat4 RX = glm::rotate(glm::mat4(1.0f), _pitch, glm::vec3(1,0,0));
      glm::mat4 RY = glm::rotate(glm::mat4(1.0f), _yaw,   glm::vec3(0,1,0));
      glm::mat4 RZ = glm::rotate(glm::mat4(1.0f), _roll,  glm::vec3(0,0,1));
      
      return RX * RY * RZ;
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLCAMERA_H_ */

