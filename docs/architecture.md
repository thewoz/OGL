# Architecture

OGL is **header-only**: everything lives under `include/ogl/` and is pulled in
through the single umbrella header [`ogl.hpp`](../include/ogl.hpp). There is no
library to link against — you only link the third-party dependencies (GLFW,
GLM, GLAD, Assimp, SOIL2, Freetype, libtiff, and optionally ImGui).

`ogl.hpp` is the **only** supported entry point: it defines the third-party
include order the internal headers rely on. Including an internal header
directly (e.g. `<ogl/objects/glSphere.hpp>`) triggers a `#error` — always
include `<ogl/ogl.hpp>` instead.

## Source layout

```
include/
  core/      glWindow, glCamera, glShader, glTexture, glColors, glObject (base class)
             glFont (shared glyph atlas used by the text objects)
  model/     glLight, glMaterial, glMesh, glModel  (Assimp import + Phong shading)
  objects/   ready-to-use drawables:
               glShape                             — base for the lit primitives (adds the light)
               glEllipse, glSphere, glCuboid, glQuad — solid/wireframe 3D shapes
               glBox, glLine, glLines              — edge/line primitives
               glGrid, glAxes, glReferenceAxes     — scene helpers
               glPoints                            — point clouds
               glPrint2D, glPrint3D               — text
               glPlot, glQuad2D                    — misc
  shader/    GLSL programs (.vs vertex, .gs geometry, .fs fragment)
  utils/     logging, error checking, GLFW helpers, snapshots
  data/      fonts and the sample Trex model
```

## Class relationships

Every drawable derives from `glObject` (transform, shader, `style`, `lineWidth`,
`color`). Drawables own GPU handles, so `glObject` is **non-copyable and
non-movable** (a virtual destructor makes deletion through a `glObject*` safe);
pass drawables by reference or pointer. The lit primitives derive from an
intermediate `glShape : glObject`, which adds the `glLight` member and
`setLight()`. `glSphere` is a thin subclass of `glEllipse` (a sphere is an
ellipsoid with equal semi-axes). Text objects (`glPrint2D`/`glPrint3D`) share a
single process-wide glyph atlas through `glFont` instead of each loading their
own copy.

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
    object.render(window.getCamera());
  window.renderEnd();        // swap buffers, poll events
}
```

`renderBegin()` / `renderEnd()` are provided by
[`glWindow`](../include/ogl/core/glWindow.hpp); the camera supplies the
projection and view matrices through `getProjection()` / `getView()`.

## Cameras

[`glCamera`](../include/ogl/core/glCamera.hpp) is a single concrete class that
holds all the camera state (perspective projection, position/orientation,
viewport) and switches behaviour according to its current `MODE`:

| mode    | behaviour                                               |
|:--------|:--------------------------------------------------------|
| `FLY`   | free-fly first-person (arrow keys + mouse look/pan)     |
| `ORBIT` | always looks at a fixed target; ignores input           |
| `PAN`   | orbits a target with mouse, pans with keys, dollies with wheel |

`glWindow` owns exactly one camera. The mode is set at creation time via the
optional last argument of `create()` / `createOffscreen()` (default: `FLY`),
and can be changed at any time with `glWindow::setCamera(mode, fov, pos, target)`
or directly on the camera object via `glWindow::getCamera().setMode(mode)`.
Every setter is always active: the current mode only decides how the stored
state is interpreted, it never makes a setter silently do nothing.
