# OGL

**OGL** is a **minimalistic, header-only C++ OpenGL engine**, designed to make it easier to create graphical applications across different platforms with a clean and lightweight setup.

OGL provides a simple API for:
- Creating windows
- Managing a camera with switchable modes (FLY, FIXED, ORBIT)
- Drawing 2D/3D basic shapes
- Handling basic input
- Integrating (optionally) ImGui

---

## ✨ Features

OGL allows you to render:

- Lines
- Spheres
- Ellipses
- Grids
- Boxes
- Cuboids
- Points
- Planes
- 2D and 3D Text
- 3D Models
- 3D Plot

It also provides utilities like window management, camera control, and snapshot functionality.

---

## Dependencies

OGL requires the following libraries:

| Library | Notes |
|:--------|:------|
| [GLFW](https://github.com/glfw/glfw) | window and input |
| [GLM](https://github.com/g-truc/glm) | math |
| [GLAD](https://glad.dav1d.de) | OpenGL loader — bundled in `external/glad/` |
| [SOIL2](https://github.com/SpartanJ/soil2) | texture loading |
| [Assimp](https://github.com/assimp/assimp) | 3D model import |
| [Freetype2](https://gitlab.freedesktop.org/freetype/freetype) | text rendering |
| [libtiff](http://www.simplesystems.org/libtiff/) | snapshot export |
| [ImGui](https://github.com/ocornut/imgui) | GUI overlay — bundled in `external/imgui/`, optional |

GLAD and ImGui are provided in the `external/` folder with their own Makefiles and
do not need to be downloaded separately (see below).

---

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/your_username/ogl.git
cd ogl
```

### 2. Install system dependencies

The `external/install.sh` script installs all required libraries. It handles
both **macOS** (via Homebrew) and **Linux** (via apt), builds SOIL2 from source,
and compiles and installs the bundled GLAD and ImGui:

```bash
cd external
bash install.sh
cd ..
```

> On macOS, Homebrew must already be installed. On Linux, the script uses `sudo apt`.

The bundled libraries can also be installed individually:

```bash
# Install only GLAD
cd external/glad && make install

# Install only ImGui
cd external/imgui && make install
```

### 3. Install the OGL headers

```bash
make install
```

This creates a symbolic link at `/usr/local/include/ogl` pointing to the
`include/` directory, so you can `#include <ogl/ogl.hpp>` in any project.

> **This step is required.** The library does not work from the bare
> repository checkout: the internal headers include each other as
> `<ogl/...>`, and the preset shaders, fonts and demo data are loaded at
> runtime from the installed location. If you install to a different
> prefix, compile your project with
> `-DOGL_RESOURCE_DIR='"/your/prefix/ogl"'` so the library can find its
> shaders, fonts and data.

### 4. Build the example programs

```bash
# Basic example (no ImGui)
make example

# ImGui integration example
make example_imgui
```

The executables are placed in `~/bin/ogl` and `~/bin/ogl_imgui`.

### 5. Run the examples

```bash
~/bin/ogl
~/bin/ogl_imgui
```

### 6. Uninstall (if needed)

```bash
make uninstall
```

---

## 🛠 Example Code

Here is a simple example showing how to create a window, set up a camera, and
render basic objects. It is the same program as [`src/main.cpp`](src/main.cpp)
(built by `make example`):

```cpp
#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

int main(int argc, char * const argv[]) {

  ogl::glWindow window;

  window.create(1024, 768);

  window.setCursorInputMode(GLFW_CURSOR_DISABLED);

  // The default camera is a FLY (free-fly) camera.
  window.getCamera().setPosition(3, 1.5f, 0);
  window.getCamera().setYaw(180);
  window.getCamera().setPitch(-20);
  // Push the far plane out so distant objects are not clipped.
  window.getCamera().setzNearFar(0.1f, 100.0f);

  // The scene owns the light and the shadow map shared by every lit object.
  ogl::glScene scene;
  scene.setDirectionalLight(glm::vec3(-1.0f, -1.0f, -1.0f));
  scene.enableShadows(true);
  scene.setBounds(glm::vec3(0.0f, 0.5f, 0.0f), 3.0f);

  ogl::glAxes axes;
  ogl::glGrid grid(10, 10, 0.5f, ogl::glColors::cyan);

  // Solid floor that receives the shadows (a quad laid flat on the XZ plane).
  ogl::glQuad floor(glm::vec2(10.0f), glm::vec3(0.55f), ogl::glShader::STYLE::SOLID);
  floor.rotate(glm::vec3(-1.57079633f, 0.0f, 0.0f)); // -90° about X: normal up
  floor.translate(glm::vec3(0.0f, -0.01f, 0.0f));

  ogl::glCuboid cuboid(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::white);
  cuboid.translate(glm::vec3(1.2f, 0.25f, 0.0f));

  ogl::glModel model(OGL_RESOURCE_DIR "/data/model/Trex/Trex.fbx");

  ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5f);

  ogl::glReferenceAxes referenceAxes;
  referenceAxes.setLineWidth(2);

  while(!window.shouldClose()) {

    window.renderBegin();

      // Shadow pass: render the casters into the scene shadow map.
      if(scene.areShadowsEnabled()) {
        scene.beginShadowPass();
        floor.renderDepth(scene.getShadowShader());
        cuboid.renderDepth(scene.getShadowShader());
        model.renderDepth(scene.getShadowShader());
        scene.endShadowPass();
      }

      // Render pass: pass &scene so lit objects use its light and shadows.
      floor.render(window.getCamera(), &scene);
      axes.render(window.getCamera());
      grid.render(window.getCamera());
      cuboid.render(window.getCamera(), &scene);
      model.render(window.getCamera(), &scene);
      text.render(window.getCamera(), "FPS: " + std::to_string(window.getFPS()));
      referenceAxes.render(window.getCamera());

    window.renderEnd();

  }

  return 0;

}
```

### With ImGui

OGL can also drive an [ImGui](https://github.com/ocornut/imgui) overlay. Simply
**do not** define `OGL_WITHOUT_IMGUI`: `glWindow` then creates the ImGui context
and runs the frame lifecycle for you (`renderBegin()` calls `ImGui::NewFrame()`,
`renderEnd()` calls `ImGui::Render()`), so you just issue your `ImGui::` widgets
in between:

```cpp
#include <ogl/ogl.hpp>   // OGL_WITHOUT_IMGUI NOT defined

// ... create window and scene objects ...

while(!window.shouldClose()) {

  window.renderBegin();

    ImGui::Begin("Scene Controls");
    ImGui::Text("FPS: %d", window.getFPS());
    ImGui::End();

    // ... render your scene ...

  window.renderEnd();

}
```

The same [`src/main.cpp`](src/main.cpp) doubles as the ImGui example: built
without `OGL_WITHOUT_IMGUI` (via `make example_imgui`) the `#ifndef`-guarded
blocks add an ImGui control panel. Build and run it with:

```bash
make example_imgui
~/bin/ogl_imgui
```

---

## Makefile Commands

| Command               | Description |
|:----------------------|:------------|
| `make install`        | Symlink `include/` to `/usr/local/include/ogl` |
| `make uninstall`      | Remove the symlink |
| `make example`        | Build the basic example to `~/bin/ogl` |
| `make example_imgui`  | Build the ImGui integration example to `~/bin/ogl_imgui` |

The Makefile automatically detects whether you are on **Linux** or **macOS**.

---

## TODO

- Add support for multiple lights per object/model
- Add render-to-texture support
- Improve key input management
- **Batch text rendering**: `glFont` creates one GL texture per glyph and
  `glPrint2D`/`glPrint3D` issue one draw call per character
  (bind + buffer upload + draw for every letter). Move to a single glyph
  atlas texture and one buffered draw call per string.
- **Deduplicate the VAO/VBO boilerplate**: the same ~15-line
  `setInGpu()`/`cleanInGpu()` block (gen/bind/attrib/cleanup guard) is
  copy-pasted across all drawables in `include/objects/`. Extract a small
  RAII buffer-set helper in `glObject` so handles are zero-initialized by
  construction and the per-object code shrinks by half.
- **Unify the glyph-rendering loop** of `glPrint2D` and `glPrint3D`
  (~50 near-identical lines; only the screen-position computation differs).
  Best done together with the glyph-atlas batching above, since that work
  rewrites the same loop.

---

## Known Issues

- **The library is single-threaded by design.** All rendering, resource
  loading and window management must happen on the main thread: the internal
  state (texture cache, color palette, font atlas, window counters) is not
  protected by locks, and GLFW itself requires event processing on the main
  thread. Multi-threaded use is not supported.
- **GPU handles are not freed on a context change.** When an object that was
  uploaded for one window is rendered in a different window, its buffers are
  re-created in the new context and the handles in the old context are
  deliberately left to be released when that context is destroyed (freeing
  them would require re-binding the old context). This is harmless in
  practice but shows up as "leaked" objects in GPU debuggers.
- Multi-Sample Anti-Aliasing does not work on Linux (driver/context limitation)
- **Textures are always loaded as RGB: the alpha channel is dropped.**
  `glTexture` forces `SOIL_LOAD_RGB`, so transparent PNGs become opaque and
  the *opacity* texture maps that `glMaterial` accepts have no effect.
  Proper support requires loading with the source channel count
  (`SOIL_LOAD_AUTO`), picking `GL_RED`/`GL_RGB`/`GL_RGBA` at upload time,
  and handling transparency in the model shader (blending plus
  back-to-front ordering of transparent objects).
- **Shared textures are not reference-counted.** `glTextures::load()` de-duplicates textures by file path, so two materials that reference the same image share a single GPU texture. However `glMaterial::cleanInGpu()` (and `~glMaterial`) deletes that texture outright, so destroying one material invalidates the texture for any *other* material still using it. In practice this is safe within a single `glModel` (the model owns its meshes/materials and tears them down together), but sharing a texture across two independent materials/models and destroying one will break the other. A proper fix would reference-count entries in `glTextures`; until then, avoid destroying one of two materials that share an image while the other is still in use.

---

## License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

OGL was created to make OpenGL programming easier, cleaner, and faster for small to medium projects, educational purposes, and prototyping.
