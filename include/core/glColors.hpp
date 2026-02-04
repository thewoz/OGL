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

#ifndef _H_OGL_COLORS_H_
#define _H_OGL_COLORS_H_

#include <cstdio>
#include <cstdlib>

#include <map>
#include <string>
#include <random>
#include <cctype>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // Class glColors
  //****************************************************************************/
  class glColors {
    
  private:
    
    glColors() {}
    
    static std::map<std::string, glm::vec4> colors;

    static bool isInited;

    static std::string normalize(const std::string & value) {
      std::string lowered;
      lowered.reserve(value.size());
      for (unsigned char ch : value) {
        lowered.push_back(static_cast<char>(std::tolower(ch)));
      }
      return lowered;
    }
    
  public:
    
    //****************************************************************************//
    // getColor
    //****************************************************************************//
    static const glm::vec4 getRandom() {
      
      static std::default_random_engine generator;
      static std::uniform_real_distribution<float> distribution(0.0,1.0);
      
      return glm::vec4(distribution(generator), distribution(generator), distribution(generator), 1);
      
    }
    
    //****************************************************************************//
    // getColor
    //****************************************************************************//
    static const glm::vec4 & get(const std::string & str, bool createColor = false) {
         
      if(!isInited) init();
      std::string key = normalize(str);
      auto colorMap = colors.find(key);
      
      if(colorMap == colors.end()) {
        
        if(!createColor) {
          fprintf(stderr, "error not found glColor\n");
          abort();
        }
        
        colors[key] = getRandom();
        
        return colors[key];
        
      } else { return colorMap->second; }
      
      abort();
      
    }
    
    //****************************************************************************//
    // heatMap
    //****************************************************************************//
    static const glm::vec4 heatMap(double value) {
            
      double color1[3] = {1.0, 1.0, 1.0}; // white
      double color2[3] = {1.0, 0.0, 0.0}; // red
      
      double R = color1[0] + value * (color2[0] - color1[0]);
      double G = color1[1] + value * (color2[1] - color1[1]);
      double B = color1[2] + value * (color2[2] - color1[2]);
      
      return glm::vec4(R, G, B, 1.0);
      
    }
    
  private:
    
    //****************************************************************************//
    // init
    //****************************************************************************//
    static void init(){

      auto add = [](const std::string & name, int r, int g, int b) {
        colors[name] = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
      };

      add("aliceblue", 240, 248, 255);
      add("antiquewhite", 250, 235, 215);
      add("aqua", 0, 255, 255);
      add("aquamarine", 127, 255, 212);
      add("azure", 240, 255, 255);
      add("beige", 245, 245, 220);
      add("bisque", 255, 228, 196);
      add("black", 0, 0, 0);
      add("blanchedalmond", 255, 235, 205);
      add("blue", 0, 0, 255);
      add("blueviolet", 138, 43, 226);
      add("brown", 165, 42, 42);
      add("burlywood", 222, 184, 135);
      add("cadetblue", 95, 158, 160);
      add("chartreuse", 127, 255, 0);
      add("chocolate", 210, 105, 30);
      add("coral", 255, 127, 80);
      add("cornflowerblue", 100, 149, 237);
      add("cornsilk", 255, 248, 220);
      add("crimson", 220, 20, 60);
      add("cyan", 0, 255, 255);
      add("darkblue", 0, 0, 139);
      add("darkcyan", 0, 139, 139);
      add("darkgoldenrod", 184, 134, 11);
      add("darkgray", 169, 169, 169);
      add("darkgreen", 0, 100, 0);
      add("darkgrey", 169, 169, 169);
      add("darkkhaki", 189, 183, 107);
      add("darkmagenta", 139, 0, 139);
      add("darkolivegreen", 85, 107, 47);
      add("darkorange", 255, 140, 0);
      add("darkorchid", 153, 50, 204);
      add("darkred", 139, 0, 0);
      add("darksalmon", 233, 150, 122);
      add("darkseagreen", 143, 188, 143);
      add("darkslateblue", 72, 61, 139);
      add("darkslategray", 47, 79, 79);
      add("darkslategrey", 47, 79, 79);
      add("darkturquoise", 0, 206, 209);
      add("darkviolet", 148, 0, 211);
      add("deeppink", 255, 20, 147);
      add("deepskyblue", 0, 191, 255);
      add("dimgray", 105, 105, 105);
      add("dimgrey", 105, 105, 105);
      add("dodgerblue", 30, 144, 255);
      add("firebrick", 178, 34, 34);
      add("floralwhite", 255, 250, 240);
      add("forestgreen", 34, 139, 34);
      add("fuchsia", 255, 0, 255);
      add("gainsboro", 220, 220, 220);
      add("ghostwhite", 248, 248, 255);
      add("gold", 255, 215, 0);
      add("goldenrod", 218, 165, 32);
      add("gray", 128, 128, 128);
      add("green", 0, 128, 0);
      add("greenyellow", 173, 255, 47);
      add("grey", 128, 128, 128);
      add("honeydew", 240, 255, 240);
      add("hotpink", 255, 105, 180);
      add("indianred", 205, 92, 92);
      add("indigo", 75, 0, 130);
      add("ivory", 255, 255, 240);
      add("khaki", 240, 230, 140);
      add("lavender", 230, 230, 250);
      add("lavenderblush", 255, 240, 245);
      add("lawngreen", 124, 252, 0);
      add("lemonchiffon", 255, 250, 205);
      add("lightblue", 173, 216, 230);
      add("lightcoral", 240, 128, 128);
      add("lightcyan", 224, 255, 255);
      add("lightgoldenrodyellow", 250, 250, 210);
      add("lightgray", 211, 211, 211);
      add("lightgreen", 144, 238, 144);
      add("lightgrey", 211, 211, 211);
      add("lightpink", 255, 182, 193);
      add("lightsalmon", 255, 160, 122);
      add("lightseagreen", 32, 178, 170);
      add("lightskyblue", 135, 206, 250);
      add("lightslategray", 119, 136, 153);
      add("lightslategrey", 119, 136, 153);
      add("lightsteelblue", 176, 196, 222);
      add("lightyellow", 255, 255, 224);
      add("lime", 0, 255, 0);
      add("limegreen", 50, 205, 50);
      add("linen", 250, 240, 230);
      add("magenta", 255, 0, 255);
      add("maroon", 128, 0, 0);
      add("mediumaquamarine", 102, 205, 170);
      add("mediumblue", 0, 0, 205);
      add("mediumorchid", 186, 85, 211);
      add("mediumpurple", 147, 112, 219);
      add("mediumseagreen", 60, 179, 113);
      add("mediumslateblue", 123, 104, 238);
      add("mediumspringgreen", 0, 250, 154);
      add("mediumturquoise", 72, 209, 204);
      add("mediumvioletred", 199, 21, 133);
      add("midnightblue", 25, 25, 112);
      add("mintcream", 245, 255, 250);
      add("mistyrose", 255, 228, 225);
      add("moccasin", 255, 228, 181);
      add("navajowhite", 255, 222, 173);
      add("navy", 0, 0, 128);
      add("oldlace", 253, 245, 230);
      add("olive", 128, 128, 0);
      add("olivedrab", 107, 142, 35);
      add("orange", 255, 165, 0);
      add("orangered", 255, 69, 0);
      add("orchid", 218, 112, 214);
      add("palegoldenrod", 238, 232, 170);
      add("palegreen", 152, 251, 152);
      add("paleturquoise", 175, 238, 238);
      add("palevioletred", 219, 112, 147);
      add("papayawhip", 255, 239, 213);
      add("peachpuff", 255, 218, 185);
      add("peru", 205, 133, 63);
      add("pink", 255, 192, 203);
      add("plum", 221, 160, 221);
      add("powderblue", 176, 224, 230);
      add("purple", 128, 0, 128);
      add("red", 255, 0, 0);
      add("rosybrown", 188, 143, 143);
      add("royalblue", 65, 105, 225);
      add("saddlebrown", 139, 69, 19);
      add("salmon", 250, 128, 114);
      add("sandybrown", 244, 164, 96);
      add("seagreen", 46, 139, 87);
      add("seashell", 255, 245, 238);
      add("sienna", 160, 82, 45);
      add("silver", 192, 192, 192);
      add("skyblue", 135, 206, 235);
      add("slateblue", 106, 90, 205);
      add("slategray", 112, 128, 144);
      add("slategrey", 112, 128, 144);
      add("snow", 255, 250, 250);
      add("springgreen", 0, 255, 127);
      add("steelblue", 70, 130, 180);
      add("tan", 210, 180, 140);
      add("teal", 0, 128, 128);
      add("thistle", 216, 191, 216);
      add("tomato", 255, 99, 71);
      add("turquoise", 64, 224, 208);
      add("violet", 238, 130, 238);
      add("wheat", 245, 222, 179);
      add("white", 255, 255, 255);
      add("whitesmoke", 245, 245, 245);
      add("yellow", 255, 255, 0);
      add("yellowgreen", 154, 205, 50);

      add("dark-gray", 169, 169, 169);
      add("dark-grey", 169, 169, 169);
      add("light-gray", 211, 211, 211);
      add("light-grey", 211, 211, 211);
      add("light-blue", 173, 216, 230);
      add("light-green", 144, 238, 144);
      add("light-cyan", 224, 255, 255);
      add("light-pink", 255, 182, 193);
      add("light-salmon", 255, 160, 122);
      add("dark-magenta", 139, 0, 139);
      add("dark-cyan", 0, 139, 139);
      add("dark-orange", 255, 140, 0);
      add("dark-red", 139, 0, 0);
      add("dark-green", 0, 100, 0);
      add("dark-blue", 0, 0, 139);
      add("dark-turquoise", 0, 206, 209);
      add("dark-violet", 148, 0, 211);
      add("dark-khaki", 189, 183, 107);
      add("dark-goldenrod", 184, 134, 11);
      add("dark-salmon", 233, 150, 122);
      add("dark-olivegreen", 85, 107, 47);
      add("midnight-blue", 25, 25, 112);
      add("medium-blue", 0, 0, 205);
      add("spring-green", 0, 255, 127);
      add("forest-green", 34, 139, 34);
      add("sea-green", 46, 139, 87);
      add("orange-red", 255, 69, 0);

      add("web-green", 0, 192, 0);
      add("web-blue", 0, 128, 255);
      add("dark-yellow", 200, 200, 0);
      add("dark-spring-green", 0, 128, 64);
      add("dark-chartreuse", 64, 128, 0);
      add("light-red", 240, 50, 50);
      add("light-magenta", 240, 85, 240);
      add("light-goldenrod", 238, 221, 130);
      add("light-turquoise", 175, 238, 238);
      add("dark-pink", 255, 20, 147);
      add("dark-plum", 221, 80, 64);
      add("orangered4", 128, 20, 0);
      add("brown4", 128, 20, 20);
      add("sienna4", 128, 64, 20);
      add("orchid4", 128, 64, 128);
      add("mediumpurple3", 128, 96, 192);
      add("slateblue1", 128, 96, 255);
      add("yellow4", 128, 128, 0);
      add("sienna1", 255, 128, 64);
      add("tan1", 255, 160, 64);
      add("khaki1", 255, 255, 128);
      add("grey0", 0, 0, 0);
      add("grey10", 26, 26, 26);
      add("grey20", 51, 51, 51);
      add("grey30", 77, 77, 77);
      add("grey40", 102, 102, 102);
      add("grey50", 127, 127, 127);
      add("grey60", 153, 153, 153);
      add("grey70", 179, 179, 179);
      add("grey80", 204, 204, 204);
      add("grey90", 229, 229, 229);
      add("grey100", 255, 255, 255);
      add("gray0", 0, 0, 0);
      add("gray10", 26, 26, 26);
      add("gray20", 51, 51, 51);
      add("gray30", 77, 77, 77);
      add("gray40", 102, 102, 102);
      add("gray50", 127, 127, 127);
      add("gray60", 153, 153, 153);
      add("gray70", 179, 179, 179);
      add("gray80", 204, 204, 204);
      add("gray90", 229, 229, 229);
      add("gray100", 255, 255, 255);
      
      isInited = true;

    }
    
  }; /* class glColors */

  std::map<std::string, glm::vec4> glColors::colors = std::map<std::string, glm::vec4>();
  
  bool glColors::isInited = false;

} /* namespace ogl */

#endif /* _H_OGL_OBJECT_H_ */
