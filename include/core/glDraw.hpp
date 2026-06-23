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

#ifndef _H_OGL_DRAW_H_
#define _H_OGL_DRAW_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

//****************************************************************************
// namespace ogl::draw
//****************************************************************************
namespace ogl::draw {

  //****************************************************************************//
  // namespace utils
  //****************************************************************************//
  namespace utils {

    //****************************************************************************//
    // norm()
    //****************************************************************************//
    inline double norm(const ImVec2 & v) { return std::sqrt(v.x * v.x + v.y * v.y); }

    //****************************************************************************//
    // shrinkLine() - move the endpoints inward along the segment.
    //****************************************************************************//
    inline void shrinkLine(ImVec2 & A, ImVec2 & B, float cutFromA, float cutFromB) {

      ImVec2 dir = B - A;

      float length = (float)norm(dir);

      if(length <= 1e-6f) {
        fprintf(stderr, "WARNING [shrinkLine]: line too short\n");
        return;
      }

      dir *= (1.0f / length);

      // Don't trim more than the whole segment (it would flip the line).
      float totalCut = cutFromA + cutFromB;

      if(totalCut >= length) {
        fprintf(stderr, "WARNING [shrinkLine]: cut too big\n");
        return;
      }

      A += dir * cutFromA;
      B -= dir * cutFromB;

    }

    //****************************************************************************//
    // shrinkLine()
    //****************************************************************************//
    inline void shrinkLine(ImVec2 & A, ImVec2 & B, float cut) {
      shrinkLine(A, B, cut, cut);
    }

  } /* namespace utils */


  //****************************************************************************//
  // rotatedText()
  //****************************************************************************//
  inline void rotatedText(ImDrawList * drawList, const std::string & text, ImFont * font, float fontSizePx, ImU32 color, const ImVec2 & center, double angleDeg)  {

    ImVec2 textSize = font->CalcTextSizeA(fontSizePx, FLT_MAX, 0.0f, text.c_str());

    ImVec2 pos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);

    int vtxStart = drawList->VtxBuffer.Size;

    drawList->AddText(font, fontSizePx, pos, color, text.c_str());

    int vtxEnd = drawList->VtxBuffer.Size;

    float angleRad = (float)(angleDeg * M_PI / 180.0);
    float s = std::sin(angleRad);
    float c = std::cos(angleRad);

    for(int i=vtxStart; i<vtxEnd; ++i) {

      ImVec2 p = drawList->VtxBuffer[i].pos;

      p.x -= center.x;
      p.y -= center.y;

      drawList->VtxBuffer[i].pos = ImVec2(center.x + p.x * c - p.y * s, center.y + p.x * s + p.y * c);

    }

  }

  //****************************************************************************//
  // distMarker()
  //****************************************************************************//
  inline void distMarker(ImDrawList * drawList,
                         ImVec2 A,
                         ImVec2 B,
                         float thicknessPx,
                         ImU32 color,
                         float shrinkPx = 20.0f,
                         float endMarkSizePx = 12.0f) {

    if(shrinkPx != 0.0f) utils::shrinkLine(A, B, shrinkPx);

    // linea principale
    drawList->AddLine(A, B, color, thicknessPx);

    // direzione della linea in coordinate schermo
    ImVec2 dir(B.x - A.x, B.y - A.y);

    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if(len < 1.0f) return;

    dir.x /= len;
    dir.y /= len;

    // perpendicolare
    ImVec2 perp(-dir.y, dir.x);

    // tacche alle estremità
    ImVec2 p1a(A.x + perp.x * endMarkSizePx, A.y + perp.y * endMarkSizePx);
    ImVec2 p1b(A.x - perp.x * endMarkSizePx, A.y - perp.y * endMarkSizePx);

    ImVec2 p2a(B.x + perp.x * endMarkSizePx, B.y + perp.y * endMarkSizePx);
    ImVec2 p2b(B.x - perp.x * endMarkSizePx, B.y - perp.y * endMarkSizePx);

    drawList->AddLine(p1a, p1b, color, thicknessPx);
    drawList->AddLine(p2a, p2b, color, thicknessPx);

  }

  //****************************************************************************//
  // distMarker()
  //****************************************************************************//
  inline void distMarker(ImDrawList * drawList,
                         ImVec2 A,
                         ImVec2 B,
                         float thicknessPx,
                         const std::string & label,
                         ImFont * font,
                         float fontSizePx,
                         ImU32 color,
                         float textOffsetPx = 15.0f,
                         float shrinkPx = 20.0f,
                         float endMarkSizePx = 12.0f) {

    if(shrinkPx != 0.0f) utils::shrinkLine(A, B, shrinkPx);

    distMarker(drawList, A, B, thicknessPx, color, 0.0f, endMarkSizePx);

    // direzione e perpendicolare del segmento accorciato
    ImVec2 dir(B.x - A.x, B.y - A.y);

    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if(len < 1.0f) return;

    dir.x /= len;
    dir.y /= len;

    ImVec2 perp(-dir.y, dir.x);

    // punto medio
    ImVec2 mid((A.x + B.x) * 0.5f, (A.y + B.y) * 0.5f);

    // Il testo viene ruotato per allinearsi alla retta, quindi la sua altezza
    // Aggiungo metà altezza del testo allo spostamento
    ImVec2 textSize = font->CalcTextSizeA(fontSizePx, FLT_MAX, 0.0f, label.c_str());

    float off = textOffsetPx + textSize.y * 0.5f;

    // centro del testo spostato lungo la perpendicolare
    ImVec2 textCenter(mid.x + perp.x * off, mid.y + perp.y * off);

    // angolo della linea
    double angleDeg = std::atan2((double)(B.y - A.y), (double)(B.x - A.x)) * 180.0 / M_PI;

    // mantiene il testo leggibile
    if(angleDeg >  90.0) angleDeg -= 180.0;
    else if(angleDeg < -90.0) angleDeg += 180.0;

    rotatedText(drawList, label, font, fontSizePx, color, textCenter, angleDeg);

  }


} /* namespace ogl::draw */


#endif /* _H_OGL_DRAW_H_ */
