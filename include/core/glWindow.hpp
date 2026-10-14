/*
 * GNU GENERAL PUBLIC LICENSE
 *
 * Copyright (C) 2017-2026
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _H_OGL_GLWINDOW_H_
#define _H_OGL_GLWINDOW_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>
#include <cmath>

#include <deque>
#include <string>
#include <algorithm>

//*****************************************************************************/
// namespace ogl
//*****************************************************************************/
namespace ogl {

  //*****************************************************************************/
  // glWindow
  //*****************************************************************************/
  class glWindow {

  private:

    inline void processKeyboardInput() {

      if(!keybord || !keyboardUserEnabled) return;

      bool canMoveWithKeyboard = (camera.getMode() == glCamera::MODE::FLY);

      if(camera.getMode() == glCamera::MODE::ORBIT) {
        canMoveWithKeyboard = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
      }

      if(canMoveWithKeyboard) {
        if(keys[GLFW_KEY_UP])    camera.processKeyboard(glCamera::MOVEMENT::FORWARD,  deltaTime);
        if(keys[GLFW_KEY_DOWN])  camera.processKeyboard(glCamera::MOVEMENT::BACKWARD, deltaTime);
        if(keys[GLFW_KEY_LEFT])  camera.processKeyboard(glCamera::MOVEMENT::LEFT,     deltaTime);
        if(keys[GLFW_KEY_RIGHT]) camera.processKeyboard(glCamera::MOVEMENT::RIGHT,    deltaTime);
      }

    }

    // Ultima posizione del mouse nella finestra
    GLfloat lastX = 0; GLfloat lastY = 0;

    // Variabile vera ogni volta che il mouse entra per la prima volta nella finestra
    bool firstMouse = true;

    // Variabile vera se il mouse e' sopra la finestra
    bool onFocus = false;

    // In double: dopo ore di esecuzione un float che contiene secondi ha una
    // risoluzione di millisecondi e deltaTime diventa rumoroso.
    double lastTime = 0;

    bool created = false;

    glm::vec3 background;

    bool isProcessMouseMovement = true;

    bool isFullscreen = false;

    // Windowed size saved when entering fullscreen, restored on exit. Per-instance
    // (not static) so that toggling fullscreen on one window does not clobber the
    // saved size of another.
    int windowedWidth  = 0;
    int windowedHeight = 0;

    bool keybord = false;

    // Always declared (even without ImGui) so that sizeof(glWindow) does not
    // change with OGL_WITHOUT_IMGUI: mixing translation units compiled with
    // different flags would otherwise be an ODR/layout violation.
    [[maybe_unused]] bool imguiFrameActive = false;

    // User-controlled keyboard toggle (disableKeybord/enableKeybord). Kept
    // separate from 'keybord', which is the internal "rendering has started"
    // gate raised by renderBegin(): without this split, renderBegin() would
    // re-enable input every frame and a user disableKeybord() would never stick.
    bool keyboardUserEnabled = true;

    // ESC closes the window, unless the application turns it off with
    // setCloseOnEscape(false): then ESC is only passed to keyboard() like any
    // other key, and the application decides what it does
    bool closeOnEscape = true;

    // Each on-screen window owns its own ImGui context (ImGuiContext*), created
    // in create() and destroyed with the window: this makes multiple windows
    // with independent ImGui UIs work. Offscreen windows never initialise it.
    // Stored as void* so the member exists (same class layout) even when the
    // library is built with OGL_WITHOUT_IMGUI.
    [[maybe_unused]] void * imguiCtx = nullptr;

    // FPS tracking — per-window (not static) so multiple windows don't share state.
    // One delta is sampled per frame in renderBegin(); getFPS() only reads the
    // history, so it can be called any number of times per frame.
    std::deque<double> fpsDeltas;

  protected:

    static uint32_t windowsCounter;
    static uint32_t windowsAlive;

    glCamera camera;

    bool keys[1024] = {false, };

    // Tempo passato dall'ultima volta che e' stato effettuato il rendering
    GLfloat deltaTime;

  public:

    // Puntatore della finestra GLFW
    GLFWwindow * window;

    // Id univoco della finestra
    uint32_t id = 0;

    //*****************************************************************************/
    // glWindow() - Costruttore vuoto
    //*****************************************************************************/
    glWindow() { window = NULL;}

    //*****************************************************************************/
    // ~glWindow() - Distruttore
    //*****************************************************************************/
    // Virtual: the callbacks below (scroll/keyboard/cursorPos/mouseButton/
    // cursorEnter) are virtual, so glWindow is meant to be derived from —
    // deleting a derived window through a glWindow* needs a virtual dtor.
    virtual ~glWindow() {
      DEBUG_LOG("glWindow::destroy() windowID " + std::to_string(id));
      shutdownImGui();
      if(window != NULL) { glfwDestroyWindow(window); window = NULL; }
      if(created) {
        created = false;
        if(windowsAlive > 0 && --windowsAlive == 0) { glfw::terminate(); }
      }

    }

    //*****************************************************************************/
    // create() - Crea una nuova finestra
    //*****************************************************************************/
    void create(GLint width, GLint height, const std::string & title = "OpenGL window", glCamera::MODE cameraMode = glCamera::FLY, bool resizable = false) {

      DEBUG_LOG("glWindow::create() windowID " + std::to_string(windowsCounter));

      glfw::init();

      // GLFW window hints are global and sticky, so set every per-window hint
      // explicitly: otherwise a GLFW_VISIBLE=FALSE left over from a previous
      // createOffscreen() would leak in and this window would stay hidden.
      // (The GL context hints set in glfw::init() must persist, so we do not
      // call glfwDefaultWindowHints() here.)
      glfwWindowHint(GLFW_VISIBLE,   GLFW_TRUE);
      glfwWindowHint(GLFW_RESIZABLE, resizable);
      // Ask for a default framebuffer that can do the sRGB conversion, which is
      // what lets renderBegin()'s glEnable(GL_FRAMEBUFFER_SRGB) actually take
      // effect. Required for portability: the Intel/macOS driver converts even
      // without this hint, but a driver is under no obligation to, and where
      // the default framebuffer is not sRGB-capable the enable is a silent
      // no-op and the whole scene renders too dark. The sRGB case of the smoke
      // test catches exactly that.
      glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

      // Create a GLFWwindow object that we can use for GLFW's functions
      window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

      if(window == NULL) {
        fprintf(stderr, "ERROR [glWindow]: failed to create GLFW window\n");
        glfwTerminate();
        abort();
      }

      glfwMakeContextCurrent(window);

      if(!gladLoadGL((GLADloadfunc) glfwGetProcAddress)) {
        fprintf(stderr, "ERROR [glWindow]: failed to initialize GLAD\n");
        abort();
      }

      glEnable(GL_MULTISAMPLE);

      glfwGetFramebufferSize(window, &width, &height);

      glfwSetWindowUserPointer(window, this);

      glfwSetWindowCloseCallback(window, windowCloseCallback);
      glfwSetCursorPosCallback(window, cursorPosCallback);
      glfwSetMouseButtonCallback(window, mouseButtonCallback);
      glfwSetScrollCallback(window, scrollCallback);
      glfwSetKeyCallback(window, keyCallback);
      glfwSetCursorEnterCallback(window, cursorEnterCallback);
      glfwSetWindowSizeCallback(window, sizeCallback);

      glfwSwapInterval(1);

      firstMouse = true;

      onFocus = true;

      isFullscreen = false;

      id = windowsCounter++;

      ++windowsAlive;
      created = true;

      lastTime = glfwGetTime();

      background = glm::vec3(0.0f, 0.1f, 0.2f);

      camera = glCamera(cameraMode, width, height);

      // only where glad and the driver have it, see utils/error.hpp
      enableGlDebugOutput();

      #ifndef OGL_WITHOUT_IMGUI
        // One ImGui context per window. The GLFW/OpenGL3 backends store their
        // state inside the current context, so as long as the right context is
        // made current (renderBegin/renderEnd do it) every window gets its own
        // independent UI. Note: ImGui_ImplGlfw chains the callbacks installed
        // above, so this must run after the glfwSet*Callback calls.
        IMGUI_CHECKVERSION();
        ImGuiContext * ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui::GetIO().IniFilename = nullptr;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 150");
        ImGui::StyleColorsDark();
        imguiCtx = ctx;
      #endif

    }

    //*****************************************************************************/
    // createOffscreen() - Crea una nuova finestra offscreen
    //*****************************************************************************/
    void createOffscreen(GLint width, GLint height, glCamera::MODE cameraMode = glCamera::FLY) {

      DEBUG_LOG("glWindow::createOffscreen() windowID " + std::to_string(windowsCounter));

      glfw::init();

      // Others Glfw options
      glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      // Ask for a default framebuffer that can do the sRGB conversion, which is
      // what lets renderBegin()'s glEnable(GL_FRAMEBUFFER_SRGB) actually take
      // effect. Required for portability: the Intel/macOS driver converts even
      // without this hint, but a driver is under no obligation to, and where
      // the default framebuffer is not sRGB-capable the enable is a silent
      // no-op and the whole scene renders too dark. The sRGB case of the smoke
      // test catches exactly that.
      glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

      // Create a GLFWwindow object that we can use for GLFW's functions
      window = glfwCreateWindow(width, height, "notitle", NULL, NULL);

      if(window == NULL) {
        fprintf(stderr, "ERROR [glWindow]: failed to create GLFW window\n");
        glfwTerminate();
        abort();
      }

      glfwMakeContextCurrent(window);

      if(!gladLoadGL((GLADloadfunc) glfwGetProcAddress)) {
        fprintf(stderr, "ERROR [glWindow]: failed to initialize GLAD\n");
        abort();
      }

      glEnable(GL_MULTISAMPLE);

      glfwGetFramebufferSize(window, &width, &height);

      glfwSetWindowUserPointer(window, this);

      glfwSetWindowCloseCallback(window, windowCloseCallback);
      glfwSetCursorPosCallback(window, cursorPosCallback);
      glfwSetMouseButtonCallback(window, mouseButtonCallback);
      glfwSetScrollCallback(window, scrollCallback);
      glfwSetKeyCallback(window, keyCallback);
      glfwSetCursorEnterCallback(window, cursorEnterCallback);
      glfwSetWindowSizeCallback(window, sizeCallback);

      glfwSwapInterval(1);

      firstMouse = true;

      onFocus = false;

      isFullscreen = false;

      id = windowsCounter++;

      ++windowsAlive;
      created = true;

      lastTime = glfwGetTime();

      background = glm::vec3(0.0f, 0.1f, 0.2f);

      camera = glCamera(cameraMode, width, height);

      // Same GL debug output as create(): headless/batch rendering is where
      // driver diagnostics are most valuable. Only where glad and the driver
      // have it, see utils/error.hpp
      enableGlDebugOutput();

    }

    //*****************************************************************************/
    // destroy() -
    //*****************************************************************************/
    void destroy() {

      DEBUG_LOG("glWindow::destroy() windowID " + std::to_string(id));

      shutdownImGui();

      if(window != NULL) {
        glfwDestroyWindow(window);
        window = NULL;
      }

      if(created) {
        created = false;
        if(windowsAlive > 0 && --windowsAlive == 0) { glfw::terminate(); }
      }

    }

  private:

    //****************************************************************************//
    // shutdownImGui() - tear down this window's own ImGui context
    //****************************************************************************//
    inline void shutdownImGui() {
      #ifndef OGL_WITHOUT_IMGUI
        if(imguiCtx != nullptr) {
          // Il backend GL3 di ImGui distrugge le sue risorse nel contesto GL
          // *corrente*: rendo corrente quello di questa finestra, così con più
          // finestre (distrutte in qualsiasi ordine) le risorse sono rilasciate
          // nel contesto che le possiede.
          if(window != NULL) glfwMakeContextCurrent(window);
          ImGui::SetCurrentContext((ImGuiContext*)imguiCtx);
          ImGui_ImplOpenGL3_Shutdown();
          ImGui_ImplGlfw_Shutdown();
          ImGui::DestroyContext((ImGuiContext*)imguiCtx);
          imguiCtx = nullptr;
        }
      #endif
    }

  public:

    //****************************************************************************//
    // getCamera() - returns a reference to the window's camera
    //****************************************************************************//
    glCamera & getCamera() { return camera; }
    const glCamera & getCamera() const { return camera; }

    //****************************************************************************//
    // setCamera() - reconfigure the camera (mode, fov, position, target)
    //****************************************************************************//
    void setCamera(glCamera::MODE mode, float fov = 45.0f, glm::vec3 position = glm::vec3(0.0f), glm::vec3 target = glm::vec3(0.0f)) {
      GLsizei w = camera.getWidth();
      GLsizei h = camera.getHeight();
      camera = glCamera(mode, w, h, fov, position, target);
    }

  private:

    //****************************************************************************//
    // imguiWantsMouse() - true when the cursor is over an ImGui widget/window,
    // so camera and scroll input must be suppressed while the user drives the UI.
    //****************************************************************************//
    inline bool imguiWantsMouse() const {
      #ifndef OGL_WITHOUT_IMGUI
        if(imguiCtx != nullptr) {
          ImGui::SetCurrentContext((ImGuiContext*)imguiCtx);
          return ImGui::GetIO().WantCaptureMouse;
        }
      #endif
      return false;
    }

    //****************************************************************************//
    // CallBack GLFW Wrapper
    //****************************************************************************//
    static inline void windowCloseCallback(GLFWwindow* window) {
      ((glWindow*)glfwGetWindowUserPointer(window))->windowCloseCallback();
    }

    static inline void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
      ((glWindow*)glfwGetWindowUserPointer(window))->cursorPosCallback(xPos, yPos);
    }

    static inline void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
      ((glWindow*)glfwGetWindowUserPointer(window))->mouseButtonCallback(button, action, mods);
    }

    static inline void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
      ((glWindow*)glfwGetWindowUserPointer(window))->scrollCallback(xoffset, yoffset);
    }

    static inline void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
      ((glWindow*)glfwGetWindowUserPointer(window))->keyCallback(key, scancode, action, mods);
    }

    static inline void cursorEnterCallback(GLFWwindow* window, int entered) {
      ((glWindow*)glfwGetWindowUserPointer(window))->cursorEnter(entered);
    }

    static inline void sizeCallback(GLFWwindow* window, int width, int height) {
      ((glWindow*)glfwGetWindowUserPointer(window))->sizeCallback(width, height);
    }

    //****************************************************************************//
    // External callback interfaces
    //****************************************************************************//
    inline void virtual scroll(double /*xoffset*/, double /*yoffset*/) { };
    inline void virtual keyboard(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/) { };
    inline void virtual cursorPos(double /*xPos*/, double /*yPos*/, double /*xoffset*/, double /*yoffset*/) { };
    inline void virtual mouseButton(int /*button*/, int /*action*/, int /*mods*/) { };
    inline void virtual cursorEnter(int entered) { if(entered) { onFocus = true; } else { onFocus = false; } }

    //****************************************************************************//
    // Callback
    //****************************************************************************//
    inline void windowCloseCallback() {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }

    inline void sizeCallback(int width, int height) {
      camera.setViewport(width, height);
    }

    inline void scrollCallback(double xoffset, double yoffset) {
      if(imguiWantsMouse()) return;
      camera.processMouseScroll(yoffset);
      scroll(xoffset, yoffset);
    }

    inline void keyCallback(int key, int scancode, int action, int mods) {

      // Track key state unconditionally: if a RELEASE arrived while input was
      // disabled and got dropped, keys[] would stay true forever and the camera
      // would keep moving after the keyboard is re-enabled.
      if(key >= 0 && key < 1024) {
        if(action == GLFW_PRESS)   keys[key] = true;
        if(action == GLFW_RELEASE) keys[key] = false;
      }

      if(!keybord || !keyboardUserEnabled) return;

      if(closeOnEscape && GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
        setShouldClose(GL_TRUE);
      }

      if(key >= 0 && key < 1024) {
        keyboard(key, scancode, action, mods);
      }

    }

    inline void cursorPosCallback(double xPos, double yPos) {

      if(onFocus) {

        if(firstMouse) {
          lastX = xPos;
          lastY = yPos;
          firstMouse = false;
        }

        GLfloat xOffset = xPos - lastX;
        GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

        lastX = xPos;
        lastY = yPos;

        bool controlKey = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)  == GLFW_PRESS ||
                           glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

        if(isProcessMouseMovement && !imguiWantsMouse())
          camera.processMouseMovement(xOffset, yOffset, controlKey);

        cursorPos(lastX, lastY, xOffset, yOffset);

      }

    }

    inline void mouseButtonCallback(int button, int action, int mods) {
      if(imguiWantsMouse()) return;
      mouseButton(button, action, mods);
    }

  public:

    //****************************************************************************//
    // Window Interface funtions
    //****************************************************************************//
    inline void setPosition(int xPos, int yPos) {
      glfwSetWindowPos(window, xPos, yPos);
    }

    inline void setTitle(const std::string & title) {
      glfwSetWindowTitle(window, title.c_str());
    }

    inline bool shouldClose() {
      return glfwWindowShouldClose(window);
    }

    inline void setShouldClose(bool mode) {
      glfwSetWindowShouldClose(window, mode);
    }

    inline void setCursorInputMode(int mode) {
      glfwSetInputMode(window, GLFW_CURSOR, mode);
    }

    inline void getCursorPos(double & x, double & y) {
      glfwGetCursorPos(window, &x, &y);
    }

    inline void setSize(int width, int height) {
      glfwSetWindowSize(window, width, height);
      camera.setViewport(width, height);
    }

    inline void setBackground(const glm::vec3 & _background) {
      background = _background;
    }

    //*****************************************************************************/
    // disable/enable Keyboard
    //*****************************************************************************/
    void disableKeyboard() { keyboardUserEnabled = false; }
    void enableKeyboard()  { keyboardUserEnabled = true;  }

    // whether ESC closes the window (the default); see closeOnEscape
    void setCloseOnEscape(bool close) { closeOnEscape = close; }

    // Deprecated misspelled aliases, kept for source compatibility.
    void disableKeybord() { disableKeyboard(); }
    void enableKeybord()  { enableKeyboard();  }

    //*****************************************************************************/
    // disable/enable mouseOnCamera()
    //*****************************************************************************/
    void disableMouseOnCamera() { isProcessMouseMovement = false; }
    void enableMouseOnCamera()  { isProcessMouseMovement = true;  }

    //*****************************************************************************/
    // resetMouse() - forget the last cursor position so the next motion produces
    // no offset. Call this after re-capturing the cursor (e.g. closing a UI panel)
    // to avoid a sudden camera jump.
    //*****************************************************************************/
    void resetMouse() { firstMouse = true; }

    inline void hide() { glfwHideWindow(window); }
    inline void show() { glfwShowWindow(window); }
    inline void iconify() { glfwIconifyWindow(window); }

    //*****************************************************************************/
    // FPS
    //*****************************************************************************/
    inline int getFPS() const {

      if(fpsDeltas.empty()) return 0;

      double sum = 0;
      for(std::size_t i = 0; i < fpsDeltas.size(); ++i) sum += fpsDeltas[i];

      if(sum <= 0.0) return 0;

      return static_cast<int>(static_cast<double>(fpsDeltas.size()) / sum);

    }

    //*****************************************************************************/
    // get projection and view matrix
    //*****************************************************************************/
    inline glm::mat4 getProjection()      const { return camera.getProjection(); }
    inline glm::mat4 getOrthoProjection() const { return camera.getOrthoProjection(); }

    //*****************************************************************************/
    // getViewport()
    //*****************************************************************************/
    inline glm::vec2 getViewport() const { return camera.getViewport(); }

    //*****************************************************************************/
    // getView()
    //*****************************************************************************/
    inline glm::mat4 getView() const { return camera.getView(); }

    //*****************************************************************************/
    // getPitch() getYaw()
    //*****************************************************************************/
    inline float getPitch() { return camera.getPitch(); }
    inline float getYaw()   { return camera.getYaw();   }

    //*****************************************************************************/
    // getCameraPosition()
    //*****************************************************************************/
    inline glm::vec3 getCameraPosition() { return camera.getPosition(); }

    //*****************************************************************************/
    // isOnRetinaDisplay()
    //*****************************************************************************/
    bool isOnRetinaDisplay() {

      float xScale, yScale;

      glfwGetWindowContentScale(window, &xScale, &yScale);

      // Any HiDPI/Retina backing (scale > 1) counts; avoids an exact == 2 test
      // that would miss fractional scale factors (1.5x, 2.5x, ...). The two
      // axes practically always match; use the larger one instead of aborting.
      return (std::max(xScale, yScale) > 1.0f);

    }

    //*****************************************************************************/
    // srgbToLinear - exact sRGB transfer function, the C++ counterpart of the
    // one in the shaders. Needed for the colors that reach the framebuffer
    // without passing through a shader of ours, i.e. the clear color
    //*****************************************************************************/
    static glm::vec3 srgbToLinear(const glm::vec3 & color) {

      glm::vec3 value;

      for(int i=0; i<3; ++i)
        value[i] = (color[i] <= 0.04045f) ? color[i] / 12.92f
                                          : std::pow((color[i] + 0.055f) / 1.055f, 2.4f);

      return value;

    }

    //*****************************************************************************/
    // renderBegin()
    //*****************************************************************************/
    void renderBegin() {

      if(window == NULL) {
        fprintf(stderr, "ERROR [glWindow]: window is not initialized\n");
        abort();
      }

      glfwMakeContextCurrent(window);

      glfwPollEvents();

      double currentTime = glfwGetTime();

      // Solo la differenza (piccola) scende a float; i tempi assoluti restano double.
      deltaTime = GLfloat(currentTime - lastTime);
      lastTime  = currentTime;

      // Sample the FPS history exactly once per frame; getFPS() is a pure getter.
      if(fpsDeltas.size() >= 60) fpsDeltas.pop_front();
      fpsDeltas.push_back(deltaTime);

      processKeyboardInput();

      glViewport(0, 0, camera.getWidth(), camera.getHeight());

      // The lit shaders output LINEAR color and the unlit ones linearize theirs,
      // so the sRGB encoding of every framebuffer write is the GPU's job. This
      // enable is what turns it on and is REQUIRED on every platform: without
      // it nothing encodes and the whole scene renders too dark. It is set per
      // frame rather than once at creation because the flag is context state,
      // so this way it holds for every window regardless of creation order.
      //
      // The companion piece is the GLFW_SRGB_CAPABLE hint in create() and
      // createOffscreen(), which asks for a default framebuffer that can do the
      // conversion. That hint is what makes this work outside macOS: the Intel
      // driver here converts even without it, but a driver is not required to,
      // and where the default framebuffer is not sRGB-capable this enable is a
      // silent no-op.
      //
      // Do NOT try to check the capability with
      // GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING: on this driver it reports
      // GL_LINEAR for the default framebuffer while the conversion does in fact
      // happen, so it yields a false negative. The reliable check is to render
      // a known color and read the pixel back, which is what the sRGB case of
      // the smoke test does (test/smoke.cpp): if a platform ever fails to
      // encode, that test fails instead of the scene quietly going dark.
      glEnable(GL_FRAMEBUFFER_SRGB);
      //
      // ORDER MATTERS: this has to come BEFORE the glClear below. renderEnd()
      // turns the encoding off to draw ImGui, so at the start of every frame it
      // is off, and a clear done first is not encoded: the linearized background
      // would come out far too dark (setBackground(0.1) as 3/255 instead of
      // 26/255). Without ImGui the state just stays on, which hides the problem
      // from a headless run: it only shows in a real window.

      // The clear is a framebuffer write like any other, so GL_FRAMEBUFFER_SRGB
      // encodes it too: the background has to be linearized here or a
      // setBackground(0.1) would come out at 0.35 on screen.
      glm::vec3 clear = srgbToLinear(background);

      glClearColor(clear.r, clear.g, clear.b, 1.0f);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);



      keybord = true;

      #ifndef OGL_WITHOUT_IMGUI
        int monitorCount = 0;
        glfwGetMonitors(&monitorCount);
        if(imguiCtx != nullptr && monitorCount > 0) {
          ImGui::SetCurrentContext((ImGuiContext*)imguiCtx);
          ImGui_ImplOpenGL3_NewFrame();
          ImGui_ImplGlfw_NewFrame();
          ImGui::NewFrame();
          imguiFrameActive = true;
        } else {
          imguiFrameActive = false;
        }
      #endif

    }

    //*****************************************************************************/
    // renderEnd()
    //*****************************************************************************/
    inline void renderEnd() {

      #ifndef OGL_WITHOUT_IMGUI
        if(imguiFrameActive) {
          ImGui::SetCurrentContext((ImGuiContext*)imguiCtx);
          ImGui::Render();
          // ImGui's own shader writes colors that are already display-referred,
          // and it is not one of ours to linearize: the sRGB encoding of the
          // framebuffer has to be off for its draw, or the whole UI comes out
          // washed out. renderBegin() turns it back on for the next frame.
          glDisable(GL_FRAMEBUFFER_SRGB);
          ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
          imguiFrameActive = false;
        }
      #endif

      glfwSwapBuffers(window);

    }

    //*****************************************************************************/
    // snapshot()
    //*****************************************************************************/
    void snapshot(std::string path) {

      glReadBuffer(GL_BACK);

      ogl::io::expandPath(path);

      ogl::snapshot(camera.getWidth(), camera.getHeight(), path.c_str());

    }

    //****************************************************************************
    // toggleFullscreen()
    //****************************************************************************
    void toggleFullscreen() {

      int & oldWidth  = windowedWidth;
      int & oldHeight = windowedHeight;

      GLFWmonitor * monitor = glfwGetPrimaryMonitor();

      const GLFWvidmode * mode = glfwGetVideoMode(monitor);

      if(!isFullscreen) {

        // Save the windowed size in screen coordinates. Asking GLFW directly is
        // correct on every display; deriving it from the framebuffer size would
        // require assuming a fixed content scale (e.g. exactly 2x on Retina).
        glfwGetWindowSize(window, &oldWidth, &oldHeight);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        camera.setViewport(width, height);

        glfwFocusWindow(window);

      } else {

        glfwSetWindowMonitor(window, NULL, 0, 0, oldWidth, oldHeight, 0);

        int width, height;

        glfwGetFramebufferSize(window, &width, &height);

        camera.setViewport(width, height);

      }

      isFullscreen = !isFullscreen;

    }

  };

  inline uint32_t glWindow::windowsCounter = 0;
  inline uint32_t glWindow::windowsAlive   = 0;

  //*****************************************************************************/
  // currentWindowID() - id of the glWindow owning the current GL context.
  // Fails with a clear message (instead of dereferencing NULL) when no OGL
  // window/context is current, e.g. rendering before create() or from a thread
  // without a context.
  //*****************************************************************************/
  inline uint32_t currentWindowID() {

    GLFWwindow * ctx = glfwGetCurrentContext();

    void * ptr = (ctx != NULL) ? glfwGetWindowUserPointer(ctx) : NULL;

    if(ptr == NULL) {
      fprintf(stderr, "ERROR [ogl]: no active OGL window/context "
                      "(create a glWindow and call renderBegin() first)\n");
      abort();
    }

    return static_cast<glWindow*>(ptr)->id;

  }

} /* namespace ogl */

#endif /* _H_OGL_GLWINDOW_H_ */
