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

#include <cstdio>
#include <cstdlib>

//#define DEBUG_OGL
#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

//*****************************************************************************/
// main
//*****************************************************************************/
int main(int argc, char * const argv []) {

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
