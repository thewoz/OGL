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

#ifndef _H_OGL_PRINT3D_H_
#define _H_OGL_PRINT3D_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

#include <string>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glPrint3D
  //****************************************************************************/
  // Draws text anchored at a 3D world position (projected to the screen). The
  // glyph atlas is shared through glFont, so this object only owns the dynamic
  // buffer its string is uploaded into. 'color' is inherited from glObject.
  //****************************************************************************/
  class glPrint3D : public glObject {

  private:


    // initialized: an object built with the name-only constructor can be
    // rendered without an init(), and must not read garbage
    glm::vec3 coord = glm::vec3(0.0f);

    float textScale = 1.0f;

    std::string text;

    bool isDynamicScale = false;

  public:
    
    //****************************************************************************/
    // glPrint3D()
    //****************************************************************************/
    glPrint3D(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glPrint()
    //****************************************************************************/
    glPrint3D(const std::string & _text, const glm::vec3 & _coord, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1.0, bool _isDynamicScale = false, const std::string & _name = "") {
      name = _name;
      init(_text, _coord, _color, _scale, _isDynamicScale);
    }
    
    //****************************************************************************/
    // glPrint()
    //****************************************************************************/
    glPrint3D(const glm::vec3 & _color, float _scale = 1.0, bool _isDynamicScale = false, const std::string & _name = "") {
      name = _name;
      init("", glm::vec3(0,0,0), _color, _scale, _isDynamicScale);
    }
    
    //****************************************************************************/
    // ~glPrint3D()
    //****************************************************************************/
    ~glPrint3D() { releaseInGpu(); }

    glPrint3D(glPrint3D &&) noexcept = default;
    glPrint3D & operator = (glPrint3D &&) noexcept = default;
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::string & _text, const glm::vec3 & _coord, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1.0, bool _isDynamicScale = false) {
      
      DEBUG_LOG("glPrint3D::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initText();
      
      text = _text;
      
      coord = _coord;
      
      color = _color;
      
      textScale = _scale;
      
      isDynamicScale = _isDynamicScale;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, const std::string & _text, const glm::vec3 & _coord, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1) {
      
      DEBUG_LOG("glPrint3D::render(" + name + ")");
      
      text = _text;
      
      coord = _coord;
      
      color = _color;
      
      textScale = _scale;
      
      _render(camera);
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, const std::string & _text) {
      
      DEBUG_LOG("glPrint3D::render(" + name + ")");
      
      text = _text;
      
      _render(camera);
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {
      
      DEBUG_LOG("glPrint3D::render(" + name + ")");
      
      _render(camera);
      
    }
    
    
  private:
    
    //****************************************************************************/
    // _render()
    //****************************************************************************/
    void _render(const glCamera & camera) {
      
      DEBUG_LOG("glPrint3D::_render(" + name + ")");
      
      glm::vec2 screen;
      
      if(!camera.screenPosition(coord, getModelMatrix(), screen)) return;

      if(!isInited){
        shader.setName(name);
        shader.initText();
        isInited = true;
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera.getOrthoProjection());
      shader.setUniform("color",      color);
      // Bind the glyph sampler to unit 0 explicitly instead of relying on the
      // uniform's default value being 0.
      shader.setUniform("text", 0);

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      //glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      
      glEnable(GL_BLEND);
            
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      buffers.bind();
            
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          
      float _scale = textScale;
      
      if(isDynamicScale) {
        // Clamp the distance: if the camera reaches the text position the
        // division would produce inf/NaN and blow up the glyph vertices.
        // The eye comes from the view matrix and not from camera.getPosition(),
        // which in ORBIT mode is not where the eye is (see glInfiniteGrid).
        glm::vec3 eye = glm::vec3(glm::inverse(camera.getView())[3]);
        float distance = glm::max(glm::distance(eye, coord), 1e-3f);
        _scale = textScale / distance;
      }
      
      // the whole string in one draw call, see glFont::draw()
      glFont::instance().draw(buffers.buffer(0), text, screen.x, screen.y, _scale);
      
      glBindVertexArray(0);

      glDepthMask(GL_TRUE);

      // Don't leak blending to whatever is drawn next.
      glDisable(GL_BLEND);

      glCheckError();
      
      
    }
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glPrint3D::setInGpu(" + name + ")");

      // Build the shared glyph atlas once (no-op if another text object already did).
      glFont::instance().load();

      // configure VAO/VBO for texture quads
      prepareBuffers(1);

      // one vec4 per vertex, <position, texture coordinate>: filled at every draw
      buffers.attribute(0, 0, 4, nullptr, 0, GL_DYNAMIC_DRAW);

      glBuffers::unbind();
      
      glCheckError();
      
    }


  };


} /* namespace ogl */

#endif /* _H_OGL_PRINT3D_H_ */
