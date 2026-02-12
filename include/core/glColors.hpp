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
    
    // mappa che mi contiene i colori
    static std::map<std::string, glm::vec4> colors;
    
    //****************************************************************************//
    // glColors()
    //****************************************************************************//
    glColors() {}
        
  public:
    
    //****************************************************************************//
    // loadColors()
    //****************************************************************************//
    // https://htmlcolorcodes.com/color-names/
    static void loadPalette() {
      
      colors["aliceblue"] = aliceblue;
      colors["antiquewhite"] = antiquewhite;
      colors["aqua"] = aqua;
      colors["aquamarine"] = aquamarine;
      colors["azure"] = azure;
      colors["beige"] = beige;
      colors["bisque"] = bisque;
      colors["black"] = black;
      colors["blanchedalmond"] = blanchedalmond;
      colors["blue"] = blue;
      colors["blueviolet"] = blueviolet;
      colors["brown"] = brown;
      colors["burlywood"] = burlywood;
      colors["cadetblue"] = cadetblue;
      colors["chartreuse"] = chartreuse;
      colors["chocolate"] = chocolate;
      colors["coral"] = coral;
      colors["cornflowerblue"] = cornflowerblue;
      colors["cornsilk"] = cornsilk;
      colors["crimson"] = crimson;
      colors["cyan"] = cyan;
      colors["darkblue"] = darkblue;
      colors["darkcyan"] = darkcyan;
      colors["darkgoldenrod"] = darkgoldenrod;
      colors["darkgray"] = darkgray;
      colors["darkgreen"] = darkgreen;
      colors["darkgrey"] = darkgrey;
      colors["darkkhaki"] = darkkhaki;
      colors["darkmagenta"] = darkmagenta;
      colors["darkolivegreen"] = darkolivegreen;
      colors["darkorange"] = darkorange;
      colors["darkorchid"] = darkorchid;
      colors["darkred"] = darkred;
      colors["darksalmon"] = darksalmon;
      colors["darkseagreen"] = darkseagreen;
      colors["darkslateblue"] = darkslateblue;
      colors["darkslategray"] = darkslategray;
      colors["darkslategrey"] = darkslategrey;
      colors["darkturquoise"] = darkturquoise;
      colors["darkviolet"] = darkviolet;
      colors["deeppink"] = deeppink;
      colors["deepskyblue"] = deepskyblue;
      colors["dimgray"] = dimgray;
      colors["dimgrey"] = dimgrey;
      colors["dodgerblue"] = dodgerblue;
      colors["firebrick"] = firebrick;
      colors["floralwhite"] = floralwhite;
      colors["forestgreen"] = forestgreen;
      colors["fuchsia"] = fuchsia;
      colors["gainsboro"] = gainsboro;
      colors["ghostwhite"] = ghostwhite;
      colors["gold"] = gold;
      colors["goldenrod"] = goldenrod;
      colors["gray"] = gray;
      colors["green"] = green;
      colors["greenyellow"] = greenyellow;
      colors["grey"] = grey;
      colors["honeydew"] = honeydew;
      colors["hotpink"] = hotpink;
      colors["indianred"] = indianred;
      colors["indigo"] = indigo;
      colors["ivory"] = ivory;
      colors["khaki"] = khaki;
      colors["lavender"] = lavender;
      colors["lavenderblush"] = lavenderblush;
      colors["lawngreen"] = lawngreen;
      colors["lemonchiffon"] = lemonchiffon;
      colors["lightblue"] = lightblue;
      colors["lightcoral"] = lightcoral;
      colors["lightcyan"] = lightcyan;
      colors["lightgoldenrodyellow"] = lightgoldenrodyellow;
      colors["lightgray"] = lightgray;
      colors["lightgreen"] = lightgreen;
      colors["lightgrey"] = lightgrey;
      colors["lightpink"] = lightpink;
      colors["lightsalmon"] = lightsalmon;
      colors["lightseagreen"] = lightseagreen;
      colors["lightskyblue"] = lightskyblue;
      colors["lightslategray"] = lightslategray;
      colors["lightslategrey"] = lightslategrey;
      colors["lightsteelblue"] = lightsteelblue;
      colors["lightyellow"] = lightyellow;
      colors["lime"] = lime;
      colors["limegreen"] = limegreen;
      colors["linen"] = linen;
      colors["magenta"] = magenta;
      colors["maroon"] = maroon;
      colors["mediumaquamarine"] = mediumaquamarine;
      colors["mediumblue"] = mediumblue;
      colors["mediumorchid"] = mediumorchid;
      colors["mediumpurple"] = mediumpurple;
      colors["mediumseagreen"] = mediumseagreen;
      colors["mediumslateblue"] = mediumslateblue;
      colors["mediumspringgreen"] = mediumspringgreen;
      colors["mediumturquoise"] = mediumturquoise;
      colors["mediumvioletred"] = mediumvioletred;
      colors["midnightblue"] = midnightblue;
      colors["mintcream"] = mintcream;
      colors["mistyrose"] = mistyrose;
      colors["moccasin"] = moccasin;
      colors["navajowhite"] = navajowhite;
      colors["navy"] = navy;
      colors["oldlace"] = oldlace;
      colors["olive"] = olive;
      colors["olivedrab"] = olivedrab;
      colors["orange"] = orange;
      colors["orangered"] = orangered;
      colors["orchid"] = orchid;
      colors["palegoldenrod"] = palegoldenrod;
      colors["palegreen"] = palegreen;
      colors["paleturquoise"] = paleturquoise;
      colors["palevioletred"] = palevioletred;
      colors["papayawhip"] = papayawhip;
      colors["peachpuff"] = peachpuff;
      colors["peru"] = peru;
      colors["pink"] = pink;
      colors["plum"] = plum;
      colors["powderblue"] = powderblue;
      colors["purple"] = purple;
      colors["red"] = red;
      colors["rosybrown"] = rosybrown;
      colors["royalblue"] = royalblue;
      colors["saddlebrown"] = saddlebrown;
      colors["salmon"] = salmon;
      colors["sandybrown"] = sandybrown;
      colors["seagreen"] = seagreen;
      colors["seashell"] = seashell;
      colors["sienna"] = sienna;
      colors["silver"] = silver;
      colors["skyblue"] = skyblue;
      colors["slateblue"] = slateblue;
      colors["slategray"] = slategray;
      colors["slategrey"] = slategrey;
      colors["snow"] = snow;
      colors["springgreen"] = springgreen;
      colors["steelblue"] = steelblue;
      colors["tan"] = tan;
      colors["teal"] = teal;
      colors["thistle"] = thistle;
      colors["tomato"] = tomato;
      colors["turquoise"] = turquoise;
      colors["violet"] = violet;
      colors["wheat"] = wheat;
      colors["white"] = white;
      colors["whitesmoke"] = whitesmoke;
      colors["yellow"] = yellow;
      colors["yellowgreen"] = yellowgreen;
      
    }
    
    //****************************************************************************//
    // getRandom()
    //****************************************************************************//
    static const glm::vec4 getRandom() {
      
      static std::default_random_engine generator;
      static std::uniform_real_distribution<float> distribution(0.0,1.0);
      
      return glm::vec4(distribution(generator), distribution(generator), distribution(generator), 1);
      
    }
    
    //****************************************************************************//
    // get()
    //****************************************************************************//
    static const glm::vec4 & get(const std::string & str, bool create = false) {
   
      auto colorMap = colors.find(normalize(str));
      
      if(colorMap == colors.end()) {
        
        if(!create) {
          fprintf(stderr, "error: color \"%s\" not found ogl::glColor\n", str.c_str());
          abort();
        }
        
        add(str, getRandom());
        
        return colors[str];
        
      }
      
      return colorMap->second;
            
    }
    
    //****************************************************************************//
    // heatMap()
    //****************************************************************************//
    static const glm::vec4 heatMap(double value) {
            
      double color1[3] = {1.0, 1.0, 1.0}; // white
      double color2[3] = {1.0, 0.0, 0.0}; // red
      
      double R = color1[0] + value * (color2[0] - color1[0]);
      double G = color1[1] + value * (color2[1] - color1[1]);
      double B = color1[2] + value * (color2[2] - color1[2]);
      
      return glm::vec4(R, G, B, 1.0);
      
    }
    
    //****************************************************************************//
    // add()
    //****************************************************************************//
    static void add(const std::string & name, int r, int g, int b) {
      colors[name] = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    }
    
    static void add(const std::string & name, const glm::vec4 & color) {
      colors[name] = color;
    }

    
  private:
    
    //****************************************************************************//
    // normalize()
    //****************************************************************************//
    static std::string normalize(const std::string & value) {
      
      std::string lowered;
      
      lowered.reserve(value.size());
      
      for (unsigned char ch : value) {
        lowered.push_back(static_cast<char>(std::tolower(ch)));
      }
      
      return lowered;
      
    }
    
  public:
    
    // https://htmlcolorcodes.com/color-names/
    
    constexpr static const glm::vec4 aliceblue = glm::vec4(0.941f, 0.973f, 1.000f, 1.000f);
    constexpr static const glm::vec4 antiquewhite = glm::vec4(0.980f, 0.922f, 0.843f, 1.000f);
    constexpr static const glm::vec4 aqua = glm::vec4(0.000f, 1.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 aquamarine = glm::vec4(0.498f, 1.000f, 0.831f, 1.000f);
    constexpr static const glm::vec4 azure = glm::vec4(0.941f, 1.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 beige = glm::vec4(0.961f, 0.961f, 0.863f, 1.000f);
    constexpr static const glm::vec4 bisque = glm::vec4(1.000f, 0.894f, 0.769f, 1.000f);
    constexpr static const glm::vec4 black = glm::vec4(0.000f, 0.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 blanchedalmond = glm::vec4(1.000f, 0.922f, 0.804f, 1.000f);
    constexpr static const glm::vec4 blue = glm::vec4(0.000f, 0.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 blueviolet = glm::vec4(0.541f, 0.169f, 0.886f, 1.000f);
    constexpr static const glm::vec4 brown = glm::vec4(0.647f, 0.165f, 0.165f, 1.000f);
    constexpr static const glm::vec4 burlywood = glm::vec4(0.871f, 0.722f, 0.529f, 1.000f);
    constexpr static const glm::vec4 cadetblue = glm::vec4(0.373f, 0.620f, 0.627f, 1.000f);
    constexpr static const glm::vec4 chartreuse = glm::vec4(0.498f, 1.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 chocolate = glm::vec4(0.824f, 0.412f, 0.118f, 1.000f);
    constexpr static const glm::vec4 coral = glm::vec4(1.000f, 0.498f, 0.314f, 1.000f);
    constexpr static const glm::vec4 cornflowerblue = glm::vec4(0.392f, 0.584f, 0.929f, 1.000f);
    constexpr static const glm::vec4 cornsilk = glm::vec4(1.000f, 0.973f, 0.863f, 1.000f);
    constexpr static const glm::vec4 crimson = glm::vec4(0.863f, 0.078f, 0.235f, 1.000f);
    constexpr static const glm::vec4 cyan = glm::vec4(0.000f, 1.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 darkblue = glm::vec4(0.000f, 0.000f, 0.545f, 1.000f);
    constexpr static const glm::vec4 darkcyan = glm::vec4(0.000f, 0.545f, 0.545f, 1.000f);
    constexpr static const glm::vec4 darkgoldenrod = glm::vec4(0.722f, 0.525f, 0.043f, 1.000f);
    constexpr static const glm::vec4 darkgray = glm::vec4(0.663f, 0.663f, 0.663f, 1.000f);
    constexpr static const glm::vec4 darkgreen = glm::vec4(0.000f, 0.392f, 0.000f, 1.000f);
    constexpr static const glm::vec4 darkgrey = glm::vec4(0.663f, 0.663f, 0.663f, 1.000f);
    constexpr static const glm::vec4 darkkhaki = glm::vec4(0.741f, 0.718f, 0.420f, 1.000f);
    constexpr static const glm::vec4 darkmagenta = glm::vec4(0.545f, 0.000f, 0.545f, 1.000f);
    constexpr static const glm::vec4 darkolivegreen = glm::vec4(0.333f, 0.420f, 0.184f, 1.000f);
    constexpr static const glm::vec4 darkorange = glm::vec4(1.000f, 0.549f, 0.000f, 1.000f);
    constexpr static const glm::vec4 darkorchid = glm::vec4(0.600f, 0.196f, 0.800f, 1.000f);
    constexpr static const glm::vec4 darkred = glm::vec4(0.545f, 0.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 darksalmon = glm::vec4(0.914f, 0.588f, 0.478f, 1.000f);
    constexpr static const glm::vec4 darkseagreen = glm::vec4(0.561f, 0.737f, 0.561f, 1.000f);
    constexpr static const glm::vec4 darkslateblue = glm::vec4(0.282f, 0.239f, 0.545f, 1.000f);
    constexpr static const glm::vec4 darkslategray = glm::vec4(0.184f, 0.310f, 0.310f, 1.000f);
    constexpr static const glm::vec4 darkslategrey = glm::vec4(0.184f, 0.310f, 0.310f, 1.000f);
    constexpr static const glm::vec4 darkturquoise = glm::vec4(0.000f, 0.808f, 0.820f, 1.000f);
    constexpr static const glm::vec4 darkviolet = glm::vec4(0.580f, 0.000f, 0.827f, 1.000f);
    constexpr static const glm::vec4 deeppink = glm::vec4(1.000f, 0.078f, 0.576f, 1.000f);
    constexpr static const glm::vec4 deepskyblue = glm::vec4(0.000f, 0.749f, 1.000f, 1.000f);
    constexpr static const glm::vec4 dimgray = glm::vec4(0.412f, 0.412f, 0.412f, 1.000f);
    constexpr static const glm::vec4 dimgrey = glm::vec4(0.412f, 0.412f, 0.412f, 1.000f);
    constexpr static const glm::vec4 dodgerblue = glm::vec4(0.118f, 0.565f, 1.000f, 1.000f);
    constexpr static const glm::vec4 firebrick = glm::vec4(0.698f, 0.133f, 0.133f, 1.000f);
    constexpr static const glm::vec4 floralwhite = glm::vec4(1.000f, 0.980f, 0.941f, 1.000f);
    constexpr static const glm::vec4 forestgreen = glm::vec4(0.133f, 0.545f, 0.133f, 1.000f);
    constexpr static const glm::vec4 fuchsia = glm::vec4(1.000f, 0.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 gainsboro = glm::vec4(0.863f, 0.863f, 0.863f, 1.000f);
    constexpr static const glm::vec4 ghostwhite = glm::vec4(0.973f, 0.973f, 1.000f, 1.000f);
    constexpr static const glm::vec4 gold = glm::vec4(1.000f, 0.843f, 0.000f, 1.000f);
    constexpr static const glm::vec4 goldenrod = glm::vec4(0.855f, 0.647f, 0.125f, 1.000f);
    constexpr static const glm::vec4 gray = glm::vec4(0.502f, 0.502f, 0.502f, 1.000f);
    constexpr static const glm::vec4 green = glm::vec4(0.000f, 0.502f, 0.000f, 1.000f);
    constexpr static const glm::vec4 greenyellow = glm::vec4(0.678f, 1.000f, 0.184f, 1.000f);
    constexpr static const glm::vec4 grey = glm::vec4(0.502f, 0.502f, 0.502f, 1.000f);
    constexpr static const glm::vec4 honeydew = glm::vec4(0.941f, 1.000f, 0.941f, 1.000f);
    constexpr static const glm::vec4 hotpink = glm::vec4(1.000f, 0.412f, 0.706f, 1.000f);
    constexpr static const glm::vec4 indianred = glm::vec4(0.804f, 0.361f, 0.361f, 1.000f);
    constexpr static const glm::vec4 indigo = glm::vec4(0.294f, 0.000f, 0.510f, 1.000f);
    constexpr static const glm::vec4 ivory = glm::vec4(1.000f, 1.000f, 0.941f, 1.000f);
    constexpr static const glm::vec4 khaki = glm::vec4(0.941f, 0.902f, 0.549f, 1.000f);
    constexpr static const glm::vec4 lavender = glm::vec4(0.902f, 0.902f, 0.980f, 1.000f);
    constexpr static const glm::vec4 lavenderblush = glm::vec4(1.000f, 0.941f, 0.961f, 1.000f);
    constexpr static const glm::vec4 lawngreen = glm::vec4(0.486f, 0.988f, 0.000f, 1.000f);
    constexpr static const glm::vec4 lemonchiffon = glm::vec4(1.000f, 0.980f, 0.804f, 1.000f);
    constexpr static const glm::vec4 lightblue = glm::vec4(0.678f, 0.847f, 0.902f, 1.000f);
    constexpr static const glm::vec4 lightcoral = glm::vec4(0.941f, 0.502f, 0.502f, 1.000f);
    constexpr static const glm::vec4 lightcyan = glm::vec4(0.878f, 1.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 lightgoldenrodyellow = glm::vec4(0.980f, 0.980f, 0.824f, 1.000f);
    constexpr static const glm::vec4 lightgray = glm::vec4(0.827f, 0.827f, 0.827f, 1.000f);
    constexpr static const glm::vec4 lightgreen = glm::vec4(0.565f, 0.933f, 0.565f, 1.000f);
    constexpr static const glm::vec4 lightgrey = glm::vec4(0.827f, 0.827f, 0.827f, 1.000f);
    constexpr static const glm::vec4 lightpink = glm::vec4(1.000f, 0.714f, 0.757f, 1.000f);
    constexpr static const glm::vec4 lightsalmon = glm::vec4(1.000f, 0.627f, 0.478f, 1.000f);
    constexpr static const glm::vec4 lightseagreen = glm::vec4(0.125f, 0.698f, 0.667f, 1.000f);
    constexpr static const glm::vec4 lightskyblue = glm::vec4(0.529f, 0.808f, 0.980f, 1.000f);
    constexpr static const glm::vec4 lightslategray = glm::vec4(0.467f, 0.533f, 0.600f, 1.000f);
    constexpr static const glm::vec4 lightslategrey = glm::vec4(0.467f, 0.533f, 0.600f, 1.000f);
    constexpr static const glm::vec4 lightsteelblue = glm::vec4(0.690f, 0.769f, 0.871f, 1.000f);
    constexpr static const glm::vec4 lightyellow = glm::vec4(1.000f, 1.000f, 0.878f, 1.000f);
    constexpr static const glm::vec4 lime = glm::vec4(0.000f, 1.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 limegreen = glm::vec4(0.196f, 0.804f, 0.196f, 1.000f);
    constexpr static const glm::vec4 linen = glm::vec4(0.980f, 0.941f, 0.902f, 1.000f);
    constexpr static const glm::vec4 magenta = glm::vec4(1.000f, 0.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 maroon = glm::vec4(0.502f, 0.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 mediumaquamarine = glm::vec4(0.400f, 0.804f, 0.667f, 1.000f);
    constexpr static const glm::vec4 mediumblue = glm::vec4(0.000f, 0.000f, 0.804f, 1.000f);
    constexpr static const glm::vec4 mediumorchid = glm::vec4(0.729f, 0.333f, 0.827f, 1.000f);
    constexpr static const glm::vec4 mediumpurple = glm::vec4(0.576f, 0.439f, 0.859f, 1.000f);
    constexpr static const glm::vec4 mediumseagreen = glm::vec4(0.235f, 0.702f, 0.443f, 1.000f);
    constexpr static const glm::vec4 mediumslateblue = glm::vec4(0.482f, 0.408f, 0.933f, 1.000f);
    constexpr static const glm::vec4 mediumspringgreen = glm::vec4(0.000f, 0.980f, 0.604f, 1.000f);
    constexpr static const glm::vec4 mediumturquoise = glm::vec4(0.282f, 0.820f, 0.800f, 1.000f);
    constexpr static const glm::vec4 mediumvioletred = glm::vec4(0.780f, 0.082f, 0.522f, 1.000f);
    constexpr static const glm::vec4 midnightblue = glm::vec4(0.098f, 0.098f, 0.439f, 1.000f);
    constexpr static const glm::vec4 mintcream = glm::vec4(0.961f, 1.000f, 0.980f, 1.000f);
    constexpr static const glm::vec4 mistyrose = glm::vec4(1.000f, 0.894f, 0.882f, 1.000f);
    constexpr static const glm::vec4 moccasin = glm::vec4(1.000f, 0.894f, 0.710f, 1.000f);
    constexpr static const glm::vec4 navajowhite = glm::vec4(1.000f, 0.871f, 0.678f, 1.000f);
    constexpr static const glm::vec4 navy = glm::vec4(0.000f, 0.000f, 0.502f, 1.000f);
    constexpr static const glm::vec4 oldlace = glm::vec4(0.992f, 0.961f, 0.902f, 1.000f);
    constexpr static const glm::vec4 olive = glm::vec4(0.502f, 0.502f, 0.000f, 1.000f);
    constexpr static const glm::vec4 olivedrab = glm::vec4(0.420f, 0.557f, 0.137f, 1.000f);
    constexpr static const glm::vec4 orange = glm::vec4(1.000f, 0.647f, 0.000f, 1.000f);
    constexpr static const glm::vec4 orangered = glm::vec4(1.000f, 0.271f, 0.000f, 1.000f);
    constexpr static const glm::vec4 orchid = glm::vec4(0.855f, 0.439f, 0.839f, 1.000f);
    constexpr static const glm::vec4 palegoldenrod = glm::vec4(0.933f, 0.910f, 0.667f, 1.000f);
    constexpr static const glm::vec4 palegreen = glm::vec4(0.596f, 0.984f, 0.596f, 1.000f);
    constexpr static const glm::vec4 paleturquoise = glm::vec4(0.686f, 0.933f, 0.933f, 1.000f);
    constexpr static const glm::vec4 palevioletred = glm::vec4(0.859f, 0.439f, 0.576f, 1.000f);
    constexpr static const glm::vec4 papayawhip = glm::vec4(1.000f, 0.937f, 0.835f, 1.000f);
    constexpr static const glm::vec4 peachpuff = glm::vec4(1.000f, 0.855f, 0.725f, 1.000f);
    constexpr static const glm::vec4 peru = glm::vec4(0.804f, 0.522f, 0.247f, 1.000f);
    constexpr static const glm::vec4 pink = glm::vec4(1.000f, 0.753f, 0.796f, 1.000f);
    constexpr static const glm::vec4 plum = glm::vec4(0.867f, 0.627f, 0.867f, 1.000f);
    constexpr static const glm::vec4 powderblue = glm::vec4(0.690f, 0.878f, 0.902f, 1.000f);
    constexpr static const glm::vec4 purple = glm::vec4(0.502f, 0.000f, 0.502f, 1.000f);
    constexpr static const glm::vec4 red = glm::vec4(1.000f, 0.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 rosybrown = glm::vec4(0.737f, 0.561f, 0.561f, 1.000f);
    constexpr static const glm::vec4 royalblue = glm::vec4(0.255f, 0.412f, 0.882f, 1.000f);
    constexpr static const glm::vec4 saddlebrown = glm::vec4(0.545f, 0.271f, 0.075f, 1.000f);
    constexpr static const glm::vec4 salmon = glm::vec4(0.980f, 0.502f, 0.447f, 1.000f);
    constexpr static const glm::vec4 sandybrown = glm::vec4(0.957f, 0.643f, 0.376f, 1.000f);
    constexpr static const glm::vec4 seagreen = glm::vec4(0.180f, 0.545f, 0.341f, 1.000f);
    constexpr static const glm::vec4 seashell = glm::vec4(1.000f, 0.961f, 0.933f, 1.000f);
    constexpr static const glm::vec4 sienna = glm::vec4(0.627f, 0.322f, 0.176f, 1.000f);
    constexpr static const glm::vec4 silver = glm::vec4(0.753f, 0.753f, 0.753f, 1.000f);
    constexpr static const glm::vec4 skyblue = glm::vec4(0.529f, 0.808f, 0.922f, 1.000f);
    constexpr static const glm::vec4 slateblue = glm::vec4(0.416f, 0.353f, 0.804f, 1.000f);
    constexpr static const glm::vec4 slategray = glm::vec4(0.439f, 0.502f, 0.565f, 1.000f);
    constexpr static const glm::vec4 slategrey = glm::vec4(0.439f, 0.502f, 0.565f, 1.000f);
    constexpr static const glm::vec4 snow = glm::vec4(1.000f, 0.980f, 0.980f, 1.000f);
    constexpr static const glm::vec4 springgreen = glm::vec4(0.000f, 1.000f, 0.498f, 1.000f);
    constexpr static const glm::vec4 steelblue = glm::vec4(0.275f, 0.510f, 0.706f, 1.000f);
    constexpr static const glm::vec4 tan = glm::vec4(0.824f, 0.706f, 0.549f, 1.000f);
    constexpr static const glm::vec4 teal = glm::vec4(0.000f, 0.502f, 0.502f, 1.000f);
    constexpr static const glm::vec4 thistle = glm::vec4(0.847f, 0.749f, 0.847f, 1.000f);
    constexpr static const glm::vec4 tomato = glm::vec4(1.000f, 0.388f, 0.278f, 1.000f);
    constexpr static const glm::vec4 turquoise = glm::vec4(0.251f, 0.878f, 0.816f, 1.000f);
    constexpr static const glm::vec4 violet = glm::vec4(0.933f, 0.510f, 0.933f, 1.000f);
    constexpr static const glm::vec4 wheat = glm::vec4(0.961f, 0.871f, 0.702f, 1.000f);
    constexpr static const glm::vec4 white = glm::vec4(1.000f, 1.000f, 1.000f, 1.000f);
    constexpr static const glm::vec4 whitesmoke = glm::vec4(0.961f, 0.961f, 0.961f, 1.000f);
    constexpr static const glm::vec4 yellow = glm::vec4(1.000f, 1.000f, 0.000f, 1.000f);
    constexpr static const glm::vec4 yellowgreen = glm::vec4(0.604f, 0.804f, 0.196f, 1.000f);
    
  }; /* class glColors */

  std::map<std::string, glm::vec4> glColors::colors = std::map<std::string, glm::vec4>();
  
} /* namespace ogl */

#endif /* _H_OGL_OBJECT_H_ */
