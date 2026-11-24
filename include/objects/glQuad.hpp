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

#ifndef _H_OGL_GLQUAD_H_
#define _H_OGL_GLQUAD_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

#include <vector>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // class glQuad
  //****************************************************************************/
  class glQuad : public glShape {

    private:
      glm::vec2 size;
      bool cullFaceEnabled = false;

      std::vector<glm::vec3> vertices;

    public:
    
      //****************************************************************************/
      // glQuad()
      //****************************************************************************/
      glQuad(const glm::vec2& _size = glm::vec2(1.0f), const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID, const std::string & _name = "") {
        name = _name;
        init(_size, _color, _style);
      }
    
      //****************************************************************************/
      // glQuad()
      //****************************************************************************/
      glQuad(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID, const std::string & _name = "") {
        name = _name;
        init(_vertices, _color, _style);
      }
    
      //****************************************************************************/
      // ~glQuad()
      //****************************************************************************/
      ~glQuad() { releaseInGpu(); }

      // The base glObject move neuters the moved-from object's destructor, so a
      // defaulted move safely transfers ownership of the GPU handles.
      glQuad(glQuad &&) noexcept = default;
      glQuad & operator = (glQuad &&) noexcept = default;
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(const glm::vec2 & _size, const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID) {
          
        DEBUG_LOG("glQuad::init(" + name + ")");

        shader.setName(name);
        if(_style == glShader::STYLE::WIREFRAME) {
          shader.initWireframe();
        } else {
          shader.initSolid();
        }

        size = _size;
        style = _style;
        color = _color;

        // back to the size-based quad even after an init(vertices)
        vertices.clear();

        isInited = true;
        isToUpdateInGpu = true; // re-init after a render must re-upload

      }
    
      //****************************************************************************/
      // init()
      //****************************************************************************/
      void init(const std::vector<glm::vec3> & _vertices, const glm::vec3 & _color = glm::vec3(1.0f), int _style = glShader::STYLE::SOLID) {
        
        DEBUG_LOG("glQuad::init(" + name + ")");

        if(_vertices.size() != 6) {
          fprintf(stderr, "ERROR [glQuad]: vertices must be six\n");
          abort();
        }
        
        shader.setName(name);
        if(_style == glShader::STYLE::WIREFRAME) {
          shader.initWireframe();
        } else {
          shader.initSolid();
        }

        size = glm::vec2(1.0f);
        style = _style;
        color = _color;
      
        vertices = _vertices;

        isInited = true;
        isToUpdateInGpu = true; // re-init after a render must re-upload

      }
    
      //****************************************************************************/
      // render()
      //****************************************************************************/
      void render(const glCamera & camera, const glScene * scene = nullptr) {

        DEBUG_LOG("glQuad::render(" + name + ")");

        if(!isInited) {
          fprintf(stderr, "ERROR [glQuad]: must be initialized before rendering\n");
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
          shader.setUniform("shininess",        shininess);
          shader.setUniform("specularStrength", specularStrength);
          shader.setUniform("unlit",            unlit);
        }

        buffers.bind();

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

        glDrawArrays(GL_TRIANGLES, 0, 6);

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

        buffers.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // vedi sopra: l'offset non deve sopravvivere a questo oggetto
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindVertexArray(0);

        glCheckError();

      }

      //****************************************************************************/
      // setCullFaceEnabled() - Toggle back-face culling
      //****************************************************************************/
      void setCullFaceEnabled(bool _enabled) { cullFaceEnabled = _enabled; }

    private:
    
      //****************************************************************************/
      // setInGpu()
      //****************************************************************************/
      void setInGpu() override {

        DEBUG_LOG("glQuad::setInGpu(" + name + ")");

        GLfloat _vertices[18]; // 6 vertices * 3 coords
        
        if(vertices.size() != 0) {
          
          for(size_t i=0; i<6; ++i) {
            _vertices[i * 3 + 0] = vertices[i].x;
            _vertices[i * 3 + 1] = vertices[i].y;
            _vertices[i * 3 + 2] = vertices[i].z;
          }
          
        } else {
          
          GLfloat defaultVertices[] = { -0.5f * size.x,  0.5f * size.y, 0.0f,
            -0.5f * size.x, -0.5f * size.y, 0.0f,
            0.5f * size.x, -0.5f * size.y, 0.0f,
            -0.5f * size.x,  0.5f * size.y, 0.0f,
            0.5f * size.x, -0.5f * size.y, 0.0f,
            0.5f * size.x,  0.5f * size.y, 0.0f };
          
          memcpy(_vertices, defaultVertices, sizeof(defaultVertices));
          
        }
        
        prepareBuffers(2);

        buffers.attribute(0, 0, 3, _vertices, sizeof(_vertices));

        glm::vec3 normal(0.0f, 0.0f, 1.0f);
        if(vertices.size() == 6) {
          glm::vec3 edgeA = vertices[1] - vertices[0];
          glm::vec3 edgeB = vertices[2] - vertices[0];
          // Check the cross product BEFORE normalizing: normalizing a zero
          // vector (degenerate/collinear vertices) divides by zero and yields
          // NaN, and a NaN comparison guard breaks under -ffast-math.
          glm::vec3 cross = glm::cross(edgeA, edgeB);
          if(glm::length(cross) > 1e-12f) {
            normal = glm::normalize(cross);
          }
        }

        glm::vec3 normals[6] = { normal, normal, normal, normal, normal, normal };

        buffers.attribute(1, 1, 3, normals, sizeof(normals));

        glBuffers::unbind();
        
        glCheckError();
        
      }

    
  };

} // namespace ogl

#endif /* _H_OGL_GLQUAD_H_ */
