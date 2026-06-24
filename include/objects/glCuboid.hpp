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

#ifndef _H_OGL_CUBOID_H_
#define _H_OGL_CUBOID_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glCuboid
  //****************************************************************************/
  // Axis-aligned solid box. Because a cube's 8 corners are shared by faces with
  // different normals, the geometry is expanded to 36 vertices (6 faces × 6
  // verts) so each face can carry its own flat outward normal. Supports SOLID
  // and WIREFRAME styles. In SOLID mode Phong shading uses the scene light.
  //****************************************************************************/
  class glCuboid : public glShape {

  private:

      GLuint vao = 0; // 0 = not yet allocated
      GLuint vbo[4];

      glm::vec3 size;

  public:

    //****************************************************************************/
    // Class glCuboid
    //****************************************************************************/
    glCuboid(const std::string & _name = "") { name = _name; }

    //****************************************************************************/
    // Class glCuboid
    //****************************************************************************/
    glCuboid(const glm::vec3 & _size, int _style = glShader::STYLE::SOLID, const glm::vec3 & _color = glm::vec3(1.0f), const std::string & _name = "") {
      name = _name;
      init(_size, _style, _color);
    }

    //****************************************************************************/
    // ~glCuboid
    //****************************************************************************/
    ~glCuboid() { cleanInGpu(); }

    glCuboid(glCuboid &&) noexcept = default;
    glCuboid & operator = (glCuboid &&) noexcept = default;

    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const glm::vec3 & _size, int _style = glShader::STYLE::SOLID, const glm::vec3 & _color = glm::vec3(1.0f)) {

      DEBUG_LOG("glCuboid::init(" + name + ")");

      shader.setName(name);
      if(_style == glShader::STYLE::WIREFRAME) {
        shader.initWireframe();
      } else {
        shader.initSolid();
      }

      size = _size;
      style = _style;
      color = _color;

      isInited = true;
      
    }

    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, const glScene * scene = nullptr) {

      DEBUG_LOG("glCuboid::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "ERROR [glCuboid]: must be initialized before rendering\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view", camera.getView());
      shader.setUniform("model", modelMatrix);
      shader.setUniform("color", color);
      if(style == glShader::STYLE::SOLID) {
        if(scene) scene->setInShader(shader, camera.getView());
        else { glLight().setInShader(shader, camera.getView()); shader.setUniform("useShadow", false); }
      }

      glBindVertexArray(vao);

      if(style == glShader::STYLE::WIREFRAME) {
        shader.setUniform("lineWidth", lineWidth);
        shader.setUniform("viewport", camera.getViewport());
        glDisable(GL_CULL_FACE);
      } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
      }

      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

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

      glBindVertexArray(vao);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
      glBindVertexArray(0);

      glCheckError();

    }

  private:

    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {

      DEBUG_LOG("glCuboid::setInGpu(" + name + ")");

      glm::vec3 corners[8] = {
          {-0.5f,  0.5f,  0.5f},
          { 0.5f,  0.5f,  0.5f},
          { 0.5f, -0.5f,  0.5f},
          {-0.5f, -0.5f,  0.5f},
          {-0.5f,  0.5f, -0.5f},
          { 0.5f,  0.5f, -0.5f},
          { 0.5f, -0.5f, -0.5f},
          {-0.5f, -0.5f, -0.5f}
      };

      for(auto & v : corners) v *= size;

      // Six faces, each defined by two triangles (same winding as before) plus its
      // own outward normal. The 8 shared corners cannot carry per-face normals, so
      // we expand to 36 vertices and give each face a flat normal.
      GLuint faceCorner[6][6] = {
        { 0, 3, 2, 2, 1, 0 }, // Front
        { 1, 2, 6, 6, 5, 1 }, // Right
        { 5, 6, 7, 7, 4, 5 }, // Back
        { 4, 7, 3, 3, 0, 4 }, // Left
        { 4, 0, 1, 1, 5, 4 }, // Top
        { 3, 7, 6, 6, 2, 3 }  // Bottom
      };

      glm::vec3 faceNormal[6] = {
        { 0,  0,  1}, // Front
        { 1,  0,  0}, // Right
        { 0,  0, -1}, // Back
        {-1,  0,  0}, // Left
        { 0,  1,  0}, // Top
        { 0, -1,  0}  // Bottom
      };

      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> normals;
      std::vector<GLuint>    indices;

      for(int f=0; f<6; ++f) {
        for(int k=0; k<6; ++k) {
          positions.push_back(corners[faceCorner[f][k]]);
          normals.push_back(faceNormal[f]);
          indices.push_back((GLuint)indices.size());
        }
      }

      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(4, vbo);

      // Positions
      glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
      glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      // Normals (one flat normal per face)
      glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
      glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(1);

      // TexCoords (dummy for now, optional)
      std::vector<glm::vec2> texcoords(positions.size(), glm::vec2(0.0f));
      glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
      glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(2);

      // Indices
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);

      isInitedInGpu = true;
      
    }

    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {

      if(isInitedInGpu) {
        glDeleteBuffers(4, vbo);
        glDeleteVertexArrays(1, &vao);
        isInitedInGpu = false;
      }
    }
    
  };

} /* namespace ogl */

#endif /* _H_OGL_CUBOID_H_ */
