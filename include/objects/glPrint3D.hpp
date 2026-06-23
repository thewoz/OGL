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
  // glyph atlas is shared through glFont, so this object only owns the small
  // dynamic quad buffer (vao/vbo). 'color' is inherited from glObject.
  //****************************************************************************/
  class glPrint3D : public glObject {

  private:

    GLuint vao;
    GLuint vbo;

    glm::vec3 coord;

    float scale;

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
    glPrint3D(const std::string _text, const glm::vec3 & _coord, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1.0, float _isDynamicScale = false, const std::string & _name = "") {
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
    ~glPrint3D() { cleanInGpu(); }

    glPrint3D(glPrint3D &&) noexcept = default;
    glPrint3D & operator = (glPrint3D &&) noexcept = default;
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(const std::string _text, const glm::vec3 & _coord, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1.0, float _isDynamicScale = false) {
      
      DEBUG_LOG("glPrint3D::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initText();
      
      text = _text;
      
      coord = _coord;
      
      color = _color;
      
      scale = _scale;
      
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
      
      scale = _scale;
      
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
            
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      //glEnable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      
      glEnable(GL_BLEND);
            
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      glActiveTexture(GL_TEXTURE0);
      
      glBindVertexArray(vao);
            
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          
      float tmpX = screen.x;
      
      float _scale = scale;
      
      if(isDynamicScale) {
        float distance = glm::distance(camera.getPosition(), coord);
        _scale = scale / distance;
      }
      
      // iterate through all characters
      for(std::string::const_iterator c = text.begin(); c != text.end(); c++) {
        
        // skip characters that were not loaded (e.g. non-ASCII)
        const glFont::Character_t * chp = glFont::instance().get(*c);
        if(chp == nullptr) continue;
        const glFont::Character_t & ch = *chp;

        float xpos = tmpX + ch.Bearing.x * _scale;
        float ypos = screen.y - (ch.Size.y - ch.Bearing.y) * _scale;
        
        float w = ch.Size.x * _scale;
        float h = ch.Size.y * _scale;
        
        // update VBO for each character
        float vertices[6][4] = {
          { xpos,     ypos + h,   0.0f, 0.0f },
          { xpos,     ypos,       0.0f, 1.0f },
          { xpos + w, ypos,       1.0f, 1.0f },
          
          { xpos,     ypos + h,   0.0f, 0.0f },
          { xpos + w, ypos,       1.0f, 1.0f },
          { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
                
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels
        tmpX += (ch.Advance >> 6) * _scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
                
      }
      
      glBindVertexArray(0);
      
      glBindTexture(GL_TEXTURE_2D, 0);
      
      glDepthMask(GL_TRUE);
      
      glCheckError();
      
      
    }
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {

      DEBUG_LOG("glPrint3D::setInGpu(" + name + ")");

      // Build the shared glyph atlas once (no-op if another text object already did).
      glFont::instance().load();

      // configure VAO/VBO for texture quads
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);
      
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
      glEnableVertexAttribArray(0);
                  
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
      glCheckError();
      
    }
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {

        // Only the per-instance quad buffer is freed here; the glyph textures
        // belong to the shared glFont atlas and must outlive this object.
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

        isInitedInGpu = false;

      }

    }


  };


} /* namespace ogl */

#endif /* _H_OGL_PRINT3D_H_ */
