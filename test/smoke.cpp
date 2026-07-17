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
 * Smoke test offscreen (non interattivo): esercita finestra offscreen, tutti i
 * principali tipi di oggetto, la shadow pass, gli update dinamici dei buffer,
 * lo snapshot e il teardown. Esce con 0 se tutto funziona.
 *
 * Build ed esecuzione:  make test          (build normale)
 *                       make test_asan     (AddressSanitizer + UBSan)
 */

#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>

#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

int main() {

  const char * snapshotPath = "/tmp/ogl_smoke_snapshot.tiff";

  const int width  = 640;
  const int height = 480;

  {
    ogl::glWindow window;
    window.createOffscreen(width, height);

    window.getCamera().setPosition(3, 1.5f, 0);
    window.getCamera().setYaw(180);
    window.getCamera().setPitch(-20);
    window.getCamera().setzNearFar(0.1f, 100.0f);

    ogl::glScene scene;
    scene.setDirectionalLight(glm::vec3(-1.0f, -1.0f, -1.0f));
    scene.enableShadows(true);
    scene.setBounds(glm::vec3(0.0f, 0.5f, 0.0f), 3.0f);

    ogl::glAxes axes;
    ogl::glGrid grid(10, 10, 0.5f, ogl::glColors::cyan);

    ogl::glQuad floor(glm::vec2(10.0f), glm::vec3(0.55f), ogl::glShader::STYLE::SOLID);
    floor.rotate(glm::vec3(-1.57079633f, 0.0f, 0.0f));
    floor.translate(glm::vec3(0.0f, -0.01f, 0.0f));

    ogl::glCuboid cuboid(glm::vec3(0.5f), ogl::glShader::STYLE::SOLID, ogl::glColors::white);
    cuboid.translate(glm::vec3(1.2f, 0.25f, 0.0f));

    ogl::glSphere sphere(0.3f, 16, 16, ogl::glShader::STYLE::SOLID, ogl::glColors::red);
    sphere.translate(glm::vec3(-1.0f, 0.3f, 0.5f));

    ogl::glModel model(OGL_RESOURCE_DIR "/data/model/Trex/Trex.fbx");

    std::vector<glm::vec3> pts = { {0,1,0}, {0.5f,1,0}, {1,1,0.5f} };
    ogl::glPoints points(pts, glm::vec4(1,1,0,1), 20.0f);

    std::vector<glm::vec3> lineV = { {0,0,0}, {1,1,1}, {2,0,1} };
    ogl::glLine line(lineV, glm::vec3(0,1,0));

    ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5f);
    ogl::glPrint3D label("hello", glm::vec3(0, 1.5f, 0), ogl::glColors::yellow, 0.5f);
    ogl::glReferenceAxes refAxes;

    ogl::glPlot plot;
    plot.init(ogl::glPlot::AxisRange(0, 5), ogl::glPlot::AxisRange(0, 2), ogl::glPlot::AxisRange(0, 3));

    for(int frame = 0; frame < 30; ++frame) {

      window.renderBegin();

      // Shadow pass: i caster nel depth-only shadow map della scena.
      if(scene.areShadowsEnabled()) {
        scene.beginShadowPass();
        floor.renderDepth(scene.getShadowShader());
        cuboid.renderDepth(scene.getShadowShader());
        sphere.renderDepth(scene.getShadowShader());
        model.renderDepth(scene.getShadowShader());
        scene.endShadowPass();
      }

      floor.render(window.getCamera(), &scene);
      axes.render(window.getCamera());
      grid.render(window.getCamera());
      cuboid.render(window.getCamera(), &scene);
      sphere.render(window.getCamera(), &scene);
      model.render(window.getCamera(), &scene);
      points.render(window.getCamera(), 0, -1, -1, &scene);
      line.render(window.getCamera());
      text.render(window.getCamera(), "frame " + std::to_string(frame) + "\nsecond line");
      label.render(window.getCamera());
      plot.render(window.getCamera());
      refAxes.render(window.getCamera());

      // Update dinamico dei buffer (percorso isToUpdateInGpu).
      if(frame == 10) {
        lineV.push_back(glm::vec3(3, 1, 0));
        line.update(lineV);
        pts.push_back(glm::vec3(1.5f, 1.2f, 0));
        points.init(pts, glm::vec4(0, 1, 1, 1), 15.0f);
      }

      window.renderEnd();

    }

    window.snapshot(snapshotPath);

    window.destroy();
  }

  // Lo snapshot deve esistere e contenere almeno i pixel RGB non compressi.
  struct stat st;
  if(stat(snapshotPath, &st) != 0 || st.st_size < (off_t)width * height * 3) {
    fprintf(stderr, "SMOKE TEST FAILED: snapshot '%s' missing or truncated\n", snapshotPath);
    return 1;
  }
  remove(snapshotPath);

  fprintf(stdout, "SMOKE TEST OK\n");
  return 0;

}
