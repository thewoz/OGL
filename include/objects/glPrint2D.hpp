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

#ifndef _H_OGL_PRINT_H_
#define _H_OGL_PRINT_H_

#include <cstdio>
#include <cstdlib>

#include <map>
#include <string>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glPrint2D
  //****************************************************************************/
  class glPrint2D : public glObject {
    
  private:
    
    GLuint vao;
    GLuint vbo;
    
    unsigned int textureID;

    float x;
    float y;
    
    float scale;
    
    glm::vec3 color;
    
    std::string text;
    
    struct Character_t {
      unsigned int TextureID; // ID handle of the glyph texture
      glm::ivec2   Size;      // Size of glyph
      glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
      unsigned int Advance;   // Horizontal offset to advance to next glyph
    };
    
    std::map<GLchar, Character_t> Characters;
    
  public:
    
    //****************************************************************************/
    // glPrint2D()
    //****************************************************************************/
    glPrint2D(const std::string & _name = "") { name = _name; }
    
    //****************************************************************************/
    // glPrint2D()
    //****************************************************************************/
    glPrint2D(float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1,  const std::string _text = "", const std::string & _name = "") {
      name = _name;
      init(_x, _y, _color, _scale, _text);
    }
    
    //****************************************************************************/
    // ~glPrint2D()
    //****************************************************************************/
    ~glPrint2D() { cleanInGpu(); }
    
    //****************************************************************************/
    // init()
    //****************************************************************************/
    void init(float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1, const std::string _text = "") {
      
      DEBUG_LOG("glPrint2D::init(" + name + ")");
      
      shader.setName(name);
      
      shader.initText2D();
      
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
    void render(const glCamera * camera, const std::string & _text, float _x, float _y, const glm::vec3 & _color = glm::vec3(1,1,1), float _scale = 1) {
      
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
    void render(const glCamera * camera, const std::string & _text) {
      
      DEBUG_LOG("glPrint2D::render(" + name + ")");

      text = _text;
      
      _render(camera);
      
    }
    
    //****************************************************************************/
    // render()
    //****************************************************************************/
    void render(const glCamera * camera) {
            
      DEBUG_LOG("glPrint2D::render(" + name + ")");
      
      _render(camera);
            
    }
    
    
  private:
    
    //****************************************************************************/
    // _render()
    //****************************************************************************/
    void _render(const glCamera * camera) {
      
      DEBUG_LOG("glPrint2D::_render(" + name + ")");
      
      if(!isInited){
        shader.setName(name);
        shader.initText2D();
        isInited = true;
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getOrthoProjection());
      shader.setUniform("color",      color);
                  
      glEnable(GL_CULL_FACE);
      
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
          const Character_t & ch = Characters['a'];
          tmpX  = x;
          tmpY += ch.Size.y * scale;
          continue;
        }
        
        const Character_t & ch = Characters[*c];
                
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
      
      glDisable(GL_CULL_FACE);
      
      glDisable(GL_BLEND);
      
    }
    
    //****************************************************************************/
    // setInGpu()
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glPrint2D::setInGpu(" + name + ")");
      
      FT_Library ft;
      
      // All functions return a value different than 0 whenever an error occurred
      if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
      
      // load font as face
      FT_Face face;
      if (FT_New_Face(ft, "/usr/local/include/ogl/data/fonts/arial.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
      
      // set size to load glyphs as
      FT_Set_Pixel_Sizes(face, 0, 48);
      
      // disable byte-alignment restriction
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      
      // load first 128 characters of ASCII set
      for(unsigned char c = 0; c < 128; c++) {
        
        // Load character glyph
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
          continue;
        }
        
        // generate texture
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);
        
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // now store character for later use
        Character_t character = {
          textureID,
          glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
          glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
          (unsigned int)face->glyph->advance.x
        };
        
        Characters.insert(std::pair<char, Character_t>(c, character));
        
      }
      
      glBindTexture(GL_TEXTURE_2D, 0);
      
      // destroy FreeType once we're finished
      FT_Done_Face(face);
      FT_Done_FreeType(ft);
      
      // configure VAO/VBO for texture quads
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
      glEnableVertexAttribArray(0);

      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            
      glBindVertexArray(0);
      
      glCheckError();

      
    }
    
    //****************************************************************************/
    // cleanInGpu()
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        
        glDeleteTextures(1, &textureID);

        isInitedInGpu = false;
        
      }
      
    }
    
    
  };


} /* namespace ogl */

#endif /* _H_OGL_PRINT_H_ */
