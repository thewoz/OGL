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

#ifndef _H_OGL_PRINT_H_
#define _H_OGL_PRINT_H_


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
  // Class glPrint2D
  //****************************************************************************/
  // Draws text in screen space. The glyph atlas is shared through glFont, so
  // this object only owns the dynamic buffer its string is uploaded into.
  // 'color' is inherited from glObject.
  //****************************************************************************/
  class glPrint2D : public glObject {

  private:


    // initialized: an object built with the name-only constructor can be
    // rendered without an init(), and must not read garbage
    float x = 0.0f;
    float y = 0.0f;

    float textScale = 1.0f;

    std::string text;

  public:
    
    //****************************************************************************/
    // glPrint2D()
    //****************************************************************************/
    glPrint2D(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glPrint2D()
    //****************************************************************************/
    glPrint2D(const std::string & _text, float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1, const std::string & _name = "") {
      name = _name;
      init(_text, _x, _y, _color, _scale);
    }
    
    //****************************************************************************/
    // glPrint2D()
    //****************************************************************************/
    glPrint2D(float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1, const std::string & _name = "") {
      name = _name;
      init("", _x, _y, _color, _scale);
    }
    
    //****************************************************************************/
    // ~glPrint2D()
    //****************************************************************************/
    ~glPrint2D() { releaseInGpu(); }

    glPrint2D(glPrint2D &&) noexcept = default;
    glPrint2D & operator = (glPrint2D &&) noexcept = default;
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1) { init("", _x, _y, _color, _scale); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::string & _text, float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1) {
      
      DEBUG_LOG("glPrint2D::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initText();
      
      text = _text;
      
      x = _x;
      y = _y;
      
      color = _color;
      
      textScale = _scale;
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, const std::string & _text, float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1) {
      
      DEBUG_LOG("glPrint2D::render(" + name + ")");

      text = _text;
      
      x = _x;
      y = _y;
      
      color = _color;
      
      textScale = _scale;
      
      _render(camera);
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera, const std::string & _text) {
      
      DEBUG_LOG("glPrint2D::render(" + name + ")");

      text = _text;
      
      _render(camera);
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera & camera) {
            
      DEBUG_LOG("glPrint2D::render(" + name + ")");
      
      _render(camera);
            
    }
    
    
  private:
    
    //****************************************************************************/
    // _render()
    //****************************************************************************/
    void _render(const glCamera & camera) {
      
      DEBUG_LOG("glPrint2D::_render(" + name + ")");
      
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

      glEnable(GL_BLEND);
      
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                  
      buffers.bind();

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      // the whole string in one draw call, see glFont::draw()
      glFont::instance().draw(buffers.buffer(0), text, x, y, textScale);
      
      glBindVertexArray(0);

      // Don't leak blending to whatever is drawn next.
      glDisable(GL_BLEND);

      glCheckError();

    }
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() override {

      DEBUG_LOG("glPrint2D::setInGpu(" + name + ")");

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

#endif /* _H_OGL_PRINT_H_ */
