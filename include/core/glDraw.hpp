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
        fprintf(stderr, "shrinkLine() warning: line too short\n");
        return;
      }

      dir *= (1.0f / length);

      // Don't trim more than the whole segment (it would flip the line).
      float totalCut = cutFromA + cutFromB;

      if(totalCut >= length) {
        fprintf(stderr, "shrinkLine() warning: cut too big\n");
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
  // drawRotatedText()
  //****************************************************************************//
  inline void drawRotatedText(ImDrawList * drawList, const std::string & text, ImFont * font, const ImVec2 & center, double angleDeg, float fontPx, ImU32 col) {

    ImVec2 textSize = font->CalcTextSizeA(fontPx, FLT_MAX, 0.0f, text.c_str());

    ImVec2 pos(center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f);

    int vtxStart = drawList->VtxBuffer.Size;

    drawList->AddText(font, fontPx, pos, col, text.c_str());

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
  // drawDistMarker()
  //****************************************************************************//
  inline void drawDistMarker(ImDrawList * drawList,
                             ImVec2 A,
                             ImVec2 B,
                             const std::string & label,
                             ImFont * font,
                             ImU32 col,
                             float thickness = 2.0f,
                             float textOffset = 15.0f,
                             double fontScale = 0.8,
                             float shrinkPx = 20.0f,
                             float endMarkSizePx = 12.0f,
                             float distMarkerFontPx = 16.0f) {

    if(shrinkPx != 0.0f) utils::shrinkLine(A, B, shrinkPx);

    // linea principale
    drawList->AddLine(A, B, col, thickness);

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

    drawList->AddLine(p1a, p1b, col, thickness);
    drawList->AddLine(p2a, p2b, col, thickness);

    // punto medio
    ImVec2 mid((A.x + B.x) * 0.5f, (A.y + B.y) * 0.5f);

    // centro del testo spostato lungo la perpendicolare
    ImVec2 textCenter(mid.x + perp.x * textOffset, mid.y + perp.y * textOffset);

    // angolo della linea
    double angleRad = std::atan2((double)(B.y - A.y), (double)(B.x - A.x));

    double angleDeg = angleRad * 180.0 / M_PI;

    // mantiene il testo leggibile
    if(angleDeg >  90.0) angleDeg -= 180.0;
    else if(angleDeg < -90.0) angleDeg += 180.0;

    float fontPx = distMarkerFontPx * (float)(fontScale / 0.8);

    drawRotatedText(drawList, label, textCenter, font, angleDeg, fontPx, col);

  }


} /* namespace ogl::draw */


#endif /* _H_OGL_DRAW_H_ */
