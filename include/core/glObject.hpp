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
  // Class glBuffers
  //****************************************************************************
  // The vertex array and the buffers of a drawable, with the handful of calls
  // every setInGpu() needs: the handles are zero by construction and the
  // gen / bind / upload / attribute boilerplate lives here once instead of
  // being repeated in every object.
  //
  // It is a plain set of GL names and deletes nothing on its own: a GL name
  // can only be deleted in the context that created it, and that is known by
  // the owner (see glObject::releaseInGpu()), which calls destroy() or
  // forget() accordingly.
  //****************************************************************************
  class glBuffers {

  public:

    static constexpr int maxBuffers = 4;

  private:

    GLuint vao = 0;
    GLuint vbo[maxBuffers] = {0, 0, 0, 0};
    int    count = 0;

  public:

    //**************************************************************************
    // create() - a new vertex array with 'count' buffers. The previous names,
    // if any, are NOT deleted: after a context change they belong to the old
    // context (call destroy() first when they are to be freed).
    //**************************************************************************
    void create(int _count) {

      if(_count < 0 || _count > maxBuffers) {
        fprintf(stderr, "ERROR [glBuffers]: at most %d buffers\n", maxBuffers);
        abort();
      }

      count = _count;

      glGenVertexArrays(1, &vao);

      if(count > 0) glGenBuffers(count, vbo);

    }

    //**************************************************************************
    // destroy() - delete the names (in the current context) / forget() - drop
    // them without deleting anything
    //**************************************************************************
    void destroy() {

      if(count > 0) glDeleteBuffers(count, vbo);
      if(vao != 0)  glDeleteVertexArrays(1, &vao);

      forget();

    }

    void forget() {

      vao = 0;
      for(int i=0; i<maxBuffers; ++i) vbo[i] = 0;
      count = 0;

    }

    bool isCreated() const { return vao != 0; }

    GLuint buffer(int index) const { return vbo[index]; }

    //**************************************************************************
    // bind() / unbind()
    //**************************************************************************
    void bind() const { glBindVertexArray(vao); }

    static void unbind() {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    //**************************************************************************
    // data() - upload a vertex buffer and leave it bound, for pointer()
    //**************************************************************************
    void data(int index, const void * data, size_t bytes, GLenum usage = GL_STATIC_DRAW) const {
      glBindBuffer(GL_ARRAY_BUFFER, vbo[index]);
      glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) bytes, data, usage);
    }

    //**************************************************************************
    // pointer() - a float attribute read from the vertex buffer bound now
    //**************************************************************************
    void pointer(GLuint location, GLint components, size_t stride = 0, size_t offset = 0) const {
      glVertexAttribPointer(location, components, GL_FLOAT, GL_FALSE, (GLsizei) stride, reinterpret_cast<void *>(offset));
      glEnableVertexAttribArray(location);
    }

    //**************************************************************************
    // attribute() - a buffer that holds one float attribute: data() + pointer()
    //**************************************************************************
    void attribute(int index, GLuint location, GLint components, const void * data, size_t bytes, GLenum usage = GL_STATIC_DRAW) const {
      this->data(index, data, bytes, usage);
      pointer(location, components);
    }

    //**************************************************************************
    // elements() - upload the index buffer of the vertex array
    //**************************************************************************
    void elements(int index, const void * data, size_t bytes, GLenum usage = GL_STATIC_DRAW) const {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[index]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) bytes, data, usage);
    }

  }; /* class glBuffers */

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

    // vertex array and buffers, see glBuffers
    glBuffers buffers;

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
    // Drawables are therefore non-copyable but movable: moving transfers
    // ownership of the GPU resources and neuters the moved-from object (see
    // below). The virtual destructor makes deletion through a glObject* well
    // defined. It frees the buffers itself: at this point the virtual calls
    // resolve to the glObject versions, which is right for every object whose
    // GPU state is just 'buffers'; a class with more than that (glModel) calls
    // releaseInGpu() from its own destructor first.
    //***************************************************************************
    virtual ~glObject() {

      // qualified on purpose: in a destructor there is no virtual dispatch,
      // and these are the versions meant here (see the comment above)
      if(isInitedInGpu && !isCurrentWindow(windowID)) glObject::forgetInGpu();

      glObject::cleanInGpu();

    }

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
        buffers(o.buffers), isInited(o.isInited), isInitedInGpu(o.isInitedInGpu), isToUpdateInGpu(o.isToUpdateInGpu),
        _position(o._position), _rotation(o._rotation), _scale(o._scale),
        modelMatrix(o.modelMatrix), style(o.style), lineWidth(o.lineWidth), color(o.color) {
      o.isInited        = false;
      o.isInitedInGpu   = false;
      o.isToUpdateInGpu = false;
      o.windowID        = 0;
      o.buffers.forget();
    }

    glObject & operator = (glObject && o) noexcept {
      if(this != &o) {
        releaseInGpu();   // the buffers this object held until now
        name            = std::move(o.name);
        windowID        = o.windowID;
        shader          = std::move(o.shader);
        buffers         = o.buffers;
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
        o.buffers.forget();
      }
      return *this;
    }
    
    //****************************************************************************
    // initInGpu() -
    //****************************************************************************
    void initInGpu() {

      windowID = currentWindowID();
      
      DEBUG_LOG("glObject::initInGpu(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "ERROR [glObject]: must be initialized before uploading to GPU\n");
        abort();
      }
      
      setInGpu();

      isInitedInGpu = true;

      // A pending update is satisfied by this upload: without this reset, an
      // init() that set the flag before the very first render would trigger a
      // second, redundant upload on the following frame.
      isToUpdateInGpu = false;

    }
    
    //****************************************************************************
    // renderDepth() - draw this object's geometry into the scene shadow map,
    // using the scene's depth-only shader. Default is a no-op: only the solid
    // shadow casters (model, cuboid, quad, sheet, sphere, ellipse) override it. Lines,
    // points, text and axes do not cast meaningful shadows.
    //****************************************************************************
    virtual void renderDepth(const glShader & /*depthShader*/) { }

    //****************************************************************************
    // Position function
    //****************************************************************************
    // Virtual so composite objects (e.g. glAxes, which carries its axis labels
    // as child glPrint3D objects) can forward the transform to their children.
    virtual void translate(const glm::vec3 & value) { _position = value; updateModelMatrix(); }
    virtual void rotate   (const glm::vec3 & value) { _rotation = value; updateModelMatrix(); }
    virtual void scale    (const glm::vec3 & value) { _scale    = value; updateModelMatrix(); }
    
    // Goes through the virtual setters (not the raw members) so composite
    // objects that override them (e.g. glAxes forwarding to its labels) stay
    // in sync when moved through this convenience call too.
    inline void move(const glm::vec3 & value1, const glm::vec3 & value2, const glm::vec3 & value3) {
      translate(value1);
      rotate(value2);
      scale(value3);
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
    inline void setName(const std::string & _name) { name = _name; }  
    
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
    // cleanInGpu - free the GPU handles in the current context. The default is
    // enough for an object whose GPU state is 'buffers'.
    //****************************************************************************
    virtual void cleanInGpu() {

      if(isInitedInGpu) {
        buffers.destroy();
        isInitedInGpu = false;
      }

    }

    //****************************************************************************
    // prepareBuffers - what a setInGpu() starts with: the vertex array, with
    // 'count' buffers, bound and ready to be filled. On the first upload in a
    // context the names are created (after a context change isInitedInGpu is
    // false and the old ones are left to their own context); on an update in
    // the same context the existing ones are filled again.
    //****************************************************************************
    void prepareBuffers(int count) {

      if(!isInitedInGpu || !buffers.isCreated()) buffers.create(count);

      buffers.bind();

    }
    
    //****************************************************************************
    // releaseInGpu - what the destructors call to free the GPU handles. They
    // are deleted only if the context that owns them is the current one: with
    // another window current the same names belong to its objects, and
    // deleting them would destroy those. In that case the handles are just
    // forgotten and go away with their own context.
    //****************************************************************************
    void releaseInGpu() {

      if(isInitedInGpu && !isCurrentWindow(windowID)) forgetInGpu();

      cleanInGpu();

    }

    //****************************************************************************
    // forgetInGpu - drop the GPU state without deleting anything
    //****************************************************************************
    virtual void forgetInGpu() {

      buffers.forget();

      isInitedInGpu = false;

    }

    //****************************************************************************
    // _setInGpu
    //****************************************************************************
    void _setInGpu() {

      windowID = currentWindowID();

    }
    
    //****************************************************************************
    // isToInitInGpu - returns true when GPU buffers need to be (re-)uploaded.
    // This happens on the very first render, when the context changes (different
    // window), or when the caller sets isToUpdateInGpu after a data change.
    //****************************************************************************
    inline bool isToInitInGpu() {

      DEBUG_LOG("glObject::isToInitInGpu(" + name + ")");

      if(windowID != currentWindowID() || !isInitedInGpu) {
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
