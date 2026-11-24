# OGL

**OGL** is a minimalistic, header-only C++17 OpenGL engine for small graphical
applications, teaching and prototyping, on macOS and Linux.

It gives you a window with a ready-made camera and a set of objects to draw:

- lines, points, grids, an infinite ground grid
- boxes, cuboids, spheres, ellipsoids, quads, curved sheets
- 3D models (through Assimp)
- 2D and 3D text
- world axes, an orientation gizmo, and plot axes with ticks and labels
- optionally, an [ImGui](https://github.com/ocornut/imgui) overlay

Solid shapes and models are Phong-lit by the light of a `glScene` and can cast
and receive shadows; most shapes also have a wireframe style. Snapshots of the
window can be saved as TIFF.

---

## Install

```bash
git clone https://github.com/thewoz/OGL.git
cd OGL
(cd external && bash install.sh)   # dependencies
make install                       # headers
```

`external/install.sh` installs the dependencies with Homebrew on macOS (which
must already be there) or `sudo apt` on Linux, builds SOIL2 from source, and
installs the bundled GLAD and ImGui.

`make install` links `include/` to `/usr/local/include/ogl`. **It is required:**
the headers include each other as `<ogl/...>` and the shaders, fonts and demo
data are loaded at runtime from that location. For a different prefix, compile
your project with `-DOGL_RESOURCE_DIR='"/your/prefix/ogl"'`.

| Dependency | Used for |
|:-----------|:---------|
| [GLFW](https://github.com/glfw/glfw) | window and input |
| [GLM](https://github.com/g-truc/glm) | math |
| [GLAD](https://glad.dav1d.de) | OpenGL loader (bundled in `external/glad/`) |
| [SOIL2](https://github.com/SpartanJ/soil2) | texture loading |
| [Assimp](https://github.com/assimp/assimp) | 3D model import |
| [Freetype2](https://gitlab.freedesktop.org/freetype/freetype) | text rendering |
| [libtiff](http://www.simplesystems.org/libtiff/) | snapshot export |
| [ImGui](https://github.com/ocornut/imgui) | GUI overlay (bundled in `external/imgui/`, optional) |

> The Makefile looks for the libraries under `/usr/local`. On Apple-Silicon
> Macs Homebrew installs under `/opt/homebrew`: the Xcode project searches both,
> with the Makefile pass the include/lib paths yourself.

---

## Build and test

| Command | What it does |
|:--------|:-------------|
| `make install` / `make uninstall` | create / remove the `/usr/local/include/ogl` link |
| `make example` | build the example without ImGui to `~/bin/ogl` |
| `make example_imgui` | build the same example with an ImGui panel to `~/bin/ogl_imgui` |
| `make test` | build and run the offscreen smoke test |
| `make test_asan` | the same test under AddressSanitizer + UBSan |
| `make lint` | static analysis with `cppcheck` and `clang-tidy` (`brew install cppcheck llvm`) |

The smoke test ([test/smoke.cpp](test/smoke.cpp)) renders offscreen frames with
every main object type, takes a snapshot and reads pixels back to check the
color pipeline. It prints `SMOKE TEST OK` on success and needs the headers
installed.

---

## Example

A minimal program: a lit, shadow-casting cuboid on a floor. The full example,
with a model, text and two switchable scenes, is [`src/main.cpp`](src/main.cpp).

```cpp
#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

int main() {

  ogl::glWindow window;
  window.create(1024, 768);
  window.setCursorInputMode(GLFW_CURSOR_DISABLED);

  window.getCamera().setPosition(3, 1.5f, 0);
  window.getCamera().lookAt(0, 0.25f, 0);
  window.getCamera().setzNearFar(0.1f, 100.0f);

  // The scene owns the light and the shadow map shared by every lit object.
  ogl::glScene scene;
  scene.setDirectionalLight(glm::vec3(-1.0f, -1.0f, -1.0f));
  scene.enableShadows(true);
  scene.setBounds(glm::vec3(0.0f, 0.5f, 0.0f), 3.0f);

  // A quad laid flat on the XZ plane, as the floor.
  ogl::glQuad floor(glm::vec2(10.0f), glm::vec3(0.55f));
  floor.rotate(glm::vec3(-1.57079633f, 0.0f, 0.0f));

  ogl::glCuboid cuboid(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::orange);
  cuboid.translate(glm::vec3(0.0f, 0.25f, 0.0f));

  ogl::glAxes axes;
  ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5f);

  while(!window.shouldClose()) {

    window.renderBegin();

      // Shadow pass: the casters go into the shadow map of the scene.
      scene.beginShadowPass();
      floor.renderDepth(scene.getShadowShader());
      cuboid.renderDepth(scene.getShadowShader());
      scene.endShadowPass();

      // Render pass: lit objects take the light and the shadows from &scene.
      floor.render(window.getCamera(), &scene);
      cuboid.render(window.getCamera(), &scene);
      axes.render(window.getCamera());
      text.render(window.getCamera(), "FPS: " + std::to_string(window.getFPS()));

    window.renderEnd();

  }

  return 0;

}
```

**ImGui.** Do not define `OGL_WITHOUT_IMGUI`: `glWindow` then creates the ImGui
context and runs its frame for you, so you just issue `ImGui::` widgets between
`renderBegin()` and `renderEnd()`.

---

## Camera controls

The window drives its camera according to the camera mode
(`window.getCamera().setMode()`, FLY by default):

| Input | FLY | ORBIT | FIXED |
|:------|:----|:------|:------|
| Mouse move | look around | orbit around the target | — |
| Ctrl + mouse move | translate along the world X / Y axes | pan | — |
| Arrow keys | move forward / back / left / right | pan, with Shift held | — |
| Scroll wheel | — | dolly in / out | — |

**ESC** closes the window; turn that off with `window.setCloseOnEscape(false)`.

---

## Colors

OGL renders through a linear lighting pipeline:

- Every color you pass in is an **sRGB value**, the one you would pick in a
  color picker. The shaders linearize it and the GPU encodes the result back.
- A lit surface therefore renders **darker** than its color, because the light
  is not at full intensity: with the default light an albedo of 0.45 lands at
  about 0.38 on screen.
- Unlit objects (lines, axes, grids, plots, text, 2D quads) and the window
  background come out exactly as given; so does a solid shape after
  `setUnlit(true)`.
- `setShininess()` and `setSpecularStrength()` set the highlight of a shape;
  0 strength gives a fully matte surface.
- A custom shader for a `glShape` must output **linear** color.

---

## TODO, known issues and license

Open work and documented limitations are in [TODO.md](TODO.md).

OGL is licensed under the **GNU General Public License v3.0**, see
[LICENSE](LICENSE).
