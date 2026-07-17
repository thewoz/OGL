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
 * OGL example.
 *
 * Build without ImGui:  make example        (defines OGL_WITHOUT_IMGUI)
 * Build with    ImGui:  make example_imgui  (OGL_WITHOUT_IMGUI not defined)
 *
 * When built with ImGui, glWindow::renderBegin() / renderEnd() drive the
 * frame lifecycle automatically; just place ImGui widgets between them.
 */

#include <cstdio>
#include <cstdlib>

#include <ogl/ogl.hpp>

//*****************************************************************************/
// main
//*****************************************************************************/
int main(int /*argc*/, char * const /*argv*/[]) {

  ogl::glWindow window;
  window.create(1024, 768);
  // Start in free-look mode: the cursor is captured and drives the camera.
  // Press P to reveal the ImGui panel (see the loop below).
  window.setCursorInputMode(GLFW_CURSOR_DISABLED);

  window.getCamera().setPosition(3, 1.5f, 0);
  window.getCamera().setYaw(180);
  window.getCamera().setPitch(-20);
  // Push the far plane out: the default (10) clips objects as soon as you
  // move a few units away from the scene.
  window.getCamera().setzNearFar(0.1f, 100.0f);
  //window.getCamera().setMode(ogl::glCamera::ORBIT);

  // --- Scene (light + shadows shared by every object) ---

  ogl::glScene scene;
  scene.setDirectionalLight(glm::vec3(-1.0f, -1.0f, -1.0f));
  scene.enableShadows(true);
  scene.setBounds(glm::vec3(0.0f, 0.5f, 0.0f), 3.0f);

  // --- Scene objects shared by both demo scenes ---

  ogl::glAxes axes;
  ogl::glGrid grid(10, 10, 0.5f, ogl::glColors::cyan);

  // Solid floor that receives the shadows (a quad laid flat on the XZ plane).
  ogl::glQuad floor(glm::vec2(10.0f), glm::vec3(0.55f), ogl::glShader::STYLE::SOLID);
  floor.rotate(glm::vec3(-1.57079633f, 0.0f, 0.0f)); // -90° about X: normal points up
  floor.translate(glm::vec3(0.0f, -0.01f, 0.0f));

  ogl::glPrint2D fpsText(10, 10, ogl::glColors::white, 0.5f);
  ogl::glPrint2D sceneText(10, 45, ogl::glColors::yellow, 0.5f);

  ogl::glReferenceAxes referenceAxes;
  referenceAxes.setLineWidth(2);

  // --- Demo scene A: the T-Rex model ---

  ogl::glModel model(OGL_RESOURCE_DIR "/data/model/Trex/Trex.fbx");

  // --- Demo scene B: primitive shapes (cuboid, sphere) + 3D text ---

  ogl::glCuboid cuboid(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::white);
  cuboid.translate(glm::vec3(1.2f, 0.25f, 0.0f));

  ogl::glSphere sphere(0.35f, 24, 24, ogl::glShader::STYLE::SOLID, ogl::glColors::orange);
  sphere.translate(glm::vec3(-1.2f, 0.35f, 0.0f));

  ogl::glPrint3D label3D("OGL", glm::vec3(0.0f, 1.3f, 0.0f), ogl::glColors::white, 0.6f, true);

  // Press TAB to switch between the two demo scenes (works with or without ImGui).
  bool showTrexScene = true;
  bool tabWasDown    = false;

#ifndef OGL_WITHOUT_IMGUI
  // --- ImGui-controlled parameters ---

  glm::vec3 lightDir(-1.0f, -1.0f, -1.0f);
  float     lightIntensity = 1.0f;
  bool      showGrid       = true;
  bool      showAxes       = true;
  bool      showShadows    = true;

  // The parameters panel is hidden at start; press P to toggle it. While it is
  // open the cursor is freed and the camera is frozen, so the mouse can drive
  // the widgets. Closing it re-captures the cursor for free-look navigation.
  bool showPanel = false;
  bool pWasDown  = false;

  ogl::glPrint2D hintText(10, 80, ogl::glColors::white, 0.5f);
#endif

  while(!window.shouldClose()) {

    window.renderBegin();

    // --- Toggle between the T-Rex model and the shapes demo with TAB ---
    // (edge-triggered, like the P-panel toggle below; works in both builds)

    bool tabIsDown = (glfwGetKey(window.window, GLFW_KEY_TAB) == GLFW_PRESS);
    if(tabIsDown && !tabWasDown) showTrexScene = !showTrexScene;
    tabWasDown = tabIsDown;

#ifndef OGL_WITHOUT_IMGUI
    // --- Toggle the panel with P (edge-triggered) ---

    bool pIsDown = (glfwGetKey(window.window, GLFW_KEY_P) == GLFW_PRESS);
    if(pIsDown && !pWasDown) {
      showPanel = !showPanel;
      if(showPanel) {
        window.setCursorInputMode(GLFW_CURSOR_NORMAL);
        window.disableMouseOnCamera();
      } else {
        window.setCursorInputMode(GLFW_CURSOR_DISABLED);
        window.enableMouseOnCamera();
        window.resetMouse();  // avoid a camera jump when re-capturing the cursor
      }
    }
    pWasDown = pIsDown;

    // --- ImGui panel ---

    if(showPanel) {

      ImGui::SetNextWindowPos(ImVec2(10, 40), ImGuiCond_Once);
      ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Once);

      ImGui::Begin("Scene Controls");

      ImGui::Text("FPS: %d", window.getFPS());
      ImGui::Separator();

      ImGui::Text("Scene: %s", showTrexScene ? "T-Rex" : "Shapes");
      ImGui::TextDisabled("(press TAB to switch)");
      if(ImGui::Button(showTrexScene ? "Show shapes" : "Show T-Rex")) showTrexScene = !showTrexScene;

      ImGui::Separator();

      ImGui::Text("Light");
      ImGui::DragFloat3("Direction##light",  &lightDir.x,     0.01f, -1.0f, 1.0f);
      ImGui::SliderFloat("Intensity##light", &lightIntensity, 0.0f,  2.0f);

      ImGui::Separator();

      ImGui::Text("Visibility");
      ImGui::Checkbox("Grid",    &showGrid);
      ImGui::Checkbox("Axes",    &showAxes);
      ImGui::Checkbox("Shadows", &showShadows);

      ImGui::End();

    }

    // --- Apply ImGui parameters to the scene ---

    scene.setDirectionalLight(lightDir);
    scene.setAmbient (glm::vec3(0.2f));
    scene.setDiffuse (glm::vec3(0.5f * lightIntensity));
    scene.setSpecular(glm::vec3(1.0f * lightIntensity));
    scene.enableShadows(showShadows);
#endif

    // --- Shadow pass: render the casters into the scene shadow map ---

    if(scene.areShadowsEnabled()) {
      scene.beginShadowPass();
      floor.renderDepth(scene.getShadowShader());
      if(showTrexScene) {
        model.renderDepth(scene.getShadowShader());
      } else {
        cuboid.renderDepth(scene.getShadowShader());
        sphere.renderDepth(scene.getShadowShader());
      }
      scene.endShadowPass();
    }

    // --- Render scene ---

    floor.render(window.getCamera(), &scene);

#ifndef OGL_WITHOUT_IMGUI
    if(showAxes)  axes.render(window.getCamera());
    if(showGrid)  grid.render(window.getCamera());
#else
    axes.render(window.getCamera());
    grid.render(window.getCamera());
#endif

    if(showTrexScene) {
      model.render(window.getCamera(), &scene);
    } else {
      cuboid.render(window.getCamera(), &scene);
      sphere.render(window.getCamera(), &scene);
      label3D.render(window.getCamera());
    }

    fpsText.render(window.getCamera(), "FPS: " + std::to_string(window.getFPS()));
    sceneText.render(window.getCamera(), std::string("Scene: ") + (showTrexScene ? "T-Rex" : "Shapes") + " (TAB to switch)");
    referenceAxes.render(window.getCamera());

#ifndef OGL_WITHOUT_IMGUI
    hintText.render(window.getCamera(), showPanel ? "Press P to hide parameters"
                                                  : "Press P to show parameters");
#endif

    window.renderEnd();

  }

  return 0;

}
