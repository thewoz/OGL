# Architecture

OGL is **header-only**: everything lives under `include/ogl/` and is pulled in
through the single umbrella header [`ogl.hpp`](../include/ogl.hpp). There is no
library to link against — you only link the third-party dependencies (GLFW,
GLM, GLAD, Assimp, SOIL2, Freetype, libtiff, and optionally ImGui).

## Source layout

```
include/
  core/      glWindow, glCamera, glShader, glTexture, glColors, glObject (base class)
  model/     glLight, glMaterial, glMesh, glModel  (Assimp import + Phong shading)
  objects/   ready-to-use drawables:
               glSphere, glEllipse, glCuboid       — solid/wireframe 3D shapes
               glBox, glLine, glLines              — edge/line primitives
               glGrid, glAxes, glReferenceAxes     — scene helpers
               glPoints                            — point clouds
               glPrint2D, glPrint3D               — text
               glPlot, glQuad, glQuad2D            — misc
  shader/    GLSL programs (.vs vertex, .gs geometry, .fs fragment)
  utils/     logging, error checking, GLFW helpers, snapshots
  data/      fonts and the sample Trex model
```

## The drawable object model

Every drawable derives from [`glObject`](../include/ogl/core/glObject.hpp),
which holds the TRS transform (`translate` / `rotate` / `scale`), a `glShader`,
and the bookkeeping flags used by the lazy GPU upload (see below).

A typical object exposes three things:

- a constructor / `init(...)` that fills CPU-side data, picks a shader, and
  sets `isInited = true`;
- a `render(camera)` method that calls `isToInitInGpu()` and then draws;
- private `setInGpu()` / `cleanInGpu()` that manage the OpenGL buffers.

## GPU life-cycle (lazy upload)

OGL never touches the GPU at construction time. Instead each object tracks two
flags, `isInited` (CPU data ready) and `isInitedInGpu` (buffers uploaded), plus
the id of the window/context it was uploaded into.

On the first `render()` — and again whenever the current context changes —
`isToInitInGpu()` returns `true` and the object uploads its buffers through
`initInGpu() → setInGpu()`. This makes objects safe to create before a window
exists and to share across multiple windows/contexts.

The same pattern is used by `glShader` (compiles/links on first `use()`) and by
`glTexture` (uploads on first `setInGpu()`).

## Shaders

Shaders are plain GLSL files loaded at runtime from
`/usr/local/include/ogl/shader/` (installed by `make install`). `glShader`
offers a small set of presets:

| preset           | files                       | used by                              |
|:-----------------|:----------------------------|:-------------------------------------|
| `initSolid`      | `solid.vs/.fs`              | sphere, ellipse, cuboid (SOLID mode) |
| `initWireframe`  | `wireframe.vs/.gs/.fs`      | sphere, ellipse, cuboid (WIREFRAME)  |
| `initLine`       | `line.vs/.gs/.fs`           | thick lines, glBox edges             |
| `initPoints`     | `points.vs/.fs`             | point clouds                         |
| `initModel`      | `model.vs/.fs`              | imported 3D models (glModel)         |
| `initText`       | `text.vs/.fs`               | 2D/3D text                           |
| `initPlain2D`    | `plain2D.vs/.fs`            | 2D overlays                          |

Uniforms are set through the templated `glShader::setUniform(name, value)`.

## The rendering loop

```cpp
while(!window.shouldClose()) {
  window.renderBegin();      // clear buffers, update camera/input
    object.render(window.getCurrentCamera());
  window.renderEnd();        // swap buffers, poll events
}
```

`renderBegin()` / `renderEnd()` are provided by
[`glWindow`](../include/ogl/core/glWindow.hpp); the camera supplies the
projection and view matrices through `getProjection()` / `getView()`.
