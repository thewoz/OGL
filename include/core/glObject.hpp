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

#ifndef _H_OGL_OBJECT_H_
#define _H_OGL_OBJECT_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

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
  // Base class for every drawable. It holds the transform (TRS), a shader, and
  // the bookkeeping flags that drive the lazy GPU upload pattern:
  //   - isInited       : CPU-side data is ready (set by the subclass init/ctor)
  //   - isInitedInGpu  : GPU buffers are uploaded for the current context
  //   - isToUpdateInGpu: data changed, buffers must be re-uploaded next frame
  //
  // On the first render() call, isToInitInGpu() returns true and initInGpu()
  // is called, which in turn calls the virtual setInGpu() implemented by each
  // subclass. This keeps constructors fast and makes objects safe to create
  // before a window/context exists.
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
    float lineWidth;

    // Single solid color, shared by every drawable that needs one. Per-vertex
    // color objects (glLines, glPoints, glPlot) keep their own color buffers.
    glm::vec3 color;

  public:

    //***************************************************************************
    // glObject()
    //***************************************************************************
    glObject(const std::string & _name = "") {

      name = _name;

      _position = glm::vec3(0.0);
      _rotation = glm::vec3(0.0);
      _scale    = glm::vec3(1.0);

      windowID = 0;

      isInited        = false;
      isInitedInGpu   = false;
      isToUpdateInGpu = false;

      style     = 0;
      lineWidth = 1.0f;

      color = glm::vec3(1.0f);

      updateModelMatrix();

    }

    //***************************************************************************
    // glObject owns GPU resources through its subclasses (vao/vbo/program), so
    // copying would duplicate the handles and double-free them on destruction.
    // Drawables are therefore non-copyable and non-movable; pass them by
    // reference or pointer. The virtual destructor makes deletion through a
    // glObject* well defined.
    //***************************************************************************
    virtual ~glObject() { }

    glObject(const glObject &) = delete;
    glObject & operator = (const glObject &) = delete;

    //***************************************************************************
    // Move is allowed and transfers ownership of the GPU resources. The base
    // move resets the moved-from object's bookkeeping flags so that its
    // destructor (and any later cleanInGpu()) becomes a no-op: this is what
    // makes a default-generated move in the subclasses safe, since the freed
    // source will never delete the handles now owned by the destination.
    //***************************************************************************
    glObject(glObject && o) noexcept
      : name(std::move(o.name)), windowID(o.windowID), shader(std::move(o.shader)),
        isInited(o.isInited), isInitedInGpu(o.isInitedInGpu), isToUpdateInGpu(o.isToUpdateInGpu),
        _position(o._position), _rotation(o._rotation), _scale(o._scale),
        modelMatrix(o.modelMatrix), style(o.style), lineWidth(o.lineWidth), color(o.color) {
      o.isInited        = false;
      o.isInitedInGpu   = false;
      o.isToUpdateInGpu = false;
      o.windowID        = 0;
    }

    glObject & operator = (glObject && o) noexcept {
      if(this != &o) {
        name            = std::move(o.name);
        windowID        = o.windowID;
        shader          = std::move(o.shader);
        isInited        = o.isInited;
        isInitedInGpu   = o.isInitedInGpu;
        isToUpdateInGpu = o.isToUpdateInGpu;
        _position       = o._position;
        _rotation       = o._rotation;
        _scale          = o._scale;
        modelMatrix     = o.modelMatrix;
        style           = o.style;
        lineWidth       = o.lineWidth;
        color           = o.color;
        o.isInited        = false;
        o.isInitedInGpu   = false;
        o.isToUpdateInGpu = false;
        o.windowID        = 0;
      }
      return *this;
    }
    
    //****************************************************************************
    // initInGpu() -
    //****************************************************************************
    void initInGpu() {
      
      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;
      
      DEBUG_LOG("glObject::initInGpu(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "ERROR [glObject]: must be initialized before uploading to GPU\n");
        abort();
      }
      
      setInGpu();
      
      isInitedInGpu = true;
      
    }
    
    //****************************************************************************
    // renderDepth() - draw this object's geometry into the scene shadow map,
    // using the scene's depth-only shader. Default is a no-op: only the solid
    // shadow casters (model, cuboid, quad, sphere, ellipse) override it. Lines,
    // points, text and axes do not cast meaningful shadows.
    //****************************************************************************
    virtual void renderDepth(const glShader & /*depthShader*/) { }

    //****************************************************************************
    // Position fuction
    //****************************************************************************
    virtual void translate(const glm::vec3 & value) { _position = value; updateModelMatrix(); }
    inline  void rotate   (const glm::vec3 & value) { _rotation = value; updateModelMatrix(); }
    inline  void scale    (const glm::vec3 & value) { _scale    = value; updateModelMatrix(); }
    
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
    // setColor() / getColor()
    //****************************************************************************
    inline void setColor(const glm::vec3 & _color) { color = _color; }
    inline glm::vec3 getColor() const { return color; }

    //****************************************************************************
    // setLineWidth()
    //****************************************************************************
    inline void setLineWidth(float _lineWidth) { lineWidth = (_lineWidth > 0.0f) ? _lineWidth : 1.0f; }
    
    //****************************************************************************
    // setName()
    //****************************************************************************
    inline void setName(std::string _name) { name = _name; }  
    
  protected:
    
    //****************************************************************************
    // updateModelMatrix - rebuild M = T * Rx * Ry * Rz * S.
    // Called every time position/rotation/scale changes. Scale is applied first
    // (in object space) and rotation after it, so a non-uniform scale on a
    // rotated object stretches along the object's own axes instead of shearing.
    //****************************************************************************
    inline void updateModelMatrix() {

      modelMatrix = glm::mat4(1.0f);
      modelMatrix = glm::translate(modelMatrix, _position);
      modelMatrix = glm::rotate   (modelMatrix, _rotation.x, glm::vec3(1, 0, 0));
      modelMatrix = glm::rotate   (modelMatrix, _rotation.y, glm::vec3(0, 1, 0));
      modelMatrix = glm::rotate   (modelMatrix, _rotation.z, glm::vec3(0, 0, 1));
      modelMatrix = glm::scale    (modelMatrix, _scale);

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
    // isToInitInGpu - returns true when GPU buffers need to be (re-)uploaded.
    // This happens on the very first render, when the context changes (different
    // window), or when the caller sets isToUpdateInGpu after a data change.
    //****************************************************************************
    inline bool isToInitInGpu() {

      DEBUG_LOG("glObject::isToInitInGpu(" + name + ")");

      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) {
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
