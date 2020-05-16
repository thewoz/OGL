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

#include <opencv2/opencv.hpp>

#include <ogl/ogl.hpp>
//#include "hull.hpp"


#include "glPrint.hpp"

/* ****************************************************************************/
// main
/* ****************************************************************************/
int main(int argc, char * const argv []) {
      
#if(1)
  
  ogl::glWindow window;
  
  window.create(800, 600, "ModelView");
  
  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.makeContextCurrent();
  
  ogl::glPrint text("testo");
  
  text.init();
  
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
    text.print(window.getOrthoProjection(), "leo", 50, 05, glm::vec3(1.0));

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
  
  std::vector<cv::Point3f> coords(1000);
  
  for(size_t i=0; i<1000; ++i) {
    coords[i].x = gaussRandom(gen);
    coords[i].y = gaussRandom(gen);
    coords[i].z = gaussRandom(gen);
  }
  
  ogl::glPoints points(coords, glm::vec4(1.0), "punti");
  
  ogl::glAxes axes(1.0, "assi");
  ogl::glSphere sphere(1.0, 10, 10, ogl::glObject::STYLE::WIREFRAME, glm::vec3(0.0,0.0,1.0), "sfera");
  ogl::glEllipse ellipse(0.1, 0.1, 0.3, 10, 20, ogl::glObject::STYLE::WIREFRAME, glm::vec3(2.0,0.0,1.0), "ellipse");
  ogl::glLine line({glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0)}, glm::vec3(1.0,0.0,1.0), "linea");
  ogl::glCuboid cube(glm::vec3(0.1), ogl::glObject::STYLE::SOLID, glm::vec3(1.0,0.0,0.0), "cubo"); cube.translate(glm::vec3(0.5));
  ogl::glBox box(glm::vec3(1.0,2.0,5.0), glm::vec3(1.0,0.0,0.0), "box"); box.translate(glm::vec3(-0.5));

  ogl::glGrid grid(10, glm::vec3(0.0,1.0,1.0), "griglia");

  while(!window.shouldClose()) {
    
    window.renderBegin();
  
      axes.render(window.getProjection(), window.getView());

      //sphere.render(window.getProjection(), window.getView());

      //grid.render(window.getProjection(), window.getView());

      //line.render(window.getProjection(), window.getView());

      //cube.render(window.getProjection(), window.getView());
    
      //box.render(window.getProjection(), window.getView());

      //ellipse.render(window.getProjection(), window.getView());

      //points.render(window.getProjection(), window.getView());
    
      //ogl::glPrint("prova", 1.0, 1.0, 1.0f);
      
    window.renderEnd();
      
  }

#endif
  
#if(0)
  
  ogl::glAxes axes;

  ogl::glModel model("/usr/local/include/ogl/model/Trex/Trex.fbx");

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
  
  ogl::glModel model("/usr/local/include/ogl/model/Trex/Trex.fbx");
  
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
  ogl::glModel model("/usr/local/include/ogl/model/Trex/Trex.fbx");

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
