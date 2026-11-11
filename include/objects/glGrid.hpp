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

#ifndef _H_OGL_GRID_H_
#define _H_OGL_GRID_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <vector>


//*****************************************************************************/
// namespace ogl
//*****************************************************************************/
namespace ogl {

  //*****************************************************************************/
  // Class glGrid
  //*****************************************************************************/
  class glGrid : public glObject {
    
  private:
        
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    
    int rows;
    int cols;

    float cellSize;

    std::vector<GLuint> indices;

    // Modalita' "intervallo", alternativa a rows/cols/cellSize: le linee stanno
    // ai multipli di 'rangeStep' dentro il rettangolo [rangeXMin, rangeXMax] x
    // [rangeZMin, rangeZMax], piu' il bordo del rettangolo. Cosi' la griglia
    // copre esattamente un'area data e le sue linee cadono sugli stessi valori
    // dei tick di un glPlot, invece di partire dal centro con celle intere.
    // Vedi initRange()
    bool  useRange = false;
    float rangeXMin = 0.0f, rangeXMax = 1.0f;
    float rangeZMin = 0.0f, rangeZMax = 1.0f;
    float rangeStep = 1.0f;

    // tetto alle linee per direzione, come i tick di glPlot: un passo minuscolo
    // non deve generare milioni di segmenti
    static constexpr int maxLinesPerAxis = 10000;
    
  public:
    
    //****************************************************************************/
    // glGrid()
    //****************************************************************************/
    glGrid(const std::string & _name = "") { name = _name; }
    glGrid(int _rows, int _cols, float _cellSize, const glm::vec3 & _color = glm::vec3(1.0), const std::string & _name = "") {
      name = _name;
      init(_rows, _cols, _cellSize, _color);
    }
    
    //****************************************************************************/
    // ~glGrid()
    //****************************************************************************/
    ~glGrid() { cleanInGpu(); }

    glGrid(glGrid &&) noexcept = default;
    glGrid & operator = (glGrid &&) noexcept = default;
 
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(int _rows, int _cols, float _cellSize, const glm::vec3 & _color = glm::vec3(0.0)) {
      
      DEBUG_LOG("glGrid::init(" + name + ")");

      shader.setName(name);
      
      shader.initLine();
      
      rows = _rows;
      
      cols = _cols;
      
      cellSize = _cellSize;

      // torna alla modalita' righe/colonne anche dopo una initRange()/setRange()
      useRange = false;

      color = _color;

      isInited = true;
      isToUpdateInGpu = true; // re-init after a render must re-upload

    }
   
    //****************************************************************************/
    // initRange() - griglia sul piano XZ (a y = 0, si sposta con translate())
    // con le linee ai multipli di 'step' dentro [xMin, xMax] x [zMin, zMax] e il
    // bordo del rettangolo. La init() compila lo shader, quindi va fatta una
    // volta sola: per cambiare l'area o il passo dopo si usa setRange()
    //****************************************************************************/
    void initRange(float _xMin, float _xMax, float _zMin, float _zMax, float _step, const glm::vec3 & _color = glm::vec3(0.0)) {

      DEBUG_LOG("glGrid::initRange(" + name + ")");

      shader.setName(name);

      shader.initLine();

      color = _color;

      isInited = true;

      setRange(_xMin, _xMax, _zMin, _zMax, _step);

    }

    //****************************************************************************/
    // setRange() - cambia area e passo di una griglia in modalita' intervallo,
    // senza toccare gli shader. Un passo non positivo lascia solo il bordo
    //****************************************************************************/
    void setRange(float _xMin, float _xMax, float _zMin, float _zMax, float _step) {

      useRange = true;

      rangeXMin = _xMin; rangeXMax = _xMax;
      rangeZMin = _zMin; rangeZMax = _zMax;
      rangeStep = _step;

      isToUpdateInGpu = true;

    }

    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {
      
      DEBUG_LOG("glGrid::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "ERROR [glGrid]: must be initialized before rendering\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection",   camera.getProjection());
      shader.setUniform("view",         camera.getView());
      shader.setUniform("model",        modelMatrix);
      shader.setUniform("lineWidth",    lineWidth);
      shader.setUniform("viewport",     camera.getViewport());
      shader.setUniform("uniformColor", glm::vec4(color, 1.0f));

      glBindVertexArray(vao);
          
      glDisable(GL_CULL_FACE);
      glDisableVertexAttribArray(1);
      glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

      glDrawElements(GL_LINES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
      
      glBindVertexArray(0);
      
      glCheckError();
            
    }
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glGrid::setInGpu(" + name + ")");

      // Same-context re-upload: drop the old buffers first (no-op after a
      // context change, where isInitedInGpu is already false).
      cleanInGpu();

      indices.clear();

      std::vector<glm::vec3> vertices;

      auto addLine = [&](const glm::vec3 & a, const glm::vec3 & b) {
        vertices.push_back(a);
        vertices.push_back(b);
        indices.push_back((GLuint)vertices.size() - 2);
        indices.push_back((GLuint)vertices.size() - 1);
      };

      if(useRange) {

        // il bordo del rettangolo
        addLine(glm::vec3(rangeXMin, 0.0f, rangeZMin), glm::vec3(rangeXMax, 0.0f, rangeZMin));
        addLine(glm::vec3(rangeXMin, 0.0f, rangeZMax), glm::vec3(rangeXMax, 0.0f, rangeZMax));
        addLine(glm::vec3(rangeXMin, 0.0f, rangeZMin), glm::vec3(rangeXMin, 0.0f, rangeZMax));
        addLine(glm::vec3(rangeXMax, 0.0f, rangeZMin), glm::vec3(rangeXMax, 0.0f, rangeZMax));

        // le linee interne, ai multipli del passo. Il confronto e' negato per
        // scartare anche i NaN. Si conta con un intero e non accumulando un
        // float, che a molte linee deriverebbe
        if(rangeStep > 0.0f) {

          const float epsilon = 1e-5f * rangeStep;

          // oltre questo indice un float non distingue piu' una linea dalla
          // successiva (e il cast a long andrebbe in overflow): niente linee
          const float maxIndex = 1e6f;

          // parallele a z, a x costante
          if((rangeXMax - rangeXMin) / rangeStep <= (float) maxLinesPerAxis && std::fabs(rangeXMin / rangeStep) < maxIndex) {
            for(long k = (long) std::ceil(rangeXMin / rangeStep); ; ++k) {
              float x = (float) k * rangeStep;
              if(x > rangeXMax + epsilon) break;
              if(x <= rangeXMin + epsilon || x >= rangeXMax - epsilon) continue;   // sono gia' il bordo
              addLine(glm::vec3(x, 0.0f, rangeZMin), glm::vec3(x, 0.0f, rangeZMax));
            }
          }

          // parallele a x, a z costante
          if((rangeZMax - rangeZMin) / rangeStep <= (float) maxLinesPerAxis && std::fabs(rangeZMin / rangeStep) < maxIndex) {
            for(long k = (long) std::ceil(rangeZMin / rangeStep); ; ++k) {
              float z = (float) k * rangeStep;
              if(z > rangeZMax + epsilon) break;
              if(z <= rangeZMin + epsilon || z >= rangeZMax - epsilon) continue;
              addLine(glm::vec3(rangeXMin, 0.0f, z), glm::vec3(rangeXMax, 0.0f, z));
            }
          }

        }

      } else {

        float halfWidth  = (cols * cellSize) * 0.5f;
        float halfHeight = (rows * cellSize) * 0.5f;

        // Linee verticali
        for(int i = 0; i <= cols; ++i) {
          float x = -halfWidth + i * cellSize;
          addLine(glm::vec3(x, 0.0f, -halfHeight), glm::vec3(x, 0.0f, halfHeight));
        }

        // Linee orizzontali
        for(int j = 0; j <= rows; ++j) {
          float z = -halfHeight + j * cellSize;
          addLine(glm::vec3(-halfWidth, 0.0f, z), glm::vec3(halfWidth, 0.0f, z));
        }

      }

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      glGenBuffers(1, &ibo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      glCheckError();
            
    }
    
  private:
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() override {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;

      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GRID_H_ */
