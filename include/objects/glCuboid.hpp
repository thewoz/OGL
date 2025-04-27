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

#ifndef _H_OGL_CUBOID_H_
#define _H_OGL_CUBOID_H_

#include <cstdio>
#include <cstdlib>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glCuboid
  //****************************************************************************/
  class glCuboid : public glObject {

  private:

      GLuint vao = -1;
      GLuint vbo[4];
    
      glm::vec3 color;
      glm::vec3 size;

  public:

    //****************************************************************************/
    // Class glCuboid
    //****************************************************************************/
    glCuboid(const std::string & _name = "") { name = _name; }

    //****************************************************************************/
    // Class glCuboid
    //****************************************************************************/
    glCuboid(const glm::vec3 & _size, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0f), const std::string & _name = "") {
      name = _name;
      init(_size, _style, _color);
    }

    //****************************************************************************/
    // ~glCuboid
    //****************************************************************************/
    ~glCuboid() { cleanInGpu(); }

    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const glm::vec3 & _size, int _style = glObject::STYLE::WIREFRAME, const glm::vec3 & _color = glm::vec3(1.0f)) {

      DEBUG_LOG("glCuboid::init(" + name + ")");

      shader.setName(name);
      shader.initPlain();

      size = _scale;
      style = _style;
      color = _color;

      isInited = true;
      
    }

    //****************************************************************************/
    // init()
    //****************************************************************************/
    void render(const glCamera * camera) {

      DEBUG_LOG("glCuboid::render(" + name + ")");

      if(!isInited) {
        fprintf(stderr, "glCuboid must be inited before render\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view", camera->getView());
      shader.setUniform("model", modelMatrix);
      shader.setUniform("color", color);

      glBindVertexArray(vao);

      if(style == glObject::STYLE::WIREFRAME) {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      } else {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }

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

      std::vector<glm::vec3> positions = {
          {-0.5f,  0.5f,  0.5f},
          { 0.5f,  0.5f,  0.5f},
          { 0.5f, -0.5f,  0.5f},
          {-0.5f, -0.5f,  0.5f},
          {-0.5f,  0.5f, -0.5f},
          { 0.5f,  0.5f, -0.5f},
          { 0.5f, -0.5f, -0.5f},
          {-0.5f, -0.5f, -0.5f}
      };

      for(auto & v : positions) v *= size;

      std::vector<glm::vec3> normals = {
          { 0,  0,  1}, // Front
          { 1,  0,  0}, // Right
          { 0,  0, -1}, // Back
          {-1,  0,  0}, // Left
          { 0,  1,  0}, // Top
          { 0, -1,  0}  // Bottom
      };

      std::vector<GLuint> indices = {
        // Front face
           0, 3, 2, 2, 1, 0,
           // Right face
           1, 2, 6, 6, 5, 1,
           // Back face
           5, 6, 7, 7, 4, 5,
           // Left face
           4, 7, 3, 3, 0, 4,
           // Top face
           4, 0, 1, 1, 5, 4,
           // Bottom face
           3, 7, 6, 6, 2, 3
      };
    
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(4, vbo);

      // Positions
      glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
      glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
      glEnableVertexAttribArray(0);

      // Normals (fake: repeat per face for now if you want per-vertex)
      std::vector<glm::vec3> expandedNormals;
      for(int i=0; i<6; ++i) {
        expandedNormals.push_back(normals[i]);
        expandedNormals.push_back(normals[i]);
        expandedNormals.push_back(normals[i]);
        expandedNormals.push_back(normals[i]);
      }

      glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
      glBufferData(GL_ARRAY_BUFFER, expandedNormals.size() * sizeof(glm::vec3), expandedNormals.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, nullptr);
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
