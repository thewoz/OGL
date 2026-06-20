# Adding a New Object

Every drawable in OGL follows the same pattern. To add your own, derive from
[`glObject`](../include/ogl/core/glObject.hpp) and implement four things:

1. an `init(...)` that prepares CPU-side data and selects a shader;
2. a `render(camera)` method;
3. a private `setInGpu()` that uploads buffers (called lazily);
4. a private `cleanInGpu()` that releases them.

## Skeleton

```cpp
#ifndef _H_OGL_MYSHAPE_H_
#define _H_OGL_MYSHAPE_H_

namespace ogl {

  class glMyShape : public glObject {

    GLuint vao = 0;
    GLuint vbo[2];
    glm::vec3 color;

  public:

    glMyShape(const glm::vec3 & _color = glm::vec3(1.0f)) { init(_color); }
    ~glMyShape() { cleanInGpu(); }

    void init(const glm::vec3 & _color) {
      shader.setName(name);
      shader.initSolid();          // pick a shader preset
      color    = _color;
      isInited = true;             // CPU data is ready
    }

    void render(const glCamera * camera) {
      if(isToInitInGpu()) initInGpu();   // lazy GPU upload

      shader.use();
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);

      glBindVertexArray(vao);
      // glDrawArrays / glDrawElements ...
      glBindVertexArray(0);

      glCheckError();
    }

  private:

    void setInGpu() override {
      // glGenVertexArrays / glGenBuffers / glBufferData / glVertexAttribPointer
      isInitedInGpu = true;
    }

    void cleanInGpu() override {
      if(isInitedInGpu) {
        glDeleteBuffers(2, vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
      }
    }
  };

}
#endif
```

## Notes

- **Never** call OpenGL from the constructor. Fill the CPU data, set
  `isInited = true`, and let the first `render()` trigger `setInGpu()`. This is
  what makes objects safe to create before a window exists.
- If your object should be lit, give it a `glLight` member and call
  `light.setInShader(shader, camera->getView())` in `render()`; use the
  `solid` shader so the Phong code is shared with the other objects. See
  [Lighting and Materials](lighting_and_materials.md).
- Register your header in [`ogl.hpp`](../include/ogl.hpp) under the
  `// Objects` section.
- Wrap GPU calls and check errors with `glCheckError()` (from
  `utils/error.hpp`).
