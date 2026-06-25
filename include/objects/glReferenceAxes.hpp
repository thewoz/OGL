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

#ifndef _H_OGL_REFERENCE_AXES_H_
#define _H_OGL_REFERENCE_AXES_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glReferenceAxes
  //****************************************************************************/
  // Renders a small orientation gizmo (XYZ axes) in the bottom-right corner of
  // the viewport. The gizmo always tracks the camera orientation: it uses only
  // the rotation part of the view matrix so the axes stay centred as the camera
  // moves. Works correctly with all three camera modes (FLY, FIXED, ORBIT).
  //
  // Axis colors: X = red, Y = green, Z = blue.
  // Axis labels (X, Y, Z) are rendered as 2D text overlaid on the main
  // framebuffer at the projected tip positions of the gizmo axes.
  //
  // Depends on glPrint2D being declared before this class (guaranteed by the
  // include order in ogl.hpp).
  //****************************************************************************/
  class glReferenceAxes : public glObject {

  private:

    GLuint vao = 0;
    GLuint vbo = 0;

    // Axis colors: X=red, Y=green, Z=blue. Shared constant, not per-object state
    // (a const member would otherwise make the class non-move-assignable).
    static inline const glm::vec3 colors[3] = {
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 0.0f, 1.0f)
    };

    // Six vertices: origin + tip for each of the three axes.
    std::vector<glm::vec3> vertices;

    // One 2D label per axis, rendered in the main framebuffer coordinate system.
    glPrint2D xlabel, ylabel, zlabel;

  public:

    //****************************************************************************/
    // glReferenceAxes()
    //****************************************************************************/
    glReferenceAxes(float _scale = 1.0f, const std::string & _name = "") {
      name = _name;
      init(_scale);
    }

    //****************************************************************************/
    // ~glReferenceAxes()
    //****************************************************************************/
    ~glReferenceAxes() { cleanInGpu(); }

    glReferenceAxes(glReferenceAxes &&) noexcept = default;
    glReferenceAxes & operator = (glReferenceAxes &&) noexcept = default;

    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(float _scale = 1.0f) {

      DEBUG_LOG("glReferenceAxes::init(" + name + ")");

      shader.setName(name);
      shader.initLine();

      scale(glm::vec3(_scale));

      // Each axis: origin → unit tip.
      vertices.resize(6);
      vertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);
      vertices[1] = glm::vec3(1.0f, 0.0f, 0.0f); // X tip
      vertices[2] = glm::vec3(0.0f, 0.0f, 0.0f);
      vertices[3] = glm::vec3(0.0f, 1.0f, 0.0f); // Y tip
      vertices[4] = glm::vec3(0.0f, 0.0f, 0.0f);
      vertices[5] = glm::vec3(0.0f, 0.0f, 1.0f); // Z tip

      isInited = true;

    }

    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {

      DEBUG_LOG("glReferenceAxes::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "ERROR [glReferenceAxes]: must be initialized before rendering\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      // --- Save the current viewport ---

      GLint viewport[4];
      glGetIntegerv(GL_VIEWPORT, viewport);

      // Place the gizmo in the bottom-right corner with a small margin.
      constexpr GLint overlaySize = 160;
      GLint overlayX = viewport[0] + viewport[2] - overlaySize - 10;
      GLint overlayY = viewport[1] + 10;

      glViewport(overlayX, overlayY, overlaySize, overlaySize);

      glDisable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);

      // --- Draw the three axes ---

      shader.use();

      glm::mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -2.0f, 2.0f);

      // Extract only the rotation from the view matrix by keeping the upper-left
      // 3×3 block and setting the 4th column to identity. This is correct for
      // all camera modes: FLY/FIXED use lookAt (rotation is the 3×3 block),
      // ORBIT composes T1*R*T2 (the 3×3 block is still just R after the
      // radians fix in glCamera::eulerRotation()).
      glm::mat4 viewRotOnly = glm::mat4(glm::mat3(camera.getView()));

      shader.setUniform("projection", projection);
      shader.setUniform("view",       viewRotOnly);
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("lineWidth",  lineWidth);
      shader.setUniform("viewport",   glm::vec2(overlaySize, overlaySize));

      glBindVertexArray(vao);
      glDisableVertexAttribArray(1);
      glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);

      for(int i = 0; i < 3; ++i) {
        shader.setUniform("uniformColor", glm::vec4(colors[i], 1.0f));
        glDrawArrays(GL_LINES, i * 2, 2);
      }

      glBindVertexArray(0);

      // --- Restore main viewport before rendering labels ---
      glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

      // --- Render X/Y/Z labels at the projected tip positions ---

      // Tips are nudged slightly beyond 1.0 to give the label a small margin
      // from the end of the axis line.
      const glm::vec3 tips[3] = {
        glm::vec3(1.18f, 0.0f,  0.0f),
        glm::vec3(0.0f,  1.18f, 0.0f),
        glm::vec3(0.0f,  0.0f,  1.18f)
      };

      glPrint2D * labels[3] = { &xlabel, &ylabel, &zlabel };
      const char * labelText[3] = { "X", "Y", "Z" };

      for(int i = 0; i < 3; ++i) {

        // Project the tip through the same matrices used to draw the gizmo.
        glm::vec4 clip = projection * viewRotOnly * glm::vec4(tips[i], 1.0f);

        // NDC (ortho → w is always 1, but divide anyway for correctness).
        float ndcX = clip.x / clip.w;
        float ndcY = clip.y / clip.w;

        // Convert from NDC to main-framebuffer pixel coordinates.
        // getOrthoProjection() maps (0,0)-(width,height) → NDC, so pixel
        // (0,0) is the bottom-left of the main viewport.
        float px = overlayX + (ndcX + 1.0f) * 0.5f * overlaySize;
        float py = overlayY + (ndcY + 1.0f) * 0.5f * overlaySize;

        labels[i]->render(camera, labelText[i], px, py, colors[i], 0.35f);

      }

      glEnable(GL_DEPTH_TEST);

      glCheckError();

    }

  private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glReferenceAxes::setInGpu(" + name + ")");

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);

      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      glCheckError();

    }

    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() override {

      if(isInitedInGpu) {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
      }

    }

  };

} /* namespace ogl */

#endif /* _H_OGL_REFERENCE_AXES_H_ */
