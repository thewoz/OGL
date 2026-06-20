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

#include <deque>
#include <vector>
#include <string>
#include <memory>

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

      if(!keybord || currentCamera == nullptr) return;

      bool canMoveWithKeyboard = (currentCamera->getMode() == glCamera::MODE::FLY);

      if(currentCamera->getMode() == glCamera::MODE::PAN) {
        canMoveWithKeyboard = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
      }

      if(canMoveWithKeyboard) {

        // Keep camera movement frame-based to avoid key-repeat stuttering.
        if(/*keys[GLFW_KEY_W] || */ keys[GLFW_KEY_UP])    currentCamera->processKeyboard(glCamera::MOVEMENT::FORWARD, deltaTime);
        if(/*keys[GLFW_KEY_S] || */ keys[GLFW_KEY_DOWN])  currentCamera->processKeyboard(glCamera::MOVEMENT::BACKWARD, deltaTime);
        if(/*keys[GLFW_KEY_A] || */ keys[GLFW_KEY_LEFT])  currentCamera->processKeyboard(glCamera::MOVEMENT::LEFT, deltaTime);
        if(/*keys[GLFW_KEY_D] || */ keys[GLFW_KEY_RIGHT]) currentCamera->processKeyboard(glCamera::MOVEMENT::RIGHT, deltaTime);

      }

      if(cameras.size() > 1 && keys[GLFW_KEY_C]) {
        changeCamera();
      }

    }
    
    // Indice della camera corrente
    unsigned int currentCameraIndex;
    
    // Ultima posizione del mouse nella finestra
    GLfloat lastX; GLfloat lastY;
    
    // Variabile vera ogni volta che il mouse entra per la prima volta nella finestra
    bool firstMouse;
    
    // Variabile vera se il mouse e' sopra la finestra
    bool onFocus;
    
    // Cameras are polymorphic (one subclass per mode), so they are owned through
    // pointers. currentCamera is a non-owning observer into this vector.
    std::vector<std::unique_ptr<ogl::glCamera>> cameras;

    GLfloat lastTime = 0;

    bool created = false;

    glm::vec3 background;
        
    bool isProcessMouseMovement = true;
    
    bool isFullscreen;
    
    bool keybord = false;

    bool imguiFrameActive = false;

    // ImGui keeps a single global context, so it is created exactly once (by the
    // first on-screen window) and torn down by that same window. Offscreen
    // windows never initialise it. 'imguiOwner' marks the window responsible for
    // the shutdown; 'imguiInitialized' is the shared "context exists" flag.
    static bool imguiInitialized;
    bool imguiOwner = false;

    // FPS tracking — per-window (not static) so multiple windows don't share state.
    std::deque<double> fpsDeltas;
    double fpsLastTimestamp = 0.0;

  protected:
    
    static uint32_t windowsCounter;
    static uint32_t windowsAlive;

    ogl::glCamera * currentCamera;
    
    bool keys[1024] = {false, };

    // Tempo passato dall'ultima volta che e' stato effettuato il rendering
    GLfloat deltaTime;
    
  public:
    
    // Puntatore della finestra GLFW
    GLFWwindow * window;
    
    // Id univoco della finestra
    uint32_t id;
    
    //*****************************************************************************/
    // glWindow() - Costruttore vuoto
    //*****************************************************************************/
    glWindow() { window = NULL;}
    
    //*****************************************************************************/
    // ~glWindow() - Distruttore
    //*****************************************************************************/
    ~glWindow() {
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
    void create(GLint width, GLint height, bool resizable = false, const char * title = "OpenGL window") {
      
      DEBUG_LOG("glWindow::create() windowID " + std::to_string(windowsCounter));
      
      glfw::init();
            
      // Others Glfw options
      glfwWindowHint(GLFW_RESIZABLE, resizable);

      // Funziona con OpenGL >= 4.3
      // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

      // Create a GLFWwindow object that we can use for GLFW's functions
      window = glfwCreateWindow(width, height, title, NULL, NULL);
      
      if(window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        abort();
      }
      
      glfwMakeContextCurrent(window);
      
      if(!gladLoadGL((GLADloadfunc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
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

      cameras.push_back(std::make_unique<glCameraFly>(width, height));

      currentCameraIndex = 0;

      currentCamera = cameras[currentCameraIndex].get();

      #ifndef __APPLE__
          glEnable(GL_DEBUG_OUTPUT);
          glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
          glDebugMessageCallback(glDebugOutput, 0);
      #endif
      
      #ifndef OGL_WITHOUT_IMGUI
        // ImGui has a single global context: initialise it only once, and let
        // this window own its lifetime.
        if(!imguiInitialized) {
          IMGUI_CHECKVERSION();
          ImGui::CreateContext();
          ImGui::GetIO().IniFilename = nullptr;  // <-- DISABILITA imgui.ini
          ImGui_ImplGlfw_InitForOpenGL(window, true);
          ImGui_ImplOpenGL3_Init("#version 150");
          ImGui::StyleColorsDark();
          imguiInitialized = true;
          imguiOwner       = true;
        }
      #endif

    }
    
    //*****************************************************************************/
    // createOffscreen() - Crea una nuova finestra
    //*****************************************************************************/
    void createOffscreen(GLint width, GLint height) {
      
      DEBUG_LOG("glWindow::createOffscreen() windowID " + std::to_string(windowsCounter));

      glfw::init();
      
      // Others Glfw options
      glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
      glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
      
      // Funziona con OpenGL >= 4.3
      // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
      
      // Create a GLFWwindow object that we can use for GLFW's functions
      window = glfwCreateWindow(width, height, "notitle", NULL, NULL);
      
      if(window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        abort();
      }
      
      glfwMakeContextCurrent(window);
      
      if(!gladLoadGL((GLADloadfunc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
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

      cameras.push_back(std::make_unique<glCameraFly>(width, height));

      currentCameraIndex = 0;

      currentCamera = cameras[currentCameraIndex].get();

      // Funziona con OpenGL >= 4.3
      // glDebugMessageCallback((GLDEBUGPROC)glDebugOutput, NULL);
      // glEnable(GL_DEBUG_OUTPUT);
      // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      
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
    // makeCamera() - build the camera subclass matching the requested mode
    //****************************************************************************//
    static std::unique_ptr<glCamera> makeCamera(glCamera::MODE mode, GLsizei width, GLsizei height, GLfloat fov, const glm::vec3 & position, const glm::vec3 & target) {
      switch(mode) {
        case glCamera::ORBIT: return std::make_unique<glCameraOrbit>(width, height, fov, position, target);
        case glCamera::PAN:   return std::make_unique<glCameraPan>(width, height, fov, position, target);
        case glCamera::FLY:
        default:              return std::make_unique<glCameraFly>(width, height, fov, position);
      }
    }

    //****************************************************************************//
    // shutdownImGui() - tear down ImGui, but only from the window that created it
    //****************************************************************************//
    inline void shutdownImGui() {
      #ifndef OGL_WITHOUT_IMGUI
        if(imguiOwner) {
          ImGui_ImplOpenGL3_Shutdown();
          ImGui_ImplGlfw_Shutdown();
          ImGui::DestroyContext();
          imguiOwner       = false;
          imguiInitialized = false;
        }
      #endif
    }

  public:
    
    //****************************************************************************//
    // updateCurrentCamera() -
    //****************************************************************************//
    void updateCurrentCamera(float fov, glm::vec3 position = glm::vec3(0.0f), glCamera::MODE mode = glCamera::MODE::FLY, glm::vec3 target = glm::vec3(0.0f)) {

      // Changing mode means swapping the camera for a different subclass, keeping
      // the same viewport.
      GLsizei w = currentCamera->getWidth();
      GLsizei h = currentCamera->getHeight();

      cameras[currentCameraIndex] = makeCamera(mode, w, h, fov, position, target);

      currentCamera = cameras[currentCameraIndex].get();

    }
    
    //****************************************************************************//
    // getCurrentCamera() -
    //****************************************************************************//
    ogl::glCamera * getCurrentCamera() { return currentCamera; }
      
    
  private:
    
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
    inline void virtual scroll(double xoffset, double yoffset) { };
    inline void virtual keyboard(int key, int scancode, int action, int mods) { };
    inline void virtual cursorPos(double xPos, double yPos, double xoffset, double yoffset) { };
    inline void virtual mouseButton(int button, int action, int mods) { };
    inline void virtual cursorEnter(int entered) { if(entered) { onFocus = true; } else { onFocus = false; } }
    
    //****************************************************************************//
    // Callback
    //****************************************************************************//
    inline void windowCloseCallback() {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    inline void sizeCallback(int width, int height) {
      for(std::size_t i=0; i<cameras.size(); ++i)
        cameras[i]->setViewport(width, height);
    }
    
    inline void scrollCallback(double xoffset, double yoffset) {
      
      currentCamera->processMouseScroll(yoffset);
      
      scroll(xoffset, yoffset);
      
    }
        
    inline void keyCallback(int key, int scancode, int action, int mods) {
                  
      if(!keybord) return;
                  
      if(GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
        setShouldClose(GL_TRUE);
      }
      
      if(key >= 0 && key < 1024) {
        
        if(action == GLFW_PRESS) {
          keys[key] = true;
        } else if (action == GLFW_RELEASE) {
          keys[key] = false;
        }

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
        
        GLfloat xOffset;
        GLfloat yOffset;
        
        xOffset = xPos - lastX;
        yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
        
        lastX = xPos;
        lastY = yPos;
        
        bool controllKey = GLFW_RELEASE;
        if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
          controllKey = GLFW_PRESS;
        
        if(isProcessMouseMovement) currentCamera->processMouseMovement(xOffset, yOffset, controllKey);
        
        cursorPos(lastX, lastY, xOffset, yOffset);
      
      }
        
    }
    
    inline void mouseButtonCallback(int button, int action, int mods) { mouseButton(button, action, mods); }
    
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

//    inline bool isImGuiFrameActive() const {
//      return imguiFrameActive;
//    }
    
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
      for(std::size_t i=0; i<cameras.size(); ++i)
        cameras[i]->setViewport(width, height);
    }
    
    inline void setBackground(const glm::vec3 & _background) {
      background = _background;
    }
    
    //inline void disableInput() { inputDisable = true; }
    //inline void enableInput() { inputDisable = false; }

    //*****************************************************************************/
    // disable/enable Keybord
    //*****************************************************************************/
    void disableKeybord() { keybord = false; }
    void enableKeybord()  { keybord = true;  }

    //*****************************************************************************/
    // disable/enable mouseOnCamera() -
    //*****************************************************************************/
    void disableMouseOnCamera() { isProcessMouseMovement = false; }
    void enableMouseOnCamera()  { isProcessMouseMovement = true;  }
    
    inline void hide() { glfwHideWindow(window); }
    inline void show() { glfwShowWindow(window); }
    inline void iconify() { glfwIconifyWindow(window); }

    //*****************************************************************************/
    // FPS
    //*****************************************************************************/
    inline int getFPS() {

      double now = glfwGetTime();

      if(fpsLastTimestamp == 0.0) { fpsLastTimestamp = now; return 0; }

      double delta = now - fpsLastTimestamp;
      fpsLastTimestamp = now;

      if(fpsDeltas.size() < 60) fpsDeltas.push_back(delta);
      else {
        fpsDeltas.pop_front();
        fpsDeltas.push_back(delta);
      }

      double sum = 0;
      for(std::size_t i = 0; i < fpsDeltas.size(); ++i) sum += fpsDeltas[i];

      if(sum <= 0.0) return 0;

      return static_cast<int>(1.0 / (sum / static_cast<double>(fpsDeltas.size())));

    }
    
    //****************************************************************************//
    // addCamera()
    //****************************************************************************//
    inline void addCamera(float fov, glm::vec3 position = glm::vec3(0.0f), glCamera::MODE mode = glCamera::FLY, glm::vec3 target = glm::vec3(0.0f)) {
      cameras.push_back(makeCamera(mode, currentCamera->getWidth(), currentCamera->getHeight(), fov, position, target));
      // unique_ptr storage keeps the existing cameras alive across reallocation,
      // so currentCamera stays valid; refresh it for clarity.
      currentCamera = cameras[currentCameraIndex].get();
    }

    //*****************************************************************************/
    // get projection and view matrix
    //*****************************************************************************/
    inline glm::mat4 getProjection()      const { return currentCamera->getProjection(); }
    inline glm::mat4 getOrthoProjection() const { return currentCamera->getOrthoProjection(); }

    //*****************************************************************************/
    // getViewport()
    //*****************************************************************************/
    inline glm::vec2 getViewport() const { return currentCamera->getViewport(); }
    
    //*****************************************************************************/
    // getView()
    //*****************************************************************************/
    inline glm::mat4 getView() const { return currentCamera->getView(); }

    //*****************************************************************************/
    // getPitch() getYaw()
    //*****************************************************************************/
    inline float getPitch() { return currentCamera->getPitch(); }
    inline float getYaw() { return currentCamera->getYaw(); }
    
    //*****************************************************************************/
    // getCameraPosition()
    //*****************************************************************************/
    inline glm::vec3 getCameraPosition() { return currentCamera->getPosition(); }
    
    //*****************************************************************************/
    // isOnRetinaDisplay()
    //*****************************************************************************/
    bool isOnRetinaDisplay() {
      
      float xScale, yScale;
      
      glfwGetWindowContentScale(window, &xScale, &yScale);

      if(xScale != yScale) {
        fprintf(stderr, "error in isOnRetinaDisplay()\n");
        abort();
      }

      return (xScale == 2);
      
    }

    //*****************************************************************************/
    // renderBegin()
    //*****************************************************************************/
    void renderBegin() {
      
      if(window == NULL) {
        fprintf(stderr, "glWindow error: the window is not initialized\n");
        abort();
      }
      
      glfwMakeContextCurrent(window);
      
      glfwPollEvents();
      
      GLfloat currentTime = glfwGetTime();
      
      deltaTime = currentTime - lastTime;
      lastTime  = currentTime;

      processKeyboardInput();
                        
      glViewport(0, 0, currentCamera->getWidth(), currentCamera->getHeight());

      glClearColor(background.r, background.g, background.b, 1.0f);
      
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);

      keybord = true;

      #ifndef OGL_WITHOUT_IMGUI
        // On macOS the system can drop all monitors after a display sleep, and ImGui::NewFrame() asserts when the monitor list is empty.
        // Skip the ImGui frame until at least one monitor is back.
        // Offscreen windows never initialise ImGui, so guard on imguiInitialized too.
        int monitorCount = 0;
        glfwGetMonitors(&monitorCount);
        if(imguiInitialized && monitorCount > 0) {
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
          ImGui::Render();
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
      
      ogl::snapshot(currentCamera->getWidth(), currentCamera->getHeight(), path.c_str());
      
    }
  
    //****************************************************************************
    // toggleFullscreen()
    //****************************************************************************
    void toggleFullscreen() {
      
      static int oldWidth;
      static int oldHeight;

      GLFWmonitor * monitor = glfwGetPrimaryMonitor();
      
      const GLFWvidmode * mode = glfwGetVideoMode(monitor);
      
      if(!isFullscreen) {
        
        getCurrentCamera()->getViewport(oldWidth, oldHeight);

#ifdef __APPLE__
        oldWidth *= 0.5; oldHeight *= 0.5;
#endif

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        
        int width, height;
        
        glfwGetFramebufferSize(window, &width, &height);
        
        getCurrentCamera()->setViewport(width, height);
        
        glfwFocusWindow(window);
                
      } else {
        
        glfwSetWindowMonitor(window, NULL, 0, 0, oldWidth, oldHeight, 0);

        glfwGetFramebufferSize(window, &oldWidth, &oldHeight);
        
        getCurrentCamera()->setViewport(oldWidth, oldHeight);
        
      }
      
      isFullscreen = !isFullscreen;
            
    }
    
    
    //*****************************************************************************/
    // changeCamera()
    //*****************************************************************************/
    void changeCamera() {
      
      if(currentCameraIndex + 1 < cameras.size()) { ++currentCameraIndex; } else { currentCameraIndex = 0; }
      
      currentCamera = cameras[currentCameraIndex].get();

    }
    
  };
  
  inline uint32_t glWindow::windowsCounter = 0;
  inline uint32_t glWindow::windowsAlive   = 0;
  inline bool     glWindow::imguiInitialized = false;

} /* namespace ogl */

#endif /* _H_OGL_GLWINDOW_H_ */
