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
#include <cstring>

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
  // Shared glyph atlas used by both glPrint2D and glPrint3D. The ASCII glyphs
  // are rendered by FreeType exactly once per window, packed into a single
  // texture (kept for the program lifetime), and a whole string is drawn with
  // one buffered draw call, see draw().
  //
  // It is a process-wide singleton: call glFont::instance().load() with an
  // active GL context (the text objects do this in their setInGpu), then draw
  // strings with draw(). The atlas is shared, so individual text objects must
  // not delete its texture.
  //****************************************************************************/
  class glFont {

  public:

    struct Character_t {
      glm::vec2    UvMin;     // Texture coordinates of the top-left corner in the atlas
      glm::vec2    UvMax;     // Texture coordinates of the bottom-right corner in the atlas
      glm::ivec2   Size;      // Size of glyph, in pixels
      glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
      unsigned int Advance = 0;   // Horizontal offset to advance to next glyph, in 1/64 of a pixel
    };

  private:

    struct Atlas_t {
      GLuint texture = 0;
      std::map<GLchar, Character_t> characters;
    };

    // One atlas per window: the texture lives in the GL context it was
    // created in and the contexts of the windows are not shared, so the name
    // of one window means nothing in another. The key is the window id
    // (currentWindowID()), which is never reused.
    std::map<uint32_t, Atlas_t> atlases;

    // vertices of the string being drawn, kept to avoid an allocation per draw
    std::vector<float> quads;

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
    // whether the atlas of the current window has been built
    inline bool isLoaded() const { return atlases.count(currentWindowID()) != 0; }

    //****************************************************************************/
    // get() - glyph for a character in the current window, or nullptr if it
    // was not loaded
    //****************************************************************************/
    const Character_t * get(GLchar c) const {
      auto atlas = atlases.find(currentWindowID());
      if(atlas == atlases.end()) return nullptr;
      auto it = atlas->second.characters.find(c);
      return (it == atlas->second.characters.end()) ? nullptr : &it->second;
    }

    //****************************************************************************/
    // load() - build the ASCII glyph atlas (idempotent, needs a GL context)
    //****************************************************************************/
    void load(const std::string & fontPath = OGL_RESOURCE_DIR "/data/fonts/arial.ttf", unsigned int pixelSize = 48) {

      const uint32_t window = currentWindowID();

      if(atlases.count(window) != 0) return;

      Atlas_t & atlas = atlases[window];

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

      // The glyphs are packed in rows ("shelves") of a texture of fixed width,
      // left to right, with a row as tall as its tallest glyph. Around each
      // glyph there is a border of empty texels, so that the linear filter at
      // the edge of one glyph never picks up its neighbour.
      const int atlasWidth = 512;
      const int border     = 2;

      struct Bitmap_t { int x = 0, y = 0, width = 0, height = 0; std::vector<unsigned char> pixels; };

      std::map<GLchar, Bitmap_t> bitmaps;

      int penX = border, penY = border, rowHeight = 0;

      // load first 128 characters of ASCII set
      for(unsigned char c = 0; c < 128; c++) {

        // Load character glyph
        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
          fprintf(stderr, "WARNING [glFont]: failed to load glyph for char %d\n", (int)c);
          continue;
        }

        const FT_Bitmap & source = face->glyph->bitmap;

        Bitmap_t bitmap;

        bitmap.width  = (int) source.width;
        bitmap.height = (int) source.rows;

        if(bitmap.width + 2 * border > atlasWidth) {
          fprintf(stderr, "WARNING [glFont]: glyph for char %d too large for the atlas\n", (int)c);
          continue;
        }

        // next shelf
        if(penX + bitmap.width + border > atlasWidth) {
          penX  = border;
          penY += rowHeight + border;
          rowHeight = 0;
        }

        bitmap.x = penX;
        bitmap.y = penY;

        // a row of the FreeType bitmap is 'pitch' bytes long, not 'width'
        bitmap.pixels.resize((size_t) bitmap.width * (size_t) bitmap.height);
        for(int row = 0; row < bitmap.height; ++row)
          memcpy(&bitmap.pixels[(size_t) row * (size_t) bitmap.width], source.buffer + (ptrdiff_t) row * source.pitch, (size_t) bitmap.width);

        penX += bitmap.width + border;
        if(bitmap.height > rowHeight) rowHeight = bitmap.height;

        Character_t character = {
          glm::vec2(0.0f),
          glm::vec2(0.0f),
          glm::ivec2(bitmap.width, bitmap.height),
          glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
          (unsigned int)face->glyph->advance.x
        };

        atlas.characters.insert(std::pair<GLchar, Character_t>((GLchar) c, character));

        bitmaps.insert(std::pair<GLchar, Bitmap_t>((GLchar) c, std::move(bitmap)));

      }

      const int atlasHeight = penY + rowHeight + border;

      // destroy FreeType once we're finished
      FT_Done_Face(face);
      FT_Done_FreeType(ft);

      // copy the glyphs into the atlas image and work out their texture coordinates
      std::vector<unsigned char> image((size_t) atlasWidth * (size_t) atlasHeight, 0);

      for(auto & entry : bitmaps) {

        const Bitmap_t & bitmap = entry.second;

        for(int row = 0; row < bitmap.height; ++row)
          memcpy(&image[(size_t) (bitmap.y + row) * (size_t) atlasWidth + (size_t) bitmap.x], &bitmap.pixels[(size_t) row * (size_t) bitmap.width], (size_t) bitmap.width);

        Character_t & character = atlas.characters[entry.first];

        character.UvMin = glm::vec2(bitmap.x / (float) atlasWidth, bitmap.y / (float) atlasHeight);
        character.UvMax = glm::vec2((bitmap.x + bitmap.width) / (float) atlasWidth, (bitmap.y + bitmap.height) / (float) atlasHeight);

      }

      // disable byte-alignment restriction: one byte per texel
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      glGenTextures(1, &atlas.texture);
      glBindTexture(GL_TEXTURE_2D, atlas.texture);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, image.data());

      // set texture options
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glBindTexture(GL_TEXTURE_2D, 0);

      glCheckError();

    }

    //****************************************************************************/
    // draw() - draw a string with one draw call: the quads of all its glyphs go
    // into 'vbo' in one upload. (x, y) is the pen position of the first glyph,
    // in the coordinates of the projection the caller set, and 'scale' the
    // size factor with respect to the pixel size of the atlas. A newline goes
    // back to x and down one line; characters that are not in the atlas (non
    // ASCII) are skipped.
    //
    // The caller has its text shader in use, with the glyph sampler on texture
    // unit 0, its vertex array bound (attribute 0 a vec4 <position, texture
    // coordinate> read from 'vbo') and the blending set.
    //****************************************************************************/
    void draw(GLuint vbo, const std::string & text, float x, float y, float scale) {

      auto found = atlases.find(currentWindowID());

      if(found == atlases.end()) return;

      const Atlas_t & atlas = found->second;

      // height of a line, the same the text always had: twice the 'a'
      auto reference = atlas.characters.find('a');
      const float lineHeight = (reference != atlas.characters.end()) ? 2.0f * reference->second.Size.y * scale : 0.0f;

      quads.clear();

      float penX = x;
      float penY = y;

      for(std::string::const_iterator c = text.begin(); c != text.end(); ++c) {

        // newline: carriage return + line feed
        if(*c == '\n') {
          penX  = x;
          penY -= lineHeight;
          continue;
        }

        // skip characters that were not loaded (e.g. non-ASCII)
        auto it = atlas.characters.find(*c);
        if(it == atlas.characters.end()) continue;

        const Character_t & ch = it->second;

        // a glyph with no image (the space) only moves the pen
        if(ch.Size.x > 0 && ch.Size.y > 0) {

          float xpos = penX + ch.Bearing.x * scale;
          float ypos = penY - (ch.Size.y - ch.Bearing.y) * scale;

          float w = ch.Size.x * scale;
          float h = ch.Size.y * scale;

          // two triangles, the top of the glyph is the first row of its image
          const float vertices[6][4] = {
            { xpos,     ypos + h,   ch.UvMin.x, ch.UvMin.y },
            { xpos,     ypos,       ch.UvMin.x, ch.UvMax.y },
            { xpos + w, ypos,       ch.UvMax.x, ch.UvMax.y },

            { xpos,     ypos + h,   ch.UvMin.x, ch.UvMin.y },
            { xpos + w, ypos,       ch.UvMax.x, ch.UvMax.y },
            { xpos + w, ypos + h,   ch.UvMax.x, ch.UvMin.y }
          };

          quads.insert(quads.end(), &vertices[0][0], &vertices[0][0] + 24);

        }

        // the advance is in 1/64 of a pixel: shifting by 6 gives pixels
        penX += (float) (ch.Advance >> 6) * scale;

      }

      if(quads.empty()) return;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, atlas.texture);

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (quads.size() * sizeof(float)), quads.data(), GL_DYNAMIC_DRAW);

      glDrawArrays(GL_TRIANGLES, 0, (GLsizei) (quads.size() / 4));

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindTexture(GL_TEXTURE_2D, 0);

    }

  };

} /* namespace ogl */

#endif /* _H_OGL_FONT_H_ */
