/*
 * MIT License
 *
 * Copyright © 2019
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <algorithm>
#include <random>

#define DEBUG_OGL
#include <ogl/ogl.hpp>

#include <imgui/imgui.hpp>

/* ****************************************************************************/
// main
/* ****************************************************************************/
int main(int argc, char * const argv []) {
  
#if(1)
  
  ogl::glWindow window;
  
  window.create(800, 600, "ModelView");
  
  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.makeContextCurrent();
  
  ogl::glLine line({glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0)}, glm::vec3(1.0,0.0,1.0));
  ogl::glAxes axes;
  ogl::glReferenceAxes referenceAxes;
  ogl::glEllipse ellipse(0.1, 0.1, 0.3, 10, 20, ogl::glObject::STYLE::WIREFRAME, glm::vec3(2.0,0.0,1.0));
  ogl::glSphere sphere(1.0, 10, 10, ogl::glObject::STYLE::WIREFRAME, glm::vec3(0.0,0.0,1.0));
  ogl::glGrid grid(10, glm::vec3(0.0,1.0,1.0));
  ogl::glBox box(glm::vec3(1.0,2.0,5.0), glm::vec3(1.0,0.0,0.0)); box.translate(glm::vec3(-0.5));
  ogl::glCuboid cube(glm::vec3(0.1), ogl::glObject::STYLE::SOLID, glm::vec3(1.0,0.0,0.0)); cube.translate(glm::vec3(0.5));
  ogl::glLines lines({glm::vec3(0.0,0.0,0.0), glm::vec3(-1.0,-1.0,-1.0), glm::vec3(-1.0,1.0,1.0)}, glm::vec4(1.0));
  ogl::glPrint text2D("OGL", 10, 10, glm::vec3(1.0));
  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx"); model.setLight(glm::vec3(1.0), glm::vec3(-1.0));
  
  std::random_device rd;
  std::mt19937 gen = std::mt19937(rd());
  std::normal_distribution<float> gaussRandom = std::normal_distribution<float>(-0.2, 0.2);

  std::vector<glm::vec3> coords(1000);

  for(size_t i=0; i<1000; ++i) {
    coords[i].x = gaussRandom(gen);
    coords[i].y = gaussRandom(gen);
    coords[i].z = gaussRandom(gen);
  }

  ogl::glPoints points(coords, glm::vec4(1.0), 10, "points");

  while(!window.shouldClose()) {
    
    window.renderBegin();
      line.render(window.getCurrentCamera()); glCheckError();
      axes.render(window.getCurrentCamera()); glCheckError();
      ellipse.render(window.getCurrentCamera()); glCheckError();
      sphere.render(window.getCurrentCamera()); glCheckError();
      grid.render(window.getCurrentCamera()); glCheckError();
      box.render(window.getCurrentCamera()); glCheckError();
      cube.render(window.getCurrentCamera()); glCheckError();
      referenceAxes.render(window.getCurrentCamera()); glCheckError();
      text2D.render(window.getCurrentCamera()); glCheckError();
      points.render(window.getCurrentCamera()); glCheckError();
      lines.render(window.getCurrentCamera()); glCheckError();
      model.render(window.getCurrentCamera()); glCheckError();
    window.renderEnd();
        
  }
  
#endif
  
  
#if(0)
  
  ogl::glWindow window;
  
  window.create(800, 600, "ModelView");
  
  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.makeContextCurrent();
  
  ogl::glAxes axes(1.0);
  ogl::glSmallAxes axesSmall(1.0);

  ogl::glPrint3D text3D("OGL", 1, 1, 1, glm::vec3(255.0));
  ogl::glPrint   text2D("testo");
  
  ogl::glLine line({glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0)}, glm::vec3(1.0,0.0,1.0));
  ogl::glCuboid cube(glm::vec3(0.1), ogl::glObject::STYLE::SOLID, glm::vec3(1.0,0.0,0.0)); cube.translate(glm::vec3(0.5));
  
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    axes.render(window.getProjection(), window.getView());
    axesSmall.render(window.getOrthoProjection(), window.getCurrentCamera()->getLookAt(glm::vec3(0)));
   
    line.render(window.getProjection(), window.getView());
    cube.render(window.getProjection(), window.getView());
    //cube.render(window.getOrthoProjection(), window.getView());
    
    text2D.print(window.getOrthoProjectionText(), "leo", 0, 0, glm::vec3(255.0));

    text3D.print(window.getProjection(), window.getCurrentCamera());

    window.renderEnd();
    
  }
  
#endif
  
  
#if(0)
  
  ogl::glWindow window;
  
  window.create(800, 600, "ModelView");
  
  //window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.makeContextCurrent();
  
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  
  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();
  
  const char* glsl_version = "#version 150";

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window.window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
  
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color;
    
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;
      
      ImGui::Begin("Params");                                 // Create a window called "Hello, world!" and append into it.
      
      ImGui::Text("List of Icaro parameters:");               // Display some text (you can use a format strings too)
      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);
      
      ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
      
      if(ImGui::Button("Button"))                             // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      
      ImGui::End();
    }
    
    // Rendering
    ImGui::Render();
   
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    window.renderEnd();
    
  }
  
#endif
  
  
#if(0)
  
  ogl::glWindow window;
  
  window.create(800, 600, "ModelView");
  
  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.makeContextCurrent();
  
  ogl::glPrint text("testo");
    
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    text.print(window.getOrthoProjection(), "leo", 0, 0, glm::vec3(255.0));

    window.renderEnd();
    
  }
  
#endif
  
#if(0)
  
  ogl::glWindow window;
    
  window.create(800, 600, "ModelView");

  window.setCursorInputMode(GLFW_CURSOR_DISABLED);

  window.makeContextCurrent();
    
  while(!window.shouldClose()) {
      
    window.renderBegin();
      
    float ratio = window.width / (float) window.height;
    glViewport(0, 0, window.width, window.height);
      
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(-0.6f, -0.4f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.6f, -0.4f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.6f, 0.f);
    glEnd();
            
    window.renderEnd();
      
  }

#endif
  
#if(0)
  
  ogl::glWindow window;

  window.create(800, 600, "ModelView");

  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.makeContextCurrent();

  std::random_device rd;
  std::mt19937 gen = std::mt19937(rd());
  std::normal_distribution<float> gaussRandom = std::normal_distribution<float>(-0.2, 0.2);
  
  std::vector<glm::vec3> coords(1000);
  
  for(size_t i=0; i<1000; ++i) {
    coords[i].x = gaussRandom(gen);
    coords[i].y = gaussRandom(gen);
    coords[i].z = gaussRandom(gen);
  }
  
  ogl::glPoints points(coords, glm::vec4(1.0), 10, "punti");
  
  ogl::glAxes axes(1.0, "assi");
  ogl::glSphere sphere(1.0, 10, 10, ogl::glObject::STYLE::WIREFRAME, glm::vec3(0.0,0.0,1.0), "sfera");
  ogl::glEllipse ellipse(0.1, 0.1, 0.3, 10, 20, ogl::glObject::STYLE::WIREFRAME, glm::vec3(2.0,0.0,1.0), "ellipse");
  ogl::glLine line({glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0)}, glm::vec3(1.0,0.0,1.0), "linea");
  ogl::glCuboid cube(glm::vec3(0.1), ogl::glObject::STYLE::SOLID, glm::vec3(1.0,0.0,0.0), "cubo"); cube.translate(glm::vec3(0.5));
  ogl::glBox box(glm::vec3(1.0,2.0,5.0), glm::vec3(1.0,0.0,0.0), "box"); box.translate(glm::vec3(-0.5));
  ogl::glPrint text("testo");
  ogl::glGrid grid(10, glm::vec3(0.0,1.0,1.0), "griglia");
  ogl::glPrint3D text3D("testo3D");

  while(!window.shouldClose()) {
    
    window.renderBegin();
  
      axes.render(window.getProjection(), window.getView());
//      sphere.render(window.getProjection(), window.getView());
//      grid.render(window.getProjection(), window.getView());
      line.render(window.getProjection(), window.getView());
      cube.render(window.getProjection(), window.getView());
//      box.render(window.getProjection(), window.getView());
//      ellipse.render(window.getProjection(), window.getView());
//      points.render(window.getProjection(), window.getView());
      text.print(window.getOrthoProjection(), "OGL", 0, 0, glm::vec3(255.0));
      text3D.print(window.getProjection(), window.getView(), "OGL 3D", 0, 1, 0, glm::vec3(255.0), 1);

    window.renderEnd();
      
  }

#endif
  
#if(0)
  
  ogl::glAxes axes;

  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx");

  ogl::glWindow window;

  window.create(800, 600, "ModelView");
  
  window.makeContextCurrent();
      
  window.addCamera(45.0, 0.1, 10.0, glm::vec3(1.1, 1.3, 1.4), ogl::glCamera::MODE::TARGET, glm::vec3(0.01, 0.01, 0.01));

  window.changeCamera();
      
  model.setLight(glm::vec3(1.0), glm::vec3(-1.0));

  uint32_t frame = 0;

  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    axes.render(window.getProjection(), window.getView());
    
    model.rotate(glm::vec3(glm::sin(glm::radians((float)frame)),
                                    glm::sin(glm::radians((float)frame)),
                                    glm::cos(glm::radians((float)frame))));

    model.render(window.getProjection(), window.getView());
    
    window.renderEnd();
    
    ++frame;
      
  }
      
#endif
  
#if(0)
  
  ogl::glAxes axes;
  
  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx");
  
  ogl::glWindow window;
  
  window.create(800, 600, "ModelView");
  
  window.makeContextCurrent();
  
  window.addCamera(45.0, 0.1, 10.0, glm::vec3(1.1, 1.3, 1.4), ogl::glCamera::MODE::TARGET, glm::vec3(0.01, 0.01, 0.01));
  
  window.changeCamera();
  
  model.setLight(glm::vec3(1.0), glm::vec3(-1.0));
  
  uint32_t frame = 0;
  
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    axes.render(window.getProjection(), window.getView());
    
    model.rotate(glm::vec3(glm::sin(glm::radians((float)frame)),
                           glm::sin(glm::radians((float)frame)),
                           glm::cos(glm::radians((float)frame))));
    
    model.render(window.getProjection(), window.getView());
    
    window.renderEnd();
    
    ++frame;
    
  }
  
#endif
  
#if(0)
  
  ogl::glAxes axes;
  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx");

  ogl::glWindow window;
    
  window.create(800, 600, "ModelView");
  
  window.makeContextCurrent();
  
  window.addCamera(45.0, 0.1, 10.0, glm::vec3(1.1, 1.3, 1.4), ogl::glCamera::MODE::TARGET, glm::vec3(0.01, 0.01, 0.01));
  
  window.changeCamera();
  
  model.setLight(glm::vec3(1.0), glm::vec3(-1.0));
  
  uint32_t frame = 0;
  
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    axes.render(window.getProjection(), window.getView());
    
    model.rotate(glm::vec3(glm::sin(glm::radians((float)frame)),
                           glm::sin(glm::radians((float)frame)),
                           glm::cos(glm::radians((float)frame))));
    
    model.render(window.getProjection(), window.getView());
    
    window.renderEnd();
    
    ++frame;
    
  }
  
  window.destroy();

  ogl::glWindow window1;
  
  window1.create(800, 600, "ModelView");
  
  window1.makeContextCurrent();
  
  window1.addCamera(45.0, 0.1, 10.0, glm::vec3(1.1, 1.3, 1.4), ogl::glCamera::MODE::TARGET, glm::vec3(0.01, 0.01, 0.01));
  
  window1.changeCamera();
  
  model.setLight(glm::vec3(1.0), glm::vec3(-1.0));
  
  frame = 0;
  
  while(!window1.shouldClose()) {
    
    window1.renderBegin();
    
    axes.render(window1.getProjection(), window1.getView());
    
    model.rotate(glm::vec3(glm::sin(glm::radians((float)frame)),
                           glm::sin(glm::radians((float)frame)),
                           glm::cos(glm::radians((float)frame))));
    
    model.render(window1.getProjection(), window1.getView());
    
    window1.renderEnd();
    
    ++frame;
    
  }
  
  window1.destroy();
  
#endif

  return 0;
  
}
