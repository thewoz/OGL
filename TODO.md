# TODO & Known Issues

Open work items and documented limitations of OGL, moved out of the README.
Ordered with the review priority scale (Critica / Alta / Media / Bassa).

## TODO — Media

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
- **Verifica su Linux della revisione 2026-07**: l'hint `GLFW_SAMPLES` ora è
  richiesto anche su Linux (prima mancava: era il probabile motivo del "MSAA
  non funziona su Linux"); da confermare su hardware Linux. Nella stessa
  occasione: rieseguire `make test_asan` con `detect_leaks=1` (LeakSanitizer
  non è supportato su macOS) e verificare la build con GCC vero.

## TODO — Bassa

Correzioni proposte nella revisione 2026-07 e non (ancora) autorizzate:

- **FIX-004** — cast espliciti `(GLfloat)` sulle coordinate mouse/scroll in
  `glWindow.hpp` (7 warning `-Wimplicit-float-conversion` residui; benigni).
- **FIX-005** — cast espliciti `(GLsizeiptr)`/`(GLuint)` sui ~44
  `-Wsign-conversion` residui (benigni; solo se si vuole dichiarare la
  libreria warning-free anche con `-Wconversion`).
- **FIX-006** — `glShader::setUniform`: rinominare il parametro `name`
  (ombreggia il membro `name`, warning `-Wshadow`).
- **FIX-008** — `glTextures::get()`: guardia sull'indice fuori range
  (oggi `textures[index]` senza check; gli indici provengono solo da `load()`).
- **FIX-010** — doppia `create()` sulla stessa `glWindow`: oggi la seconda
  chiamata perde la finestra precedente (leak GLFW + contatori); aggiungere
  `destroy()` implicito o errore esplicito.
- **FIX-011** — `ogl::imgui::color()`: tipo di ritorno `int` → `ImU32` e cast
  esplicito dell'alpha (2 warning nella build ImGui; cambia una firma pubblica).
- **FIX-013** — `glMesh::bounds()` su mesh senza vertici restituisce valori
  spazzatura (FLT_MAX); aggiungere early-return a zero (caso remoto).
- Strumenti di analisi statica non presenti sul sistema (cppcheck,
  clang-tidy, scan-build): installarli e integrarli nel flusso
  (`brew install cppcheck llvm`).

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
- **Multi-Sample Anti-Aliasing on Linux is unverified.** Until 2026-07 the
  MSAA hint was requested only on macOS (`GLFW_SAMPLES` was inside the
  `__APPLE__` branch of `glfw::init()`), which is the most likely reason it
  "did not work" on Linux. The hint is now requested on both platforms, but
  the fix has not been verified on Linux hardware yet.
- **`glPoints` radius is not a world-space size.** The point shader computes
  `gl_PointSize = radius / depth`, so `radius` means "diameter in pixels at
  unit view distance": points shrink with distance but are never a metric
  world-space radius. For world-sized spheres use `glSphere`.
- **GL state convention: `render()` sets what it needs and restores almost
  nothing.** Each drawable enables/disables the global GL state it requires
  (culling, depth test, polygon mode) right before drawing, and only
  side-effect-heavy state (blending, point size, depth mask) is restored
  afterwards. Custom rendering code must therefore set its own state before
  drawing and must not assume a "clean" state after an OGL object rendered.
- **Textures are always loaded as RGB: the alpha channel is dropped.**
  `glTexture` forces `SOIL_LOAD_RGB`, so transparent PNGs become opaque and
  the *opacity* texture maps that `glMaterial` accepts have no effect.
  Proper support requires loading with the source channel count
  (`SOIL_LOAD_AUTO`), picking `GL_RED`/`GL_RGB`/`GL_RGBA` at upload time,
  and handling transparency in the model shader (blending plus
  back-to-front ordering of transparent objects).
- **Shared textures stay resident for the whole process.** `glTextures::load()`
  de-duplicates textures by type + file path, so materials that reference the
  same image (for the same map type) share a single GPU texture. Cache entries
  are never deleted by materials (destroying or re-initializing one model
  cannot invalidate a texture another model still uses); like the `glFont`
  glyph atlas, they are released only when the process exits. Loading many
  distinct images over a long session therefore grows GPU memory
  monotonically.
