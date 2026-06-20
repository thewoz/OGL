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

/**
 * ImGui integration example for OGL.
 *
 * How ImGui works with OGL:
 *   - Do NOT define OGL_WITHOUT_IMGUI (it is not defined here).
 *   - glWindow::renderBegin() calls ImGui::NewFrame() automatically.
 *   - glWindow::renderEnd()   calls ImGui::Render() automatically.
 *   - Between those two calls, use any standard ImGui widgets.
 *
 * Run with:
 *   make example_imgui
 */

#include <cstdio>
#include <cstdlib>

// OGL_WITHOUT_IMGUI is intentionally NOT defined here so that
// glWindow initialises the ImGui context and drives the frame lifecycle.
#include <ogl/ogl.hpp>

//*****************************************************************************/
// main
//*****************************************************************************/
int main(int argc, char * const argv[]) {

  ogl::glWindow window;
  window.create(1024, 768);

  window.getCurrentCamera()->setPosition(3, 1.5f, 0);
  window.getCurrentCamera()->setYaw(180);
  window.getCurrentCamera()->setPitch(-20);

  // --- Scene objects ---

  ogl::glAxes axes;
  ogl::glGrid grid(10, 10, 0.5f, ogl::glColors::cyan);

  // Solid cuboid with a directional light (position is light color, direction
  // is the light direction vector).
  ogl::glCuboid cuboid(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::white);
  cuboid.setLight(glm::vec3(1.0f), glm::vec3(-1.0f));

  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx");
  model.setLight(glm::vec3(1.0f), glm::vec3(-1.0f));

  ogl::glPrint2D fpsText(10, 10, ogl::glColors::white, 0.5f);

  ogl::glReferenceAxes referenceAxes;
  referenceAxes.setLineWidth(2);

  // --- ImGui-controlled parameters ---

  glm::vec3 cuboidPos(0.0f, 0.0f, 0.0f);
  glm::vec3 lightDir(-1.0f, -1.0f, -1.0f);
  float     lightIntensity = 1.0f;
  bool      showGrid       = true;
  bool      showAxes       = true;
  bool      showModel      = true;

  while(!window.shouldClose()) {

    window.renderBegin();

    // --- ImGui panel ---
    // glWindow::renderBegin() already called ImGui::NewFrame().

    ImGui::SetNextWindowPos(ImVec2(10, 40), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

    ImGui::Begin("Scene Controls");

    ImGui::Text("FPS: %d", window.getFPS());
    ImGui::Separator();

    ImGui::Text("Cuboid");
    ImGui::DragFloat3("Position##cuboid", &cuboidPos.x, 0.01f, -5.0f, 5.0f);

    ImGui::Separator();

    ImGui::Text("Light");
    ImGui::DragFloat3("Direction##light",   &lightDir.x,    0.01f, -1.0f, 1.0f);
    ImGui::SliderFloat("Intensity##light",  &lightIntensity, 0.0f,  2.0f);

    ImGui::Separator();

    ImGui::Text("Visibility");
    ImGui::Checkbox("Grid",  &showGrid);
    ImGui::Checkbox("Axes",  &showAxes);
    ImGui::Checkbox("TRex",  &showModel);

    ImGui::End();

    // --- Apply ImGui parameters to scene objects ---

    cuboid.translate(cuboidPos);
    cuboid.setLight(glm::vec3(lightIntensity), lightDir);

    if(showModel)
      model.setLight(glm::vec3(lightIntensity), lightDir);

    // --- Render scene ---

    if(showAxes)  axes.render(window.getCurrentCamera());
    if(showGrid)  grid.render(window.getCurrentCamera());

    cuboid.render(window.getCurrentCamera());

    if(showModel) model.render(window.getCurrentCamera());

    fpsText.render(window.getCurrentCamera(), "FPS: " + std::to_string((int)window.getFPS()));

    referenceAxes.render(window.getCurrentCamera());

    // glWindow::renderEnd() calls ImGui::Render() and swaps buffers.
    window.renderEnd();

  }

  return 0;

}
