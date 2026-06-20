# OGL Documentation

This folder collects the technical documentation of **OGL**, a minimalistic,
header-only C++ OpenGL engine.

## Contents

- [Architecture](architecture.md) — how the library is organized: objects,
  shaders, the GPU life-cycle and the rendering loop.
- [Lighting and Materials](lighting_and_materials.md) — how lights, materials
  and the Phong shading model work, both for the base objects and for imported
  3D models.
- [Adding a New Object](adding_a_new_object.md) — a short guide to extend the
  library with your own drawable object.

## Quick links

- Main project [README](../README.md)
- Example program: [`src/main.cpp`](../src/main.cpp)
- Public headers: [`include/ogl.hpp`](../include/ogl.hpp)
- GLSL shaders: [`include/shader/`](../include/shader)
