/*
 * GNU GENERAL PUBLIC LICENSE
 *
 * Copyright (C) 2019
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

#include <cstdlib>
#include <cstdio>

#include <deque>
#include <vector>
#include <string>

//*****************************************************************************/
// namespace ogl
//*****************************************************************************/
namespace ogl {
  
  //*****************************************************************************/
  // glWindow
  //*****************************************************************************/
  class glWindow {
    
  private:
    
    // Indice della camera corrente
    unsigned int currentCameraIndex;
    
    // Ultima posizione del mouse nella finestra
    GLfloat lastX; GLfloat lastY;
    
    // Variabile vera ogni volta che il mouse entra per la prima volta nella finestra
    bool firstMouse;
    
    // Variabile vera se il mouse e' sopra la finestra
    bool onFocus;
    
    // Vettore delle telecamere attive nella finestra
    std::vector<ogl::glCamera> cameras;
        
    // Tempo dell'ultimo rendering
    GLfloat lastTime;
    
    // Memoria di appoggio per salvare il contenuto della finestra
    GLubyte * image;

    // Background color
    glm::vec3 background;
    
    //bool inputDisable;
    
    bool isProcessMouseMovement = true;
    
    bool isFullscreen;
    
    bool keybord = false;

  protected:
    
    // Contatore del numero di finestre create
    static uint32_t windowsCounter;
    
    // Camera corrente
    ogl::glCamera * currentCamera;
    
    bool keys[1024] = {false, };

    // Tempo passato dall'ultima volta che e' stato effettuato il rendering
    GLfloat deltaTime;
    
    //TODO:
    // mettere un controllo su offscreen
    
  public:
    
    // Puntatore della finestra GLFW
    GLFWwindow * window;
    
    // Id univoco della finestra
    uint32_t id;
    
    //*****************************************************************************/
    // glWindow() - Costruttore vuoto
    //*****************************************************************************/
    glWindow() { window = NULL; image = NULL; }
    
    //*****************************************************************************/
    // ~glWindow() - Distruttore
    //*****************************************************************************/
    ~glWindow() {
      DEBUG_LOG("glWindow::destroy() windowID " + std::to_string(id));
      #ifndef OGL_WITHOUT_IMGUI
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
      #endif
      if(window != NULL) { glfwDestroyWindow(window); window = NULL; }
      if(image  != NULL) { free(image); image = NULL; }
      if(windowsCounter == 0) { glfw::close(); }
      
    }
    
    //*****************************************************************************/
    // create() - Crea una nuova finestra
    //*****************************************************************************/
    void create(GLint width, GLint height, bool resizable = false, const char * title = "OpenGL window") {
      
      DEBUG_LOG("glWindow::create() windowID " + std::to_string(windowsCounter));
      
      glfw::init();
            
      // Others Glfw options
      glfwWindowHint(GLFW_RESIZABLE, resizable);
      glfwWindowHint(GLFW_SAMPLES, 4);
      
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
      
      if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        abort();
      }
            
      glfwGetFramebufferSize(window, &width, &height);
      
      glfwSetWindowUserPointer(window, this);

      //glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
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

      background = glm::vec3(0.0f, 0.1f, 0.2f);

      cameras.push_back(glCamera(width, height));

      currentCameraIndex = 0;

      currentCamera = &cameras[currentCameraIndex];

      //inputDisable = false;
        
      // Funziona con OpenGL >= 4.3
      // glDebugMessageCallback((GLDEBUGPROC)glDebugOutput, NULL);
      // glEnable(GL_DEBUG_OUTPUT);
      // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      
      #ifndef OGL_WITHOUT_IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 150");
        ImGui::StyleColorsDark();
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
      glfwWindowHint(GLFW_SAMPLES, 4);
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
      
      if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        abort();
      }
      
      glfwGetFramebufferSize(window, &width, &height);
     
      glfwSetWindowUserPointer(window, this);
      
      //glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
      glfwSetWindowCloseCallback(window, windowCloseCallback);
      glfwSetCursorPosCallback(window, cursorPosCallback);
      glfwSetMouseButtonCallback(window, mouseButtonCallback);
      glfwSetScrollCallback(window, scrollCallback);
      glfwSetKeyCallback(window, keyCallback);
      glfwSetCursorEnterCallback(window, cursorEnterCallback);
      //glfwSetWindowFocusCallback(window, framebufferSizeCallback);
      glfwSetWindowSizeCallback(window, sizeCallback);

      glfwSwapInterval(1);
      
      firstMouse = true;
      
      onFocus = false;
      
      isFullscreen = false;

      id = windowsCounter++;
      
      background = glm::vec3(0.0f, 0.1f, 0.2f);
      
      cameras.push_back(glCamera(width, height));
      
      currentCameraIndex = 0;
      
      currentCamera = &cameras[currentCameraIndex];
      
      //inputDisable = false;
      
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
      
      if(window != NULL) {
        glfwDestroyWindow(window);
        window = NULL;
      }
      
    }
    
    //****************************************************************************//
    // updateCurrentCamera() -
    //****************************************************************************//
    void updateCurrentCamera(float fov, glm::vec3 position = glm::vec3(0.0f), glCamera::MODE mode = glCamera::MODE::FREE, glm::vec3 target = glm::vec3(0.0f)) {
      
      currentCamera->init(currentCamera->getWidth(), currentCamera->getHeight(), fov, position, mode, target);
      
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
    
    //static inline void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    //  ((glWindow*)glfwGetWindowUserPointer(window))->framebufferSizeCallback(width, height);
    //}
    
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
    
    
    //static inline void  windowFocusCallback(GLFWwindow* window, int focused) {
    //  ((glWindow*)glfwGetWindowUserPointer(window))->focus(focused);
    //}
        
    //****************************************************************************//
    // External callback interfaces
    //****************************************************************************//
    inline void virtual scroll(double xoffset, double yoffset) { };
    inline void virtual keyboard(int key, int scancode, int action, int mods) { };
    inline void virtual cursorPos(double xPos, double yPos, double xoffset, double yoffset) { };
    inline void virtual mouseButton(int button, int action, int mods) { };
    inline void virtual cursorEnter(int entered) { if(entered) { onFocus = true; } else { onFocus = false; } }
    //inline void virtual focus(int focused) { if(focused) { onFocus = true; } else { onFocus = false; } }
    
    //****************************************************************************//
    // Callback
    //****************************************************************************//
    inline void windowCloseCallback() {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    inline void sizeCallback(int width, int height) {
      for(std::size_t i=0; i<cameras.size(); ++i)
        cameras[i].setSensorSize(width, height);
    }
    
//    inline void framebufferSizeCallback(int width, int height) {
//      for(std::size_t i=0; i<cameras.size(); ++i)
//        cameras[i].setSensorSize(width, height);
//    }
    
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

        if(currentCamera->getMode() == glCamera::MODE::FREE || (currentCamera->getMode() == glCamera::MODE::BILLBOARD && mods == GLFW_MOD_SHIFT)) {
        
          //Moves/alters the camera positions based on user input
          if(/*keys[GLFW_KEY_W] || */ keys[GLFW_KEY_UP])    currentCamera->processKeyboard(glCamera::MOVEMENT::FORWARD, deltaTime);
          if(/*keys[GLFW_KEY_S] || */ keys[GLFW_KEY_DOWN])  currentCamera->processKeyboard(glCamera::MOVEMENT::BACKWARD, deltaTime);
          if(/*keys[GLFW_KEY_A] || */ keys[GLFW_KEY_LEFT])  currentCamera->processKeyboard(glCamera::MOVEMENT::LEFT, deltaTime);
          if(/*keys[GLFW_KEY_D] || */ keys[GLFW_KEY_RIGHT]) currentCamera->processKeyboard(glCamera::MOVEMENT::RIGHT, deltaTime);
          
        }
        
        if(cameras.size() > 1) if(keys[GLFW_KEY_C]) { changeCamera(); }
        
        keyboard(key, scancode, action, mods);

      }
      
    }
    
    inline void cursorPosCallback(double xPos, double yPos){
      //    if(cursorPos(xpos, ypos)){
      //      //cameraMouseCallback(buttonMouse, xpos - oldXpos, ypos - oldYpos);
      //      oldXpos = xpos; oldYpos = ypos;
      //    }
      
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
    
    inline void mouseButtonCallback(int button, int action, int mods) {
      mouseButton(button, action, mods);
      //    if(action == GLFW_PRESS){
      //      buttonMouse = button;
      //      if(buttonMouse==GLFW_MOUSE_BUTTON_1 && mods==GLFW_MOD_CONTROL) buttonMouse = GLFW_MOUSE_BUTTON_2;
      //      else if(button==GLFW_MOUSE_BUTTON_1 && mods==GLFW_MOD_SHIFT)   buttonMouse = GLFW_MOUSE_BUTTON_3;
      //    } else if(action == GLFW_RELEASE){ buttonMouse = -1; }
    }
    
    
  public:
    
    //****************************************************************************//
    // Window Interface funtions
    //****************************************************************************//
    inline void setPosition(int xPos, int yPos){
      glfwSetWindowPos(window, xPos, yPos);
    }
    
    inline void setTitle(const std::string & title){
      glfwSetWindowTitle(window, title.c_str());
    }
    
    inline bool shouldClose(){
      return glfwWindowShouldClose(window);
    }
    
    inline void setShouldClose(bool mode){
      glfwSetWindowShouldClose(window, mode);
    }
    
    inline void setCursorInputMode(int mode) {
      glfwSetInputMode(window, GLFW_CURSOR, mode);
    }
    
    inline void getCursorPos(double & x, double & y){
      glfwGetCursorPos(window, &x, &y);
    }
    
    inline void setSize(int width, int height){
      glfwSetWindowSize(window, width, height);
      for(std::size_t i=0; i<cameras.size(); ++i)
        cameras[i].setSensorSize(width, height);
    }
    
    inline void setBackground(const glm::vec3 & _background){
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
      
      static std::deque<double> deltas;

      static double lastFrameTimestamp = glfwGetTime();
    
      double currentFrameTimestamp = glfwGetTime();
      
      double delta = currentFrameTimestamp - lastFrameTimestamp;
      
      lastFrameTimestamp = currentFrameTimestamp;
      
      if(deltas.size() < 60) deltas.push_back(delta);
      else {
        deltas.pop_front();
        deltas.push_back(delta);
      }
      
      double sum = 0;
      
      for(int i=0; i<deltas.size(); ++i)
        sum += deltas[i];
      
      return 1.0 / (sum / (double) deltas.size());
      
    }
    
    //****************************************************************************//
    // addCamera()
    //****************************************************************************//
    inline void addCamera(float fov, glm::vec3 position = glm::vec3(0.0f), glCamera::MODE mode = glCamera::FREE, glm::vec3 target = glm::vec3(0.0f)) {
      cameras.push_back(glCamera(currentCamera->getWidth(), currentCamera->getHeight(), fov, position, mode, target));
      currentCamera = &cameras[currentCameraIndex];
    }
    
    //*****************************************************************************/
    // get projection and view matrix
    //*****************************************************************************/
    inline glm::mat4 getProjection() const { return currentCamera->getProjection(); }
    
    inline glm::mat4 getOrthoProjection() const { return currentCamera->getOrthoProjection(); }
      
//    inline glm::mat4 getOrthoProjectionText() const {
//      
//     // float aspect = static_cast<float>(width) / static_cast<float>(height);
//     // return glm::ortho(-aspect, aspect, -1.0f, 1.0f, currentCamera->getzNear(), currentCamera->getzFar());
//      
//      return glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
//      
//    }
    
    inline glm::vec2 getViewport() const { return currentCamera->getViewport(); }

    
    inline glm::mat4 getView() const { return currentCamera->getView(); }

    inline float getPitch() { return currentCamera->getPitch(); }
    inline float getYaw() { return currentCamera->getYaw(); }
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
      
//      int wWidth, wHeight;
//      int fWidth, fHeight;
//
//      glfwGetWindowSize(window, &wWidth, &wHeight);
//
//      glfwGetFramebufferSize(window, &fWidth, &fHeight);
//
//      double xScale = fWidth  / (double)wWidth;
//      double yScale = fHeight / (double)wHeight;
//
//      if(xScale != yScale) {
//        fprintf(stderr, "errore");
//        abort();
//      }
//
//      return (xScale == 2);
      
    }
    
    //*****************************************************************************/
    // makeContextCurrent()
    //*****************************************************************************/
    inline void makeContextCurrent() {
      
      glfwMakeContextCurrent(window);
      gladLoadGLLoader((GLADloadproc) glfwGetProcAddress); //NOTE: non sono sicuro che serva

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
                  
      //printf("%d %d\n", currentCamera->getWidth(), currentCamera->getHeight());
      
      glViewport(0, 0, currentCamera->getWidth(), currentCamera->getHeight());

      glClearColor(background.r, background.g, background.b, 1.0f);
      
      glClearDepth(1.0f); // set to -1.0 for GL_GREATER

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      glEnable(GL_DEPTH_TEST);
      
      glDepthFunc(GL_LEQUAL);
      
      keybord = true;

      #ifndef OGL_WITHOUT_IMGUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
      #endif
      
    }
    
    //*****************************************************************************/
    // renderEnd()
    //*****************************************************************************/
    inline void renderEnd() {
      
      #ifndef OGL_WITHOUT_IMGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      #endif

      glDisable(GL_DEPTH_TEST);
    
      glfwSwapBuffers(window);
      
    }

    //*****************************************************************************/
    // snapshot()
    //*****************************************************************************/
    void snapshot(const std::string & filename) {
      
      glReadBuffer(GL_BACK);
      
//#ifdef OPENCV_ALL_HPP
#ifdef OGL_TEST_SNAPSHOT
      cv::snapshot(currentCamera->getWidth(), currentCamera->getHeight(), filename.c_str());
#else
      tiff::snapshot(currentCamera->getWidth(), currentCamera->getHeight(), filename.c_str());
#endif
      
    }
    
    //*****************************************************************************/
    // pixelsValue()
    //*****************************************************************************/
    glm::vec3 pixelsValue() {
      
      glm::vec3 rbg;
      
      glReadBuffer(GL_BACK);

      double pixelsNum = currentCamera->getWidth() * currentCamera->getHeight();

      // creo un immagine grande quanto la finestra
      image = (GLubyte *) realloc(image, pixelsNum * sizeof(GLubyte) * 3);
      
      glPixelStorei(GL_PACK_ALIGNMENT, 1);
      
      glReadPixels(0, 0, currentCamera->getWidth(), currentCamera->getHeight(), GL_RGB, GL_UNSIGNED_BYTE, image);
      
      glCheckError();

      rbg = glm::vec3(0);
      
      size_t counter = 0;
      
      for(int p=0; p<pixelsNum; ++p) {
        
       // salto i pixels neri
       if(image[(p*3)] == 0 && image[(p*3)+1] == 0 && image[(p*3)+2] == 0) continue;

        rbg.r += image[(p*3)];
        rbg.g += image[(p*3)+1];
        rbg.b += image[(p*3)+2];
        
        ++counter;
        
      }
      
      if(counter != 0) {
      
        rbg.r /= (double)counter;
        rbg.g /= (double)counter;
        rbg.b /= (double)counter;

      }
            
      return rbg;
      
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
      
      currentCamera = &cameras[currentCameraIndex];

    }
    
  };
  
  uint32_t glWindow::windowsCounter = 0;
  
} /* namespace ogl */

#endif /* _H_OGL_GLWINDOW_H_ */
