# OGL

**OGL** is a **minimalistic, header-only C++ OpenGL engine**, designed to make it easier to create graphical applications across different platforms with a clean and lightweight setup.

OGL provides a simple API for:
- Creating windows
- Managing multiple cameras
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

It also provides utilities like window management, camera control, and snapshot functionality.

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
    
    window.getCurrentCamera()->setPosition(3.0f, 1.5f, 0.0f);
    window.getCurrentCamera()->setYaw(180.0f);
    window.getCurrentCamera()->setPitch(-20.0f);

    ogl::glAxes axes;
    ogl::glReferenceAxes referenceAxes;
    ogl::glGrid grid(10, 10, 0.5f, ogl::glColors::cyan);
    ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx");
    model.setLight(glm::vec3(1.0f), glm::vec3(-1.0f));
    ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5f, "FPS: ");
    
    while (!window.shouldClose()) {
        
        window.renderBegin();
        
            axes.render(window.getCurrentCamera());
            grid.render(window.getCurrentCamera());
            referenceAxes.render(window.getCurrentCamera());
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
| `make uninstall`| Remove the installed OGL headers               |
| `make example`  | Build the example program in `~/bin/ogl`        |

> Note: the Makefile automatically detects whether you are on **Linux** or **macOS**.

---

## 🧰 TODO

- Implement thick line rendering
- Add shadow support
- Add lighting for objects like cuboids, spheres, ellipses
- Add render-to-texture support
- Add axis labels (X,Y,Z) and a background rectangle to `glReferenceAxes` for better visualization
- Include an example with ImGui

---

## 🐞 Known Issues

- Resolve the issue with screenshot capturing on Linux

---

## 📜 License

This project is licensed under the **GNU General Public License v3.0** — see the [LICENSE](LICENSE) file for details.

---

# 🌟 Final Notes

OGL was created to make OpenGL programming easier, cleaner, and faster for small to medium projects, educational purposes, and prototyping.

Enjoy building with it! 🚀

