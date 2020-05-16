/*
 * MIT License
 *
 * Copyright (c) 2017 Leonardo Parisi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _H_MPL_OPENGL_PRINT_H_
#define _H_MPL_OPENGL_PRINT_H_

#include <cstdio>
#include <cstdlib>

#include <map>

#include "glObject.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  class glPrint : public glObject {
    
  private:
    
    GLuint vao;
    GLuint vbo;
    
    struct Character_t {
      unsigned int TextureID; // ID handle of the glyph texture
      glm::ivec2   Size;      // Size of glyph
      glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
      unsigned int Advance;   // Horizontal offset to advance to next glyph
    };
    
    std::map<GLchar, Character_t> Characters;
    
  public:
    
    //****************************************************************************/
    // glPrint
    //****************************************************************************/
    glPrint(const std::string & _name = "") : glObject(_name) { }
  
    
    //****************************************************************************/
    // ~glPrint
    //****************************************************************************/
    ~glPrint() { cleanInGpu(); }
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init() {
      
      DEBUG_LOG("glPrint::init(" + name + ")");
      
      glObject::initText();

      isInited = true;
      
    }
    
    //****************************************************************************/
    // print
    //****************************************************************************/
    void print(const glm::mat4 & projection, std::string text, float x, float y, glm::vec3 _color, float scale = 1) {
      
      glEnable(GL_CULL_FACE);
      
      glEnable(GL_BLEND);
      
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      color = _color;
      
      glObject::renderBegin(projection);
            
      glActiveTexture(GL_TEXTURE0);
      
      glBindVertexArray(vao);
      
      // iterate through all characters
      for(std::string::const_iterator c = text.begin(); c != text.end(); c++) {
        
        Character_t ch = Characters[*c];
        
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        
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
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
      
      }
      
      glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);
      
      glObject::renderEnd();
      
      glDisable(GL_CULL_FACE);
      
      glDisable(GL_BLEND);

    }
    
  private:
    
    //****************************************************************************/
    // setInGpu
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glPrint::setInGpu(" + name + ")");
      
      FT_Library ft;
      
      // All functions return a value different than 0 whenever an error occurred
      if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
      
      // load font as face
      FT_Face face;
      if (FT_New_Face(ft, "/usr/local/include/ogl/fonts/arial.ttf", 0, &face))
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
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                     GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer
                     );
        
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // now store character for later use
        Character_t character = {
          texture,
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
      
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
      
      glEnableVertexAttribArray(0);
      
      glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
      
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      
      glBindVertexArray(0);
      
    }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
        
        isInitedInGpu = false;
        
      }
      
    }
    
    
  };


} /* namespace ogl */

#endif /* _H_MPL_OPENGL_PRINT_H_ */
