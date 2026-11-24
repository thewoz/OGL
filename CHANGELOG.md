# Changelog

## v5.0 — 2026-11-24

Changes since v4.2.3. This release reworks lighting, text and resource handling
and renames part of the public API: see **Breaking changes** before upgrading.

### New

- **Scene lighting and shadows.** A `glScene` holds the light (directional or
  point) and a shadow map shared by all lit objects. Models, cuboids, spheres,
  ellipsoids, quads and sheets cast and receive shadows through
  `renderDepth()` and `render(camera, &scene)`.
- **New objects**: `glSheet` (open half cylinder, also elliptical) and
  `glInfiniteGrid` (shader-based ground grid with adjustable cell size and
  distance fade).
- **Linear color pipeline.** Colors are given in sRGB, lit in linear space and
  encoded back by the GPU; color textures are uploaded as sRGB.
- **Shapes**: per-object highlight (`setShininess()`, `setSpecularStrength()`)
  and `setUnlit()`; `glPoints` shading modes (`FLAT`, `DIFFUSE`, `PHONG`).
- **`glGrid`**: range mode (`initRange()`, `setRange()`) with lines on the
  multiples of a step inside a rectangle.
- **`glPlot`**: per-axis tick steps, axis crossing point, label side and
  offsets, inward ticks.
- **Window**: `setCloseOnEscape()`, `resetMouse()`; a second `create()` on the
  same window now closes the previous one.
- **Multiple windows**: text, textures and models can be drawn in more than one
  window; each window gets its own copy of the GPU resources.
- **ImGui**: one ImGui context per window; drawing helpers in `ogl::draw`.
- **`OGL_RESOURCE_DIR`**: compile-time location of shaders, fonts and data, for
  installations outside `/usr/local/include/ogl`.
- **Tooling**: `make test` and `make test_asan` (offscreen smoke test),
  `make lint` (`cppcheck` + `clang-tidy`), `make example_imgui`.

### Changed

- **Text** is drawn from a single glyph atlas texture with one draw call per
  string, instead of one texture and one draw call per character.
- The viewport follows the framebuffer size in pixels (HiDPI displays).
- Normal mapping for models (tangent space), and models with a transformed node
  hierarchy are now posed correctly.
- `expandPath()` no longer expands environment variables or shell globs.
- The library builds without warnings with `-Wall -Wextra`, and also with
  `-Wconversion -Wsign-conversion -Wshadow`.

### Fixed

- GPU resources deleted in the wrong window when an object was destroyed or
  re-initialized while another window was current.
- Re-initializing an object: stale geometry shader (WIREFRAME to SOLID), stale
  vertices in `glQuad`, stale colors in `glLines` and `glPoints`.
- `glPlot`: major ticks of a logarithmic axis overdrawn by the minor ones;
  overflow with extreme ranges or tiny tick steps (also in `glGrid`).
- `glPrint3D` dynamic scale in ORBIT camera mode.
- `glPrint2D` / `glPrint3D` built with the name only and rendered without
  `init()` read uninitialized position and scale.
- Shared textures freed while another model was still using them.
- Multi-Sample Anti-Aliasing was not requested on Linux.

### Breaking changes

- **Lights**: `setLight()` on objects and models is gone. Create a `glScene`,
  set its light and pass it to `render(camera, &scene)`.
- **Camera modes** renamed: `FREE`, `TARGET`, `BILLBOARD` are now `FLY`,
  `FIXED`, `ORBIT`.
- **Renames**: `setKeybordSpeed()` is `setKeyboardSpeed()`;
  `disableKeybord()` / `enableKeybord()` are `disableKeyboard()` /
  `enableKeyboard()`.
- **Headers**: include only `<ogl/ogl.hpp>`; the single headers refuse to be
  included directly.
- **Colors** are interpreted as sRGB: lit surfaces render darker than before
  for the same values.
- `glFont::Character_t` no longer has `TextureID` (the glyphs live in one
  atlas); `ogl::imgui::color()` returns `ImU32`.

### Known limitations

See [TODO.md](TODO.md). In short: single-threaded, windows do not share GPU
resources, textures are loaded as RGB (no alpha channel), and the Linux and
HiDPI paths are still to be verified on real hardware.
