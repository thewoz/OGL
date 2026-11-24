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

#ifndef _H_OGL_POINTS_H_
#define _H_OGL_POINTS_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glPoints
  //****************************************************************************/
  // Renders a point cloud as sphere impostors (one GL_POINTS vertex per point,
  // shaded as a little 3D sphere in points.fs). It derives from glShape and is
  // shaded by the scene light passed to render(); with no scene it falls back to
  // a camera head light. The impostors are not shadow casters/receivers.
  //****************************************************************************/
  class glPoints : public glShape {
    
  private:
        
    
    std::vector<glm::vec3> points;
    std::vector<glm::vec4> colors;

    float radius = 1.0f;

    // how the impostors are shaded (see points.fs); PHONG keeps the old look
    int shadingMode = PHONG;

  public:

    // Shading modes for the sphere impostors:
    //   FLAT    - uniform matte colour, no lighting, view independent
    //   DIFFUSE - matte sphere (ambient + diffuse, no specular)
    //   PHONG   - full Phong with specular highlight (default, shiny)
    enum SHADING { FLAT = 0, DIFFUSE = 1, PHONG = 2 };

    
    //****************************************************************************/
    // glPoints()
    //****************************************************************************/
    glPoints(const std::string & _name = "") { name = _name; }
    glPoints(const std::vector<glm::vec3> & _points, const glm::vec4 & _color = glm::vec4(1.0), float _radius = 1, const std::string & _name = "") {
      name = _name;
      init(_points, _color, _radius);
    }
    glPoints(const std::vector<glm::vec3> & _points, const std::vector<glm::vec4> & _color, float _radius = 1, const std::string & _name = "") {
      name = _name;
      init(_points, _color, _radius);
    }

    //****************************************************************************/
    // ~glPoints()
    //****************************************************************************/
    ~glPoints() { releaseInGpu(); }

    glPoints(glPoints &&) noexcept = default;
    glPoints & operator = (glPoints &&) noexcept = default;
 
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _points, const glm::vec4 & _color = glm::vec4(1.0), float _radius = 1) {
      
      DEBUG_LOG("glPoints::init(" + name + ")");
         
      shader.setName(name);
      shader.initPoints();
      
      points = _points;

      // assign, not resize: on a re-init resize() would keep the old colors
      colors.assign(points.size(), _color);

      radius = _radius;

      isInited = true;
      isToUpdateInGpu = true; // re-init after a render must re-upload

    }
   
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::vector<glm::vec3> & _points, const std::vector<glm::vec4> & _color, float _radius = 1) {
      
      DEBUG_LOG("glPoints::init(" + name + ")");
      
      shader.setName(name);
      shader.initPoints();

      points = _points;

      colors = _color;

      // the color buffer must cover every point, otherwise the GPU reads past
      // the end of the color VBO during rendering (same guard as glLines)
      if(colors.size() < points.size())
        colors.resize(points.size(), colors.empty() ? glm::vec4(1.0f) : colors.back());

      radius = _radius;

      isInited = true;
      isToUpdateInGpu = true; // re-init after a render must re-upload

    }
    
    //****************************************************************************/
    // setRadius()
    //****************************************************************************/
    void setRadius(float _radius) { radius = _radius; }

    //****************************************************************************/
    // setShadingMode() - FLAT / DIFFUSE / PHONG (see SHADING)
    //****************************************************************************/
    void setShadingMode(int _mode) { shadingMode = _mode; }
    int  getShadingMode() const { return shadingMode; }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, int from = 0, int to = -1, int index = -1, const glScene * scene = nullptr) {
            
      DEBUG_LOG("glPoints::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "ERROR [glPoints]: must be initialized before rendering\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       camera.getView());
      shader.setUniform("model",      modelMatrix);
      shader.setUniform("pointSize",  radius);
      shader.setUniform("shadingMode", shadingMode);
      shader.setUniform("shininess",        shininess);
      shader.setUniform("specularStrength", specularStrength);
      shader.setUniform("viewport",   camera.getViewport());

      // Shade the impostors with the scene light (head-light fallback by default).
      // The point impostors receive light but do not sample the shadow map.
      if(scene) scene->setInShader(shader, camera.getView());
      else      glLight().setInShader(shader, camera.getView());
            
      int n = (int) points.size();

      if(to == -1) to = n;

      // Clamp the requested [from, to) range to the valid window so a bad
      // argument can't make glDrawArrays read past the buffer.
      if(from < 0) from = 0;
      if(to   > n) to   = n;

      // 'to' is an end index (exclusive); the draw call needs a vertex count
      int count = to - from;

      if(index != -1) {
        // draw a single point at absolute position (from + index)
        int at = from + index;
        if(at < 0 || at >= n) return;
        from  = at;
        count = 1;
      }

      if(count <= 0) return;

      glEnable(GL_PROGRAM_POINT_SIZE);

      buffers.bind();

      //glEnable(GL_CULL_FACE);
      //glCullFace(GL_BACK);

      // points.fs antialiases the disc edge through its alpha, which here is
      // turned into MSAA coverage instead of being blended. Blending the rim
      // made the picture depend on the draw order: the impostors write their
      // spherical depth, so a translucent rim of a point in front, drawn first,
      // stops a point behind it from showing under the rim and the background
      // shows through instead (a white ring on a white background). Coverage
      // and depth are resolved per sample, so the order does not matter.
      glDisable(GL_BLEND);
      glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

      glDrawArrays(GL_POINTS, from, count);

      glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
      glDisable(GL_PROGRAM_POINT_SIZE);

      glBindVertexArray(0);
      
      glCheckError();

    }
    
  private:
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {
      
      DEBUG_LOG("glPoints::setInGpu(" + name + ")");

      prepareBuffers(2);

      buffers.attribute(0, 0, 3, points.data(), points.size() * sizeof(glm::vec3));
      buffers.attribute(1, 1, 4, colors.data(), colors.size() * sizeof(glm::vec4));

      glBuffers::unbind();

      glCheckError();

      isInitedInGpu = true;

    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_POINTS_H_ */
