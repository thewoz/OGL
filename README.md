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
> On Apple-Silicon Macs, Homebrew installs under `/opt/homebrew`: the Xcode
> project already searches both prefixes, but the Makefile assumes
> `/usr/local` — pass the include/lib paths explicitly if your libraries live
> in `/opt/homebrew`.

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

### 6. Run the tests

A non-interactive smoke test ([test/smoke.cpp](test/smoke.cpp)) renders 30
offscreen frames with every main object type (shapes, model, text, plot,
shadow pass, dynamic buffer updates), takes a snapshot and verifies it:

```bash
# Normal build
make test

# Same test under AddressSanitizer + UndefinedBehaviorSanitizer
make test_asan
```

Both print `SMOKE TEST OK` on success. The headers must be installed first
(`make install`), since the test loads shaders, fonts and the demo model from
the installed location.

### 7. Uninstall (if needed)

```bash
make uninstall
```

---

## 🛠 Example Code

Here is a simplified version of [`src/main.cpp`](src/main.cpp) (built by
`make example`): it renders two switchable demo scenes — the T-Rex model, or
a cuboid + sphere + 3D text label — toggled at runtime by pressing **TAB**:

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

  ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5f);

  ogl::glReferenceAxes referenceAxes;
  referenceAxes.setLineWidth(2);

  // Demo scene A: the T-Rex model.
  ogl::glModel model(OGL_RESOURCE_DIR "/data/model/Trex/Trex.fbx");

  // Demo scene B: primitive shapes + 3D text.
  ogl::glCuboid cuboid(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::white);
  cuboid.translate(glm::vec3(1.2f, 0.25f, 0.0f));
  ogl::glSphere sphere(0.35f, 24, 24, ogl::glShader::STYLE::SOLID, ogl::glColors::orange);
  sphere.translate(glm::vec3(-1.2f, 0.35f, 0.0f));
  ogl::glPrint3D label3D("OGL", glm::vec3(0.0f, 1.3f, 0.0f), ogl::glColors::white, 0.6f, true);

  bool showTrexScene = true;
  bool tabWasDown = false;

  while(!window.shouldClose()) {

    window.renderBegin();

      // TAB toggles between the two demo scenes (edge-triggered).
      bool tabIsDown = (glfwGetKey(window.window, GLFW_KEY_TAB) == GLFW_PRESS);
      if(tabIsDown && !tabWasDown) showTrexScene = !showTrexScene;
      tabWasDown = tabIsDown;

      // Shadow pass: render the casters into the scene shadow map.
      if(scene.areShadowsEnabled()) {
        scene.beginShadowPass();
        floor.renderDepth(scene.getShadowShader());
        if(showTrexScene) model.renderDepth(scene.getShadowShader());
        else { cuboid.renderDepth(scene.getShadowShader()); sphere.renderDepth(scene.getShadowShader()); }
        scene.endShadowPass();
      }

      // Render pass: pass &scene so lit objects use its light and shadows.
      floor.render(window.getCamera(), &scene);
      axes.render(window.getCamera());
      grid.render(window.getCamera());
      if(showTrexScene) {
        model.render(window.getCamera(), &scene);
      } else {
        cuboid.render(window.getCamera(), &scene);
        sphere.render(window.getCamera(), &scene);
        label3D.render(window.getCamera());
      }
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
| `make test`           | Build and run the offscreen smoke test |
| `make test_asan`      | Run the smoke test under ASan + UBSan |

The Makefile automatically detects whether you are on **Linux** or **macOS**.

---

## TODO & Known Issues

Open work items and documented limitations live in [TODO.md](TODO.md).

---

## License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

OGL was created to make OpenGL programming easier, cleaner, and faster for small to medium projects, educational purposes, and prototyping.
