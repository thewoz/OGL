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
  // this object only owns the small dynamic quad buffer (vao/vbo). 'color' is
  // inherited from glObject.
  //****************************************************************************/
  class glPrint2D : public glObject {

  private:

    GLuint vao;
    GLuint vbo;

    float x;
    float y;

    float scale;

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
    ~glPrint2D() { cleanInGpu(); }

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
      
      scale = _scale;
      
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
      
      scale = _scale;
      
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
                  
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      glEnable(GL_BLEND);
      
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                  
      glBindVertexArray(vao);

      glActiveTexture(GL_TEXTURE0);
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      float tmpX = x;
      float tmpY = y;

      // iterate through all characters
      for(std::string::const_iterator c = text.begin(); c != text.end(); c++) {
        
        if(*c == '\n') {
          const glFont::Character_t * chA = glFont::instance().get('a');
          float lineHeight = chA ? chA->Size.y * scale : 0.0f;
          tmpX  = x;
          tmpY -= 2 * lineHeight;
          continue;
        }

        // skip characters that were not loaded (e.g. non-ASCII)
        const glFont::Character_t * chp = glFont::instance().get(*c);
        if(chp == nullptr) continue;
        const glFont::Character_t & ch = *chp;
                
        float xpos = tmpX + ch.Bearing.x * scale;
        float ypos = tmpY - (ch.Size.y - ch.Bearing.y) * scale;
        
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        
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
        tmpX += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        
        glCheckError();
        
      }
      
      glBindTexture(GL_TEXTURE_2D, 0);
      
      glBindVertexArray(0);
      
      glCheckError();
      
    }
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {

      DEBUG_LOG("glPrint2D::setInGpu(" + name + ")");

      // Build the shared glyph atlas once (no-op if another text object already did).
      glFont::instance().load();

      // configure VAO/VBO for texture quads
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
      glEnableVertexAttribArray(0);

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            
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

#endif /* _H_OGL_PRINT_H_ */
