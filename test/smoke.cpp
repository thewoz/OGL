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
#include <vector>

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

//*****************************************************************************/
// checkClearIsEncoded - lo sfondo deve uscire al valore impostato.
//
// La clear e' una scrittura nel framebuffer come le altre, quindi va ricodificata
// in sRGB dalla GPU, ma solo se GL_FRAMEBUFFER_SRGB e' acceso al momento della
// clear. renderEnd() lo spegne per disegnare ImGui, quindi a inizio frame e'
// spento, e renderBegin() deve riaccenderlo PRIMA di pulire. Qui non c'e' ImGui
// (OGL_WITHOUT_IMGUI), quindi lo stato lasciato da renderEnd() si simula a mano:
// senza questo test il difetto non si vede in headless e solo in una finestra
// vera, dove lo sfondo scuro usciva a 3/255 invece di 26/255.
//*****************************************************************************/
static bool checkClearIsEncoded(ogl::glWindow & window) {

  window.setBackground(glm::vec3(0.1f));

  // lo stato in cui renderEnd() lascia il contesto con ImGui attivo
  glDisable(GL_FRAMEBUFFER_SRGB);

  window.renderBegin();

  unsigned char pixel[3] = {0, 0, 0};
  glReadBuffer(GL_BACK);
  glReadPixels(2, 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);

  window.renderEnd();

  int expected = (int) std::lround(0.1f * 255.0f);

  if(std::abs((int) pixel[0] - expected) > 1) {
    fprintf(stderr, "SMOKE TEST FAILED: background 0.1 came out as %d instead of %d.\n"
                    "  The clear is not being encoded to sRGB: check that renderBegin() enables\n"
                    "  GL_FRAMEBUFFER_SRGB before glClear, not after.\n", (int) pixel[0], expected);
    return false;
  }

  return true;

}

//*****************************************************************************/
// checkPointOrderIndependence - il disegno dei punti non deve dipendere
// dall'ordine in cui sono disegnati.
//
// Gli impostori sferici scrivono la loro profondita' reale e in icaro sono
// disegnati in ordine di indice, non di profondita'. Se il bordo antialiasato
// e' semitrasparente e scrive la profondita', un punto davanti disegnato per
// primo impedisce a quello dietro di comparire sotto il suo bordo, e al suo
// posto si vede lo sfondo: un anello bianco su sfondo bianco. Qui la stessa
// scena si disegna nei due ordini, "davanti prima" (il peggiore) e "dietro
// prima" (il corretto), e le immagini devono coincidere. Con punti piccoli come
// quelli di icaro il difetto vecchio dava decine di pixel diversi, con scarti
// fino a 161/255.
//*****************************************************************************/
static bool checkPointOrderIndependence(ogl::glWindow & window, int width, int height) {

  window.setBackground(glm::vec3(1.0f));

  std::vector<glm::vec3> front = { glm::vec3(0.0f, 0.0f,  1.0f) };
  std::vector<glm::vec3> back  = { glm::vec3(0.0f, 0.0f, -1.0f) };
  std::vector<glm::vec4> blue  = { glm::vec4(0.1f, 0.2f, 0.9f, 1.0f) };
  std::vector<glm::vec4> red   = { glm::vec4(0.9f, 0.1f, 0.1f, 1.0f) };

  // camera a z = 5 che guarda lungo -z: davanti a 4 e dietro a 6 unita', quindi
  // circa 10 e 40 pixel. La camera dello smoke e' FLY con yaw 180: se non la si
  // raddrizza guarda altrove, non si disegna niente e le due immagini
  // coincidono per forza, cioe' il test passerebbe anche sul codice difettoso
  window.getCamera().setPosition(0.0f, 0.0f, 5.0f);
  window.getCamera().setYaw(-90.0f);
  window.getCamera().setPitch(0.0f);
  window.getCamera().setzNearFar(0.1f, 50.0f);

  ogl::glPoints a(front, blue, 40.0f);
  ogl::glPoints b(back,  red,  240.0f);
  a.setShadingMode(ogl::glPoints::DIFFUSE);
  b.setShadingMode(ogl::glPoints::DIFFUSE);

  std::vector<unsigned char> frontFirst((size_t) width * height * 3);
  std::vector<unsigned char> backFirst ((size_t) width * height * 3);

  for(int order = 0; order < 2; ++order) {

    std::vector<unsigned char> & image = order == 0 ? frontFirst : backFirst;

    for(int i=0; i<2; ++i) {

      window.renderBegin();

      if(order == 0) { a.render(window.getCamera()); b.render(window.getCamera()); }
      else           { b.render(window.getCamera()); a.render(window.getCamera()); }

      // si legge prima dello swap, vedi checkSrgbPipeline()
      if(i == 1) {
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image.data());
      }

      window.renderEnd();

    }

  }

  // guardia contro il test vuoto: deve esserci qualcosa di disegnato
  long drawn = 0;

  for(size_t i=0; i<frontFirst.size(); i+=3)
    if(frontFirst[i] < 240 || frontFirst[i+1] < 240 || frontFirst[i+2] < 240) ++drawn;

  if(drawn < 200) {
    fprintf(stderr, "SMOKE TEST FAILED: checkPointOrderIndependence drew only %ld pixels, the test is vacuous.\n", drawn);
    return false;
  }

  long different = 0;

  for(size_t i=0; i<frontFirst.size(); ++i)
    if(std::abs((int) frontFirst[i] - (int) backFirst[i]) > 8) ++different;

  if(different != 0) {
    fprintf(stderr, "SMOKE TEST FAILED: the point rendering depends on the draw order (%ld values differ).\n"
                    "  The impostor edge must not be blended and write depth: check that glPoints\n"
                    "  uses alpha to coverage and that points.fs outputs a non premultiplied alpha.\n", different);
    return false;
  }

  return true;

}

//*****************************************************************************/
// checkInfiniteGrid - la griglia infinita disegna linee ai multipli della cella,
// la cella si cambia al volo e la griglia non scrive profondita'.
//
// Camera sopra il piano y = 0 che guarda in giu': con cella 1 e poi cella 0.5 le
// linee devono raddoppiare. Poi un quadrato sotto il piano, attraversato da una
// linea, disegnato dopo la griglia deve avere tutti i suoi pixel come quando e'
// disegnato da solo: se la griglia scrivesse profondita' la linea lo taglierebbe.
// NB: non basta guardare un pixel qualsiasi, dentro le celle la griglia scarta i
// frammenti e non scrive profondita' neanche se fosse sbagliata
//*****************************************************************************/
static long countGridColumns(const std::vector<unsigned char> & image, int width, int height) {
  // colonne accese lungo la riga centrale, contate come tratti separati
  int y = height / 2;
  long runs = 0;
  bool inside = false;
  for(int x=0; x<width; ++x) {
    bool lit = image[((size_t) y * width + x) * 3 + 1] > 60;
    if(lit && !inside) ++runs;
    inside = lit;
  }
  return runs;
}

static bool checkInfiniteGrid(ogl::glWindow & window, int width, int height) {

  window.setBackground(glm::vec3(0.0f));

  window.getCamera().setPosition(0.3f, 6.0f, 0.3f);
  window.getCamera().setYaw(-90.0f);
  window.getCamera().setPitch(-89.0f);
  window.getCamera().setzNearFar(0.1f, 100.0f);

  ogl::glInfiniteGrid grid;
  grid.init(1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
  grid.setLineWidth(1.0f);

  ogl::glQuad below(glm::vec2(0.5f), glm::vec3(1.0f, 0.0f, 0.0f), ogl::glShader::STYLE::SOLID);
  below.rotate(glm::vec3(-1.57079633f, 0.0f, 0.0f));
  below.translate(glm::vec3(0.3f, -0.5f, 0.3f));
  below.setSpecularStrength(0.0f);   // visto dritto dall'alto il riflesso lo farebbe bianco

  std::vector<unsigned char> image((size_t) width * height * 3);

  auto draw = [&](bool withGrid, bool withQuad) {
    for(int i=0; i<2; ++i) {
      window.renderBegin();
      if(withGrid) grid.render(window.getCamera());
      if(withQuad) below.render(window.getCamera());
      if(i == 1) {
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image.data());
      }
      window.renderEnd();
    }
  };

  draw(true, false);
  long coarse = countGridColumns(image, width, height);

  grid.setCellSize(0.5f);
  if(grid.getCellSize() != 0.5f) { fprintf(stderr, "SMOKE TEST FAILED: glInfiniteGrid::setCellSize ignored.\n"); return false; }
  grid.setCellSize(-1.0f);   // valori non validi vanno ignorati
  if(grid.getCellSize() != 0.5f) { fprintf(stderr, "SMOKE TEST FAILED: glInfiniteGrid accepted a negative cell size.\n"); return false; }

  draw(true, false);
  long fine = countGridColumns(image, width, height);

  if(coarse < 3 || fine < 2 * coarse - 2 || fine > 2 * coarse + 2) {
    fprintf(stderr, "SMOKE TEST FAILED: glInfiniteGrid drew %ld lines with cell 1 and %ld with cell 0.5 (expected about twice).\n", coarse, fine);
    return false;
  }

  // il quadrato (da x = 0.05 a 0.55) e' attraversato dalla linea a x = 0.5
  auto countRed = [&]() {
    long n = 0;
    for(size_t i=0; i<image.size(); i+=3) if(image[i] > 60 && image[i] > 2 * image[i+1] && image[i] > 2 * image[i+2]) ++n;
    return n;
  };
  draw(false, true);
  long alone = countRed();
  draw(true, true);
  long afterGrid = countRed();

  // qualche pixel di scarto e' fisiologico: sul bordo antialiasato del quadrato la
  // linea verde gia' disegnata si mescola col rosso (8 su 2113). Se la griglia
  // scrive profondita' se ne perdono invece circa 180
  if(alone < 200 || afterGrid < alone - alone / 50) {
    fprintf(stderr, "SMOKE TEST FAILED: a surface below glInfiniteGrid has %ld pixels after the grid and %ld alone:\n"
                    "  the grid must not write depth.\n", afterGrid, alone);
    return false;
  }

  // Una superficie SUL piano della griglia (il fondo di una scatola, un target)
  // deve coprirla anche con un zNear minuscolo, come quando la camera sta dentro
  // una scena grande e zNear scende al tetto zFar/10000: con un polygon offset
  // fisso la precisione non bastava e le linee passavano attraverso il fondo a
  // tratti. Il quadrato e' disegnato PRIMA della griglia, come in icaro il
  // landmark, e non deve avere neanche un pixel verde
  {
    // la configurazione del caso reale che lo mostrava: icaro con le traiettorie
    // lunghe 6.5 m in z, un box 1x1x0.5 e la camera a 3.5 dal suo centro. Due
    // pose: dall'alto, dove contava la precisione con zNear minuscolo, e
    // inclinata di 30 gradi, dove la profondita' del piano cambia in fretta da un
    // pixel all'altro e serviva il termine di pendenza di grid.fs
    ogl::glQuad onFloor(glm::vec2(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), ogl::glShader::STYLE::SOLID);
    onFloor.rotate(glm::vec3(-1.57079633f, 0.0f, 0.0f));
    onFloor.translate(glm::vec3(0.0f, -0.5f, 0.0f));
    onFloor.setSpecularStrength(0.0f);

    grid.setHeight(-0.5f);
    grid.setCellSize(0.408f);

    struct Pose { glm::vec3 position; float pitch; int halfH; const char * name; };
    // a 3.75 dal centro del quadrato in entrambe le pose; inclinato, il
    // quadrato si accorcia in verticale e il riquadro controllato con lui
    const Pose poses[2] = { { glm::vec3(0.0f, 3.25f,  0.0f),  -89.9f, 50, "from above" },
                            { glm::vec3(0.0f, 1.375f, 3.248f), -30.0f, 15, "at 30 degrees" } };

    for(const Pose & pose : poses) {

      window.getCamera().setPosition(pose.position);
      window.getCamera().setYaw(-90.0f);
      window.getCamera().setPitch(pose.pitch);
      window.getCamera().setzNearFar(0.00448f, 44.8f);

      for(int i=0; i<2; ++i) {
        window.renderBegin();
        onFloor.render(window.getCamera());
        grid.render(window.getCamera());
        if(i == 1) {
          glReadBuffer(GL_BACK);
          glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image.data());
        }
        window.renderEnd();
      }

      // il quadrato e' largo circa 155 pixel al centro dello schermo: si guarda
      // un riquadro interno, lontano dai bordi dove l'antialiasing mescola i colori
      long red = 0, through = 0;
      for(int y = height/2 - pose.halfH; y < height/2 + pose.halfH; ++y) {
        for(int x = width/2 - 50; x < width/2 + 50; ++x) {
          const unsigned char * p = &image[((size_t) y * width + x) * 3];
          if(p[0] > 60 && p[1] < 40) ++red;
          if(p[1] > 40) ++through;
        }
      }

      if(red < pose.halfH * 2 * 100 * 2 / 3 || through != 0) {
        fprintf(stderr, "SMOKE TEST FAILED: glInfiniteGrid shows through a surface on its own plane, %s (%ld grid pixels over %ld of the surface).\n"
                        "  grid.fs must push the grid depth away from the eye.\n", pose.name, through, red);
        return false;
      }

    }
  }

  return true;

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
    axes.setLabelColor(glm::vec3(0.0f, 0.0f, 0.0f));
    ogl::glGrid grid(10, 10, 0.5f, ogl::glColors::cyan);

    // griglia infinita sul pavimento, copre grid.vs/fs
    ogl::glInfiniteGrid infiniteGrid;
    infiniteGrid.init(0.5f, glm::vec3(0.5f));
    infiniteGrid.setHeight(-0.02f);

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

    // semiasse lungo Z diverso dal raggio: una semiellisse e non una semicirconferenza
    ogl::glSheet ellipseSheet(0.4f, 0.8f, 32, ogl::glShader::STYLE::SOLID, ogl::glColors::green, "", 0.15f);
    ellipseSheet.translate(glm::vec3(1.2f, 0.9f, -1.2f));

    // griglia per intervallo: linee ai multipli del passo dentro un rettangolo
    ogl::glGrid rangeGrid;
    rangeGrid.initRange(-1.2f, 2.3f, -1.0f, 3.1f, 0.5f, ogl::glColors::white);

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
    plot.setLabelSide(0, glm::vec3(0.0f, 0.0f, 1.0f));   // le scritte di X e di Y dal lato +Z
    plot.setLabelSide(1, glm::vec3(0.0f, 0.0f, 1.0f));
    plot.setAxisCrossing(glm::vec3(5.0f, 0.0f, 0.0f));   // Y e Z all'estremo di X
    plot.setAxisTickSteps(1, 0.25f, 0.05f);               // un passo diverso sull'asse Y

    for(int frame = 0; frame < 30; ++frame) {

      window.renderBegin();

      // Shadow pass: i caster nel depth-only shadow map della scena.
      if(scene.areShadowsEnabled()) {
        scene.beginShadowPass();
        floor.renderDepth(scene.getShadowShader());
        cuboid.renderDepth(scene.getShadowShader());
        sphere.renderDepth(scene.getShadowShader());
        sheet.renderDepth(scene.getShadowShader());
        ellipseSheet.renderDepth(scene.getShadowShader());
        model.renderDepth(scene.getShadowShader());
        scene.endShadowPass();
      }

      floor.render(window.getCamera(), &scene);
      axes.render(window.getCamera());
      grid.render(window.getCamera());
      infiniteGrid.render(window.getCamera());
      cuboid.render(window.getCamera(), &scene);
      sphere.render(window.getCamera(), &scene);
      sheet.render(window.getCamera(), &scene);
      ellipseSheet.render(window.getCamera(), &scene);
      rangeGrid.render(window.getCamera());
      wire.render(window.getCamera());
      model.render(window.getCamera(), &scene);
      points.render(window.getCamera(), 0, -1, -1, &scene);
      line.render(window.getCamera());
      text.render(window.getCamera(), "frame " + std::to_string(frame) + "\nsecond line");
      label.render(window.getCamera());
      plot.render(window.getCamera());
      overlay.render(window.getCamera());
      refAxes.render(window.getCamera());

      // la griglia per intervallo si aggiorna senza ricompilare gli shader
      if(frame == 12) rangeGrid.setRange(-2.0f, 2.0f, -2.0f, 2.0f, 0.25f);
      if(frame == 12) infiniteGrid.setCellSize(0.25f);
      if(frame == 20) plot.clearAxisCrossing();

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

    if(!checkSrgbPipeline(window, width, height) || !checkClearIsEncoded(window) || !checkPointOrderIndependence(window, width, height) ||
       !checkInfiniteGrid(window, width, height)) {
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
