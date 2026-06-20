# OGL

**OGL** is a **minimalistic, header-only C++ OpenGL engine**, designed to make it easier to create graphical applications across different platforms with a clean and lightweight setup.

OGL provides a simple API for:
- Creating windows
- Managing multiple cameras
- Drawing 2D/3D basic shapes
- Handling basic input
- Integrating (optionally) ImGui

---

## 🆕 Version 4.2 Released

Version 4.2 of the library has been released. Among the highlights (along with other updates):

- Updated and improved color management
- Implemented thick line rendering
- Implemented lighting for objects like cuboids, spheres, ellipses
- Camera Management Improvements
- Added a class for 3D plot axis rendering

### Recent fixes & clean-up

- Fixed model texturing: each material texture is now bound to its own texture
  unit and sampler (previously every sampler read texture unit 0, breaking
  specular and normal maps).
- Unified and simplified the lighting model across base objects and models
  (directional → point → head-light fallback, so objects are always lit).
- Simplified `glMaterial` down to a clean Phong subset
  (emissive / ambient / diffuse / specular + shininess + opacity, plus the
  common texture maps).

The model shaders still carry a shadow-mapping path; it is kept for future work
but is not driven yet (no depth pass), so shadows are currently disabled.

See [docs/lighting_and_materials.md](docs/lighting_and_materials.md) for details.

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

## 📦 Dependencies

You will need the following libraries installed:

- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [GLAD](https://glad.dav1d.de)
- [ImGui](https://github.com/ocornut/imgui) *(optional)*
- [SOIL2](https://github.com/SpartanJ/soil2)
- [Assimp](https://github.com/assimp/assimp)
- [Freetype2](https://gitlab.freedesktop.org/freetype/freetype)
- [libtiff](http://www.simplesystems.org/libtiff/)

---

## 🚀 Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/your_username/ogl.git
cd ogl
```

### 2. Install the headers

```bash
make install
```

This will create a symbolic link at `/usr/local/include/ogl` pointing to the `include/` directory.

### 3. Build the example program

```bash
make example
```

The example executable will be created inside your `~/bin/` folder.

### 4. Run the example

```bash
~/bin/ogl
```

### 5. Uninstall (if needed)

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

## 📋 Makefile Commands

| Command         | Description                                    |
|:----------------|:-----------------------------------------------|
| `make install`  | Install the OGL headers to `/usr/local/include/ogl` |
| `make uninstall`| Remove the installed OGL headers                    |
| `make example`  | Build the example program in `~/bin/ogl`            |

> Note: the Makefile automatically detects whether you are on **Linux** or **macOS**.

---

## 🧰 TODO

- Finish shadow mapping (the model shader already supports it; a depth pass and the light-space matrix still need to be driven)
- Add support for multiple lights per object/model
- Add render-to-texture support
- Add axis labels (X,Y,Z) to `glReferenceAxes` for better visualization
- Include an example with ImGui
- Rewrite camera class
- Improves key management

---

## 🐞 Known Issues

- Multi-Sample Anti-Aliasing do not work on Linux
- glReferenceAxes doesn't work well with all types of cameras

---

## 📜 License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

# 🌟 Final Notes

OGL was created to make OpenGL programming easier, cleaner, and faster for small to medium projects, educational purposes, and prototyping.

Enjoy building with it! 🚀
