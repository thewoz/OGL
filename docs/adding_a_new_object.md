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
    // note: the solid color lives in the base class (glObject::color) — do not
    // redeclare a 'color' member here, it would shadow it and break setColor().

  public:

    glMyShape(const glm::vec3 & _color = glm::vec3(1.0f)) { init(_color); }
    ~glMyShape() { cleanInGpu(); }

    void init(const glm::vec3 & _color) {
      shader.setName(name);
      shader.initSolid();          // pick a shader preset
      color    = _color;
      isInited = true;             // CPU data is ready
    }

    void render(const glCamera & camera, const glScene * scene = nullptr) {
      if(isToInitInGpu()) initInGpu();   // lazy GPU upload

      shader.use();
      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       camera.getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);

      // Lit object: take the light (and shadow map) from the scene, or fall
      // back to a camera head light when no scene is passed.
      if(scene) scene->setInShader(shader, camera.getView());
      else { glLight().setInShader(shader, camera.getView()); shader.setUniform("useShadow", false); }

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
- If your object should be lit, accept a `const glScene * scene = nullptr`
  parameter in `render()` and call `scene->setInShader(shader, camera.getView())`
  (with the head-light fallback shown above); use the `solid` shader so the
  Phong code is shared with the other objects. To also cast shadows, override
  `renderDepth(const glShader &)` and draw the geometry with the depth shader.
  See [Lighting and Materials](lighting_and_materials.md).
- Register your header in [`ogl.hpp`](../include/ogl.hpp) under the
  `// Objects` section.
- Wrap GPU calls and check errors with `glCheckError()` (from
  `utils/error.hpp`).
