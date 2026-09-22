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
 * la pipeline colore sRGB, lo snapshot e il teardown. Esce con 0 se tutto
 * funziona.
 *
 * Build ed esecuzione:  make test          (build normale)
 *                       make test_asan     (AddressSanitizer + UBSan)
 */

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <sys/stat.h>

#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

//*****************************************************************************/
// checkSrgbPipeline - il controllo di portabilita' della pipeline colore.
//
// OGL delega alla GPU la codifica sRGB delle scritture nel framebuffer
// (GL_FRAMEBUFFER_SRGB in renderBegin, piu' l'hint GLFW_SRGB_CAPABLE alla
// creazione della finestra), e in cambio gli shader emettono colore lineare.
// Se su una piattaforma quella codifica NON avviene - default framebuffer non
// sRGB-capable, o enum non supportato - non si vede un errore: tutta la scena
// esce semplicemente troppo scura. Questo test rende quel caso un fallimento.
//
// glQuad2D passa da plain2D.fs, che linearizza il colore proprio perche' la
// GPU lo ri-codifica: il round-trip deve quindi restituire esattamente il
// colore dato. NB: non si puo' usare la query COLOR_ENCODING al suo posto,
// perche' su almeno un driver (Intel/macOS) riporta GL_LINEAR per il default
// framebuffer anche quando la conversione avviene davvero.
//*****************************************************************************/
static bool checkSrgbPipeline(ogl::glWindow & window, int width, int height) {

  const float levels[] = { 0.05f, 0.25f, 0.5f, 0.75f, 1.0f };

  bool ok = true;

  for(float level : levels) {

    ogl::glQuad2D probe;
    probe.init(glm::vec2(0.0f), glm::vec2((float) width, (float) height),
               glm::vec4(level, level, level, 1.0f));

    unsigned char pixel[3] = {0, 0, 0};

    // Il pixel va letto PRIMA di renderEnd(): dopo lo swap dei buffer il
    // contenuto del back buffer e' indefinito da specifica, quindi leggerlo
    // dopo funzionerebbe per caso su alcuni driver e non su altri.
    window.renderBegin();
    probe.render(window.getCamera());

    glReadBuffer(GL_BACK);
    glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

    window.renderEnd();

    int expected = (int) std::lround(level * 255.0f);

    // una tolleranza di 1 LSB copre l'arrotondamento della conversione
    if(std::abs((int) pixel[0] - expected) > 1) {
      fprintf(stderr, "SMOKE TEST FAILED: sRGB round trip broken, color %.2f "
                      "read back as %d instead of %d.\n"
                      "  The framebuffer is not encoding to sRGB: check that the\n"
                      "  GLFW_SRGB_CAPABLE hint is honoured and that\n"
                      "  glEnable(GL_FRAMEBUFFER_SRGB) is supported here.\n",
              level, (int) pixel[0], expected);
      ok = false;
    }

  }

  return ok;

}

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

    ogl::glSheet sheet(0.4f, 0.8f, 32, ogl::glShader::STYLE::SOLID, ogl::glColors::blue);
    sheet.translate(glm::vec3(0.0f, 0.9f, -1.2f));
    sheet.setSpecularStrength(0.3f);
    sheet.setShininess(64.0f);

    // WIREFRAME copre wireframe.vs/gs/fs, che nessun altro oggetto qui usa
    ogl::glCuboid wire(glm::vec3(0.3f), ogl::glShader::STYLE::WIREFRAME, ogl::glColors::magenta);
    wire.translate(glm::vec3(-1.2f, 0.4f, -0.8f));

    // glQuad2D copre plain2D.vs/fs
    ogl::glQuad2D overlay;
    overlay.init(glm::vec2(10.0f, 60.0f), glm::vec2(120.0f, 40.0f), glm::vec4(0.2f, 0.4f, 0.8f, 0.6f));

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
        sheet.renderDepth(scene.getShadowShader());
        model.renderDepth(scene.getShadowShader());
        scene.endShadowPass();
      }

      floor.render(window.getCamera(), &scene);
      axes.render(window.getCamera());
      grid.render(window.getCamera());
      cuboid.render(window.getCamera(), &scene);
      sphere.render(window.getCamera(), &scene);
      sheet.render(window.getCamera(), &scene);
      wire.render(window.getCamera());
      model.render(window.getCamera(), &scene);
      points.render(window.getCamera(), 0, -1, -1, &scene);
      line.render(window.getCamera());
      text.render(window.getCamera(), "frame " + std::to_string(frame) + "\nsecond line");
      label.render(window.getCamera());
      plot.render(window.getCamera());
      overlay.render(window.getCamera());
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

    if(!checkSrgbPipeline(window, width, height)) {
      window.destroy();
      return 1;
    }

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
