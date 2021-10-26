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

#ifndef _H_OGL_OBJECT_H_
#define _H_OGL_OBJECT_H_

#include <cstdio>
#include <cstdlib>

#include <string>

//****************************************************************************
// namespace ogl
//****************************************************************************
namespace ogl {
  
  //****************************************************************************
  // Class glObject
  //****************************************************************************
  class glObject {
    
  protected:
    
    std::string name;
    
    uint32_t windowID;
    
    glShader shader;
    
    bool isInited;
    bool isInitedInGpu;
    bool isToUpdateInGpu;
    
    glm::vec3 _position;
    glm::vec3 _rotation;
    glm::vec3 _scale;
    
    glm::mat4 modelMatrix;
    
    int style;
    
  public:
    
    enum STYLE { WIREFRAME, SOLID, MODEL };
    
    //***************************************************************************
    // glObject()
    //***************************************************************************
    glObject(const std::string & _name = "") {
      
      name = _name;
      
      _position = glm::vec3(0.0);
      _rotation = glm::vec3(0.0);
      _scale    = glm::vec3(1.0);
      
      isInited = false;
      isInitedInGpu = false;
      isToUpdateInGpu = false;
      
      updateModelMatrix();
      
    }
    
    //****************************************************************************
    // initInGpu() -
    //****************************************************************************
    void initInGpu() {
      
      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;
      
      DEBUG_LOG("glObject::initInGpu(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glObject must be inited before set in GPU\n");
        abort();
      }
      
      setInGpu();
      
      isInitedInGpu = true;
      
    }
    
    //****************************************************************************
    // Position fuction
    //****************************************************************************
    inline void translate(const glm::vec3 & value) { _position = value; updateModelMatrix(); }
    inline void rotate   (const glm::vec3 & value) { _rotation = value; updateModelMatrix(); }
    inline void scale    (const glm::vec3 & value) { _scale    = value; updateModelMatrix(); }
    
    inline void move(const glm::vec3 & value1, const glm::vec3 & value2, const glm::vec3 & value3) {
      _position = value1;
      _rotation = value2;
      _scale    = value3;
      updateModelMatrix();
    }
    
    //****************************************************************************
    // getModelMatrix() -
    //****************************************************************************
    glm::mat4 getModelMatrix() const { return modelMatrix; }
    
    //****************************************************************************
    // getShader() -
    //****************************************************************************
    const glShader & getShader() const { return shader; }
    
    //****************************************************************************
    //
    //****************************************************************************
    glm::vec3 getTranslation() const { return _position; }
    glm::vec3 getRotation()    const { return _rotation; }
    
    //****************************************************************************
    // setStyle()
    //****************************************************************************
    inline void setStyle(int _style) { style = _style; }
    
    //****************************************************************************
    // setName()
    //****************************************************************************
    inline void setName(std::string _name) { name = _name; }  
    
  protected:
    
    //****************************************************************************
    // updateModelMatrix()
    //****************************************************************************
    inline void updateModelMatrix() {
      
      modelMatrix = glm::mat4(1.0f);
      
      modelMatrix = glm::translate(modelMatrix, _position); // Translate it down a bit so it's at the center of the scene
      
      modelMatrix = glm::scale(modelMatrix, _scale);

      modelMatrix  = glm::rotate(modelMatrix, _rotation.x, glm::vec3(1, 0, 0)); // where x, y, z is axis of rotation (e.g. 0 1 0)
      modelMatrix  = glm::rotate(modelMatrix, _rotation.y, glm::vec3(0, 1, 0)); // where x, y, z is axis of rotation (e.g. 0 1 0)
      modelMatrix  = glm::rotate(modelMatrix, _rotation.z, glm::vec3(0, 0, 1)); // where x, y, z is axis of rotation (e.g. 0 1 0)
      
    }
    
    //****************************************************************************
    // setInGpu
    //****************************************************************************
    virtual void setInGpu() = 0;
    
    //****************************************************************************
    // cleanInGpu
    //****************************************************************************
    virtual void cleanInGpu() = 0;
    
    //****************************************************************************
    // _setInGpu
    //****************************************************************************
    void _setInGpu() {
      
      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;
      
    }
    
    //****************************************************************************
    // isToInitInGpu
    //****************************************************************************
    inline bool isToInitInGpu() {
      
      DEBUG_LOG("glObject::isToInitInGpu(" + name + ")");
      
      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) {
        //cleanInGpu();
        isInitedInGpu = false;
        return true;
      }
      
      if(isToUpdateInGpu) {
        isToUpdateInGpu = false;
        return true;
      }
      
      return false;
      
    }
    
  }; /* class glObject */
  
} /* namespace ogl */

#endif /* _H_OGL_OBJECT_H_ */
