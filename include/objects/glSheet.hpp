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

#ifndef _H_OGL_SHEET_H_
#define _H_OGL_SHEET_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <vector>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glSheet
  //****************************************************************************/
  // Open half cylinder of axis Y: a curved sheet, the lateral surface of half a
  // cylinder with no caps and no lid. Supports SOLID and WIREFRAME styles.
  //
  // The cross section is a half ELLIPSE: 'radius' is its semi-axis along X (half
  // the chord between the two open edges) and 'depth' the one along Z (how far
  // the ridge is from the chord). With depth equal to the radius, which is the
  // default, it is the half circle of a true half cylinder.
  //
  // Geometry, with 'radius' R, 'depth' D and 'height' H:
  //
  //   - the axis is Y, the top edge lies at y = 0 and the bottom one at y = -H,
  //     so the sheet develops downwards like the other landmark shapes
  //   - the arc runs from the open edge (-R, y, 0) to the opposite one
  //     (+R, y, 0) passing through the ridge (0, y, -D): the chord between the
  //     two open edges lies on X, its midpoint is the origin, and the concave
  //     side faces +Z, so the sheet occupies z from -D to 0
  //   - the surface is centred on the origin in XZ; move it with translate()
  //
  // Unlike a strip of independent glQuads, the arc is a single indexed mesh in
  // one VAO drawn with one call, and the normals are the analytic ones of the
  // ellipse, proportional to (-cos a / R, 0, -sin a / D) at the angle a (the
  // gradient of x^2/R^2 + z^2/D^2), so they are per vertex and
  // the shading comes out smooth instead of faceted. 'segments' therefore costs
  // vertices only, not draw calls or shader programs.
  //
  // The normals point towards the axis, i.e. towards the concave side, which is
  // the one being observed. Back face culling is off by default so the sheet is
  // rendered from both sides (see setCullFaceEnabled). solid.fs flips the normal
  // when it points away from the eye, so the convex side is lit properly too.
  //****************************************************************************/
  class glSheet : public glShape {

  private:

    GLuint vao = 0;
    GLuint vbo[3] = {0, 0, 0};   // positions, normals, indices

    float radius = 1.0f;   // semi-axis along X
    float depth  = 1.0f;   // semi-axis along Z
    float height = 1.0f;

    int segments = 32;

    GLsizei indicesNum = 0;

    bool cullFaceEnabled = false;

  public:

    //****************************************************************************/
    // glSheet()
    //****************************************************************************/
    glSheet(const std::string & _name = "") { name = _name; }

    //****************************************************************************/
    // glSheet()
    //****************************************************************************/
    glSheet(float _radius, float _height, int _segments = 32, int _style = glShader::STYLE::SOLID, const glm::vec3 & _color = glm::vec3(1.0f), const std::string & _name = "", float _depth = -1.0f) {

      name = _name;

      init(_radius, _height, _segments, _style, _color, _depth);

    }

    //****************************************************************************/
    // ~glSheet()
    //****************************************************************************/
    ~glSheet() { cleanInGpu(); }

    glSheet(glSheet &&) noexcept = default;
    glSheet & operator = (glSheet &&) noexcept = default;

    //****************************************************************************/
    // init()
    //****************************************************************************/
    // _depth is the semi-axis along Z; a value that is not positive (the
    // default) means "same as the radius", i.e. a half circle. It comes last so
    // that every existing call keeps its meaning
    void init(float _radius, float _height, int _segments = 32, int _style = glShader::STYLE::SOLID, const glm::vec3 & _color = glm::vec3(1.0f), float _depth = -1.0f) {

      DEBUG_LOG("glSheet::init(" + name + ")");

      if(!(_radius > 0.0f) || !(_height > 0.0f)) {
        fprintf(stderr, "ERROR [glSheet]: radius and height must be positive\n");
        abort();
      }

      shader.setName(name);

      if(_style == glShader::STYLE::WIREFRAME) {
        shader.initWireframe();
      } else {
        shader.initSolid();
      }

      radius = _radius;
      depth  = (_depth > 0.0f) ? _depth : _radius;
      height = _height;

      // below three segments the arc is not a curve any more
      segments = (_segments < 3) ? 3 : _segments;

      style = _style;

      color = _color;

      isInited = true;
      isToUpdateInGpu = true; // re-init after a render must re-upload

    }

    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, const glScene * scene = nullptr) {

      DEBUG_LOG("glSheet::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "ERROR [glSheet]: must be initialized before rendering\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       camera.getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("color",      color);

      if(style == glShader::STYLE::SOLID) {
        if(scene) scene->setInShader(shader, camera.getView());
        else { glLight().setInShader(shader, camera.getView()); shader.setUniform("useShadow", false); }
        shader.setUniform("shininess",        shininess);
        shader.setUniform("specularStrength", specularStrength);
        shader.setUniform("unlit",            unlit);
      }

      glBindVertexArray(vao);

      if(style == glShader::STYLE::WIREFRAME) {
        shader.setUniform("lineWidth", lineWidth);
        shader.setUniform("viewport",  camera.getViewport());
        glDisable(GL_CULL_FACE);
      } else if(cullFaceEnabled) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
      } else {
        glDisable(GL_CULL_FACE);
      }

      glDrawElements(GL_TRIANGLES, indicesNum, GL_UNSIGNED_INT, nullptr);

      glBindVertexArray(0);

      glCheckError();

    }

    //****************************************************************************/
    // renderDepth() - cast a shadow into the scene shadow map (SOLID only)
    //****************************************************************************/
    void renderDepth(const glShader & depthShader) override {

      if(!isInited || style != glShader::STYLE::SOLID) return;

      if(isToInitInGpu()) initInGpu();

      depthShader.setUniform("model", modelMatrix);

      // Open surface with no thickness: there is no far surface to fall back on,
      // so the front face culling that beginPass() sets up would drop this
      // object from the shadow map entirely whenever the light happens to see
      // its front, and it would cast no shadow at all. Culling is therefore off
      // here, so that the surface is recorded whichever side the light sees.
      //
      // What the culling was buying, though, was protection from the surface
      // shadowing itself, and with no thickness the bias in solid.fs is not
      // enough on its own: at grazing light the acne comes back as striping.
      // The polygon offset pushes the recorded depth away from the light, which
      // is the per-object equivalent of what the culling did for the closed
      // volumes. It MUST be turned off again right after the draw, or the
      // closed casters drawn later inherit it and their shadows detach.
      glDisable(GL_CULL_FACE);

      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2.0f, 4.0f);

      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, indicesNum, GL_UNSIGNED_INT, nullptr);

      // vedi sopra: l'offset non deve sopravvivere a questo oggetto
      glDisable(GL_POLYGON_OFFSET_FILL);
      glBindVertexArray(0);

      glCheckError();

    }

    //****************************************************************************/
    // setCullFaceEnabled() - Toggle back-face culling
    //****************************************************************************/
    void setCullFaceEnabled(bool _enabled) { cullFaceEnabled = _enabled; }

    //****************************************************************************/
    // interrogation
    //****************************************************************************/
    float getRadius()   const { return radius;   }
    float getDepth()    const { return depth;    }
    float getHeight()   const { return height;   }
    int   getSegments() const { return segments; }

    //****************************************************************************/
    // getEdge() - the open edge at the given height, +R side for _right
    //****************************************************************************/
    glm::vec3 getEdge(bool _right, float _y = 0.0f) const {

      return glm::vec3(_right ? radius : -radius, _y, 0.0f);

    }

  private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glSheet::setInGpu(" + name + ")");

      // Same-context re-upload: drop the old buffers first (no-op after a
      // context change, where isInitedInGpu is already false).
      cleanInGpu();

      {

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;

        positions.reserve((segments + 1) * 2);
        normals.reserve((segments + 1) * 2);

        // Two rows of segments+1 vertices, interleaved per column: the vertex
        // 2k is the top one (y = 0) and 2k+1 the bottom one (y = -height).
        for(int k=0; k<=segments; ++k) {

          // The half turn is the one from pi to 2 pi, i.e. the arc rotated by
          // 180 degrees about Y with respect to the one from 0 to pi: (x, z)
          // becomes (-x, -z), so the ridge lies on -Z and the concave side
          // faces +Z. It is a proper rotation, so the winding below and the
          // analytic normal keep their meaning unchanged.
          double angle = M_PI + M_PI * (double) k / (double) segments;

          float x = radius * (float) std::cos(angle);
          float z = depth  * (float) std::sin(angle);

          // Analytic normal. The outward one is the gradient of the implicit
          // equation of the ellipse, (x/R^2, 0, z/D^2), which is proportional to
          // (cos a / R, 0, sin a / D); the inward one is its opposite. With
          // R = D it reduces to (-cos a, 0, -sin a) of the half circle. A
          // surface normal is NOT the position vector for an ellipse, same
          // point as in glEllipse. No cross product, hence no discontinuity
          // between the segments.
          glm::vec3 normal = glm::normalize(glm::vec3(-(float) std::cos(angle) / radius, 0.0f, -(float) std::sin(angle) / depth));

          positions.push_back(glm::vec3(x,  0.0f,    z));
          normals.push_back(normal);

          positions.push_back(glm::vec3(x, -height,  z));
          normals.push_back(normal);

        }

        // One quad per segment, wound so that the geometric normal of the
        // triangles agrees with the inward normal assigned above.
        std::vector<GLuint> indices;

        indices.reserve(segments * 6);

        for(int k=0; k<segments; ++k) {

          GLuint t0 = (GLuint)(k * 2);
          GLuint b0 = t0 + 1;
          GLuint t1 = t0 + 2;
          GLuint b1 = t0 + 3;

          indices.push_back(t0); indices.push_back(b0); indices.push_back(b1);
          indices.push_back(t0); indices.push_back(b1); indices.push_back(t1);

        }

        indicesNum = (GLsizei) indices.size();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(3, vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glCheckError();

      }

      isInitedInGpu = true;

    }

    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() override {

      if(isInitedInGpu) {

        glDeleteBuffers(3, vbo);
        glDeleteVertexArrays(1, &vao);

        isInitedInGpu = false;

      }

    }

  }; /* class glSheet */

} /* namespace ogl */

#endif /* _H_OGL_SHEET_H_ */
