/*
 * MIT License
 *
 * Copyright © 2017
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _H_OGL_OBJECT_H_
#define _H_OGL_OBJECT_H_

#include <cstdio>
#include <cstdlib>

#include <string>

/* ****************************************************************************/
// namespace ogl
/* ****************************************************************************/
namespace ogl {

  /* ****************************************************************************/
  // Class glObject
  /* ****************************************************************************/
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
      
    //  glm::vec3 color;
      
      int style;
      
    public:
    
    enum STYLE { WIREFRAME, SOLID, MODEL };
    
    //***************************************************************************
    // glObject()
    //***************************************************************************
    glObject() {
    
      name = "glObject";
      
      _position = glm::vec3(0.0);
      _rotation = glm::vec3(0.0);
      _scale    = glm::vec3(1.0);
    
      updateModelMatrix();
      
    }
    
//     //***************************************************************************
//     // render()
//     //***************************************************************************
//     void render(const glCamera * camera, int from = 0, int to = -1) {
//
//       DEBUG_LOG("glObject::render(" + name + ")");
//
//       if(!isInited){
//         fprintf(stderr, "line must be inited before render\n");
//         abort();
//       }
//
//       if(isToInitInGpu()) initInGpu();
//
//       shader.use();
//
//       _render(camera);
//
//     }
       
//
//      /* ****************************************************************************/
//      // glObject() -
//      /* ****************************************************************************/
//      glObject(const std::string & _name = "") : isInited(false), isInitedInGpu(false), isToUpdateInGpu(false), name(_name) {
//
//        _init(glm::vec3(0.0), glm::vec3(0.0), glm::vec3(1.0));
//
//      }
//
//      /* ****************************************************************************/
//      // initModel() -
//      /* ****************************************************************************/
//      void initModel(const glm::vec3 & _center = glm::vec3(0.0), const glm::vec3 & _angles = glm::vec3(0.0), const glm::vec3 & _size = glm::vec3(1.0), const std::string & _name = "") {
//
//        if(name.empty()) name = _name;
//
//        DEBUG_LOG("glObject::init(" + name + ")");
//
//        shader.setName(name);
//
//        shader.initModel();
//
//        _init(_center, _angles, _size);
//
//      }
//
//      /* ****************************************************************************/
//      // initPlain() -
//      /* ****************************************************************************/
//      void initPlain(const glm::vec3 & _center = glm::vec3(0.0), const glm::vec3 & _angles = glm::vec3(0.0), const glm::vec3 & _size = glm::vec3(1.0), const std::string & _name = "") {
//
//        if(name.empty()) name = _name;
//
//        DEBUG_LOG("glObject::init(" + name + ")");
//
//        shader.setName(name);
//
//        shader.initPlain();
//
//        _init(_center, _angles, _size);
//
//      }
//
//    /* ****************************************************************************/
//    // initOrto() -
//    /* ****************************************************************************/
//    void initOrto(const std::string & _name = "") {
//
//      if(name.empty()) name = _name;
//
//      DEBUG_LOG("glObject::init(" + name + ")");
//
//      shader.setName(name);
//
//      shader.initOrto();
//
//      _init();
//
//    }
//
//    /* ****************************************************************************/
//    // initAdvanced() -
//    /* ****************************************************************************/
//    void initAdvanced(const glm::vec3 & _center = glm::vec3(0.0), const glm::vec3 & _angles = glm::vec3(0.0), const glm::vec3 & _size = glm::vec3(1.0), const std::string & _name = "") {
//
//      if(name.empty()) name = _name;
//
//      DEBUG_LOG("glObject::init(" + name + ")");
//
//      shader.setName(name);
//
//      shader.initAdvanced();
//
//      _init(_center, _angles, _size);
//
//    }
//
//    /* ****************************************************************************/
//    // initText() -
//    /* ****************************************************************************/
//    void initText(const std::string & _name = "") {
//
//      if(name.empty()) name = _name;
//
//      DEBUG_LOG("glObject::init(" + name + ")");
//
//      shader.setName(name);
//
//      shader.initText();
//
//      _init();
//
//    }
//
//    /* ****************************************************************************/
//    // initText3D() -
//    /* ****************************************************************************/
//    void initText3D(const std::string & _name = "") {
//
//      if(name.empty()) name = _name;
//
//      DEBUG_LOG("glObject::init(" + name + ")");
//
//      shader.setName(name);
//
//      shader.initText3D();
//
//      _init();
//
//    }
//
//
//      /* ****************************************************************************/
//      // initSphere() -
//      /* ****************************************************************************/
//      void initSphere(const glm::vec3 & _center = glm::vec3(0.0), const glm::vec3 & _angles = glm::vec3(0.0), const glm::vec3 & _size = glm::vec3(1.0), const std::string & _name = "") {
//
//        if(name.empty()) name = _name;
//
//        DEBUG_LOG("glObject::init(" + name + ")");
//
//        shader.setName(name);
//
//        shader.initSphere();
//
//        _init(_center, _angles, _size);
//
//      }
//
//      /* ****************************************************************************/
//      // renderBegin() -
//      /* ****************************************************************************/
//      void renderBegin(const glm::mat4 & projection, const glm::mat4 & view) {
//
//        DEBUG_LOG("glObject::renderBegin(" + name + ")");
//
//        if(!isInited){
//          fprintf(stderr, "line must be inited before render\n");
//          abort();
//        }
//
//        if(isToInitInGpu()) initInGpu();
//
//        shader.use();
//
//        shader.setUniform("projection", projection);
//        shader.setUniform("view", view);
//        shader.setUniform("model", model);
//        shader.setUniform("color", color);
//
//        //if(shader.style != glShader::STYLE::ORTO)  shader.setUniform("model", model);
//        //if(shader.style != glShader::STYLE::MODEL) shader.setUniform("color", color);
//
//        glEnable(GL_DEPTH_TEST);
//
//      }
//
//    /* ****************************************************************************/
//    // renderBegin() -
//    /* ****************************************************************************/
//    void renderBegin(const glm::mat4 & projection) {
//
//      DEBUG_LOG("glObject::renderBegin(" + name + ")");
//
//      if(!isInited){
//        fprintf(stderr, "line must be inited before render\n");
//        abort();
//      }
//
//      if(isToInitInGpu()) initInGpu();
//
//      shader.use();
//
//      shader.setUniform("projection", projection);
//      shader.setUniform("color", color);
//
//      glEnable(GL_DEPTH_TEST);
//
//    }
//
//      /* ****************************************************************************/
//      // renderEnd() -
//      /* ****************************************************************************/
//      void renderEnd() {
//
//        glDisable(GL_DEPTH_TEST);
//
//      }
      
      //virtual void renderEnd() = 0;
    

      /* ****************************************************************************/
      // initInGpu() -
      /* ****************************************************************************/
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
      
      /* ****************************************************************************/
      // Position fuction
      /* ****************************************************************************/
      inline void translate(const glm::vec3 & value) { _position = value; updateModelMatrix(); }
      inline void rotate   (const glm::vec3 & value) { _rotation = value; updateModelMatrix(); }
      inline void scale    (const glm::vec3 & value) { _scale    = value; updateModelMatrix(); }
    
      inline void move(const glm::vec3 & value1, const glm::vec3 & value2, const glm::vec3 & value3) {
        _position = value1;
        _rotation = value2;
        _scale    = value3;
        updateModelMatrix();
      }
      
      /* ****************************************************************************/
      // getModelMatrix() -
      /* ****************************************************************************/
      glm::mat4 getModelMatrix() const { return modelMatrix; }
    
      /* ****************************************************************************/
      // getShader() -
      /* ****************************************************************************/
      const glShader & getShader() const { return shader; }
    
      /* ****************************************************************************/
      //
      /* ****************************************************************************/
      glm::vec3 getTranslation() const { return _position; }
      glm::vec3 getRotation()    const { return _rotation; }
    
//
//      /* ****************************************************************************/
//      // setColor() -
//      /* ****************************************************************************/
//      inline void setColor(const glm::vec3 & _color) { color = _color; }
//
        //****************************************************************************/
        // setStyle()
        //****************************************************************************/
        inline void setStyle(int _style) { style = _style; }

        //****************************************************************************/
        // setName()
        //****************************************************************************/
        inline void setName(std::string _name) { name = _name; }
    
    private:
      
      //****************************************************************************/
      // updateModelMatrix()
      //****************************************************************************/
      inline void updateModelMatrix() {
        
        modelMatrix = glm::mat4(1.0f);
        
        modelMatrix = glm::translate(modelMatrix, _position); // Translate it down a bit so it's at the center of the scene
        
        modelMatrix = glm::scale(modelMatrix, _scale);
        
        modelMatrix  = glm::rotate(modelMatrix, _rotation.x, glm::vec3(1, 0, 0)); // where x, y, z is axis of rotation (e.g. 0 1 0)
        modelMatrix  = glm::rotate(modelMatrix, _rotation.y, glm::vec3(0, 1, 0)); // where x, y, z is axis of rotation (e.g. 0 1 0)
        modelMatrix  = glm::rotate(modelMatrix, _rotation.z, glm::vec3(0, 0, 1)); // where x, y, z is axis of rotation (e.g. 0 1 0)
        
      }
      
  protected:
      
    //virtual void _render(const glCamera * camera, int from = 0, int to = -1) = 0;

    /* ****************************************************************************/
    // setInGpu
    /* ****************************************************************************/
    virtual void setInGpu() = 0;
    
    /* ****************************************************************************/
    // cleanInGpu
    /* ****************************************************************************/
    virtual void cleanInGpu() = 0;
    
    /* ****************************************************************************/
    // _setInGpu
    /* ****************************************************************************/
    void _setInGpu() {
      
      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;
      
    }
    
    /* ****************************************************************************/
    // isToInitInGpu
    /* ****************************************************************************/
    inline bool isToInitInGpu() {
      
      DEBUG_LOG("glObject::isToInitInGpu(" + name + ")");
      
      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) {
        cleanInGpu();
        isInitedInGpu   = false;
        isToUpdateInGpu = false;
        return true;
      }
      
      return false;
      
    }
    
  private:
//
//    /* ****************************************************************************/
//    // _init
//    /* ****************************************************************************/
//    void _init(const glm::vec3 & _center = glm::vec3(0.0), const glm::vec3 & _angles = glm::vec3(0.0), const glm::vec3 & _size = glm::vec3(1.0)) {
//
//      center = _center;
//
//      angles = _angles;
//
//      size = _size;
//
//      updateModelMatrix();
//
//      isInited = false;
//
//      isInitedInGpu = false;
//
//      isToUpdateInGpu = false;
//
//    }
    
  }; /* class glObject */

} /* namespace ogl */

#endif /* _H_OGL_OBJECT_H_ */
