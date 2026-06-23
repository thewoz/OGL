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

#ifndef _H_OGL_FONT_H_
#define _H_OGL_FONT_H_

#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdio>
#include <cstdlib>

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glFont
  //****************************************************************************/
  // Shared glyph atlas used by both glPrint2D and glPrint3D. The ASCII glyph
  // textures are loaded from FreeType exactly once (program-lifetime) instead
  // of per text instance, which used to upload 128 textures for every label.
  //
  // It is a process-wide singleton: call glFont::instance().load() with an
  // active GL context (the text objects do this on their first setInGpu), then
  // look glyphs up through get(). The atlas is shared, so individual text
  // objects must not delete its textures.
  //****************************************************************************/
  class glFont {

  public:

    struct Character_t {
      unsigned int TextureID; // ID handle of the glyph texture
      glm::ivec2   Size;      // Size of glyph
      glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
      unsigned int Advance;   // Horizontal offset to advance to next glyph
    };

  private:

    std::map<GLchar, Character_t> characters;

    bool loaded = false;

    glFont() { }

  public:

    //****************************************************************************/
    // instance() - the single shared font atlas
    //****************************************************************************/
    static glFont & instance() {
      static glFont instance;
      return instance;
    }

    //****************************************************************************/
    // isLoaded()
    //****************************************************************************/
    inline bool isLoaded() const { return loaded; }

    //****************************************************************************/
    // get() - glyph for a character, or nullptr if it was not loaded
    //****************************************************************************/
    const Character_t * get(GLchar c) const {
      auto it = characters.find(c);
      return (it == characters.end()) ? nullptr : &it->second;
    }

    //****************************************************************************/
    // load() - build the ASCII glyph atlas (idempotent, needs a GL context)
    //****************************************************************************/
    void load(const std::string & fontPath = "/usr/local/include/ogl/data/fonts/arial.ttf", unsigned int pixelSize = 48) {

      if(loaded) return;

      FT_Library ft;

      // All functions return a value different than 0 whenever an error occurred
      if(FT_Init_FreeType(&ft)) {
        fprintf(stderr, "ERROR [glFont]: could not init FreeType library\n");
        abort();
      }

      // load font as face
      FT_Face face;
      if(FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        fprintf(stderr, "ERROR [glFont]: failed to load font\n");
        abort();
      }

      // set size to load glyphs as
      FT_Set_Pixel_Sizes(face, 0, pixelSize);

      // disable byte-alignment restriction
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      // load first 128 characters of ASCII set
      for(unsigned char c = 0; c < 128; c++) {

        // Load character glyph
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          fprintf(stderr, "WARNING [glFont]: failed to load glyph for char %d\n", (int)c);
          continue;
        }

        // generate texture
        GLuint textureID;
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

        characters.insert(std::pair<char, Character_t>(c, character));

      }

      glBindTexture(GL_TEXTURE_2D, 0);

      // destroy FreeType once we're finished
      FT_Done_Face(face);
      FT_Done_FreeType(ft);

      loaded = true;

      glCheckError();

    }

  };

} /* namespace ogl */

#endif /* _H_OGL_FONT_H_ */
