# OGL

**OGL** is a **minimalistic, header-only C++ OpenGL engine**, designed to make it easier to create graphical applications across different platforms with a clean and lightweight setup.

OGL provides a simple API for:
- Creating windows
- Managing multiple cameras
- Drawing 2D/3D basic shapes
- Handling basic input
- Integrating (optionally) ImGui

---

## Version 4.2

Current release. Highlights:

- Phong lighting for solid objects (sphere, ellipse, cuboid) with directional / point / head-light fallback
- Thick line rendering
- 3D plot axis (`glPlot`)
- Improved color management and camera handling
- Simplified material model (clean Phong subset, no unnecessary complexity)

See [docs/lighting_and_materials.md](docs/lighting_and_materials.md) for the full lighting and material reference.

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

## 📚 Documentation

Detailed documentation lives in the [`docs/`](docs/) folder:

- [Architecture](docs/architecture.md) — library layout, the GPU life-cycle and the rendering loop
- [Lighting and Materials](docs/lighting_and_materials.md) — the Phong lighting and material model
- [Adding a New Object](docs/adding_a_new_object.md) — how to write your own drawable

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

Here is a simple example showing how to create a window, set up a camera, and render basic objects:

```cpp
#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

int main(int argc, char* const argv[]) {

  ogl::glWindow window;
  
  window.create(800, 600);

  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.getCurrentCamera()->setPosition(3, 1.5, 0);
  window.getCurrentCamera()->setYaw(180);
  window.getCurrentCamera()->setPitch(-20);

  ogl::glAxes axes;
  ogl::glGrid grid(10, 10, 0.5, ogl::glColors::cyan);
  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx"); model.setLight(glm::vec3(1.0), glm::vec3(-1.0));
  ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5, "FPS: ");
    
  while(!window.shouldClose()) {
    
    window.renderBegin();
  
      axes.render(window.getCurrentCamera());
      grid.render(window.getCurrentCamera());
      model.render(window.getCurrentCamera());
      text.render(window.getCurrentCamera(), "FPS: " + std::to_string(window.getFPS()));
    
    window.renderEnd();

  }
  
  return 0;
  
}
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

- Add shadow mapping (currently lights shade surfaces directly, with no shadows)
- Add support for multiple lights per object/model
- Add render-to-texture support
- Rewrite camera class
- Improve key input management

---

## Known Issues

- Multi-Sample Anti-Aliasing does not work on Linux (driver/context limitation)
- **Shared textures are not reference-counted.** `glTextures::load()` de-duplicates textures by file path, so two materials that reference the same image share a single GPU texture. However `glMaterial::cleanInGpu()` (and `~glMaterial`) deletes that texture outright, so destroying one material invalidates the texture for any *other* material still using it. In practice this is safe within a single `glModel` (the model owns its meshes/materials and tears them down together), but sharing a texture across two independent materials/models and destroying one will break the other. A proper fix would reference-count entries in `glTextures`; until then, avoid destroying one of two materials that share an image while the other is still in use.

---

## License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

OGL was created to make OpenGL programming easier, cleaner, and faster for small to medium projects, educational purposes, and prototyping.
