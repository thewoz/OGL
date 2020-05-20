/*
 * MIT License
 *
 * Copyright © 2017
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
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

#ifndef _H_OGL_COLORS_H_
#define _H_OGL_COLORS_H_

#include <cstdio>
#include <cstdlib>

#include <map>
#include <string>
#include <random>

#include <opencv2/opencv.hpp>

/* ****************************************************************************/
// namespace ogl
/* ****************************************************************************/
namespace ogl {

  /* ****************************************************************************/
  // Class glColors
  /* ****************************************************************************/
  class glColors {
    
  private:
    
    glColors() {}
    
    static std::map<std::string, glm::vec4> colors;

    static bool isInited;
    
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
    static const glm::vec4 & get(const std::string & str) {
         
      if(isInited) init();
      
      std::map<std::string, glm::vec4>::iterator color;
      
      if((color = colors.find(str)) == colors.end()) {
        fprintf(stderr, "error not found glColor\n");
        abort();
      }
      
      return color->second;
      
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

      colors["white"]             = glm::vec4(1.000000, 1.000000, 1.000000, 1.000000);
      colors["black"]             = glm::vec4(0.000000, 0.000000, 0.000000, 1.000000);
      colors["dark-grey"]         = glm::vec4(0.627451, 0.627451, 0.627451, 1.000000);
      colors["red"]               = glm::vec4(1.000000, 0.000000, 0.000000, 1.000000);
      colors["web-green"]         = glm::vec4(0.000000, 0.752941, 0.000000, 1.000000);
      colors["web-blue"]          = glm::vec4(0.000000, 0.501961, 1.000000, 1.000000);
      colors["dark-magenta"]      = glm::vec4(0.752941, 0.000000, 1.000000, 1.000000);
      colors["dark-cyan"]         = glm::vec4(0.000000, 0.933333, 0.933333, 1.000000);
      colors["dark-orange"]       = glm::vec4(0.752941, 0.250980, 0.000000, 1.000000);
      colors["dark-yellow"]       = glm::vec4(0.784314, 0.784314, 0.000000, 1.000000);
      colors["royalblue"]         = glm::vec4(0.254902, 0.411765, 0.882353, 1.000000);
      colors["goldenrod"]         = glm::vec4(1.000000, 0.752941, 0.125490, 1.000000);
      colors["dark-spring-green"] = glm::vec4(0.000000, 0.501961, 0.250980, 1.000000);
      colors["purple"]            = glm::vec4(0.752941, 0.501961, 1.000000, 1.000000);
      colors["steelblue"]         = glm::vec4(0.188235, 0.376471, 0.501961, 1.000000);
      colors["dark-red"]          = glm::vec4(0.545098, 0.000000, 0.000000, 1.000000);
      colors["dark-chartreuse"]   = glm::vec4(0.250980, 0.501961, 0.000000, 1.000000);
      colors["orchid"]            = glm::vec4(1.000000, 0.501961, 1.000000, 1.000000);
      colors["aquamarine"]        = glm::vec4(0.498039, 1.000000, 0.831373, 1.000000);
      colors["brown"]             = glm::vec4(0.647059, 0.164706, 0.164706, 1.000000);
      colors["yellow"]            = glm::vec4(1.000000, 1.000000, 0.000000, 1.000000);
      colors["turquoise"]         = glm::vec4(0.250980, 0.878431, 0.815686, 1.000000);
      colors["grey0"]             = glm::vec4(0.000000, 0.000000, 0.000000, 1.000000);
      colors["grey10"]            = glm::vec4(0.101961, 0.101961, 0.101961, 1.000000);
      colors["grey20"]            = glm::vec4(0.200000, 0.200000, 0.200000, 1.000000);
      colors["grey30"]            = glm::vec4(0.301961, 0.301961, 0.301961, 1.000000);
      colors["grey40"]            = glm::vec4(0.400000, 0.400000, 0.400000, 1.000000);
      colors["grey50"]            = glm::vec4(0.498039, 0.498039, 0.498039, 1.000000);
      colors["grey60"]            = glm::vec4(0.600000, 0.600000, 0.600000, 1.000000);
      colors["grey70"]            = glm::vec4(0.701961, 0.701961, 0.701961, 1.000000);
      colors["grey"]              = glm::vec4(0.752941, 0.752941, 0.752941, 1.000000);
      colors["grey80"]            = glm::vec4(0.800000, 0.800000, 0.800000, 1.000000);
      colors["grey90"]            = glm::vec4(0.898039, 0.898039, 0.898039, 1.000000);
      colors["grey100"]           = glm::vec4(1.000000, 1.000000, 1.000000, 1.000000);
      colors["light-red"]         = glm::vec4(0.941176, 0.196078, 0.196078, 1.000000);
      colors["light-green"]       = glm::vec4(0.564706, 0.933333, 0.564706, 1.000000);
      colors["light-blue"]        = glm::vec4(0.678431, 0.847059, 0.901961, 1.000000);
      colors["light-magenta"]     = glm::vec4(0.941176, 0.333333, 0.941176, 1.000000);
      colors["light-cyan"]        = glm::vec4(0.878431, 1.000000, 1.000000, 1.000000);
      colors["light-goldenrod"]   = glm::vec4(0.933333, 0.866667, 0.509804, 1.000000);
      colors["light-pink"]        = glm::vec4(1.000000, 0.713725, 0.756863, 1.000000);
      colors["light-turquoise"]   = glm::vec4(0.686275, 0.933333, 0.933333, 1.000000);
      colors["gold"]              = glm::vec4(1.000000, 0.843137, 0.000000, 1.000000);
      colors["green"]             = glm::vec4(0.000000, 1.000000, 0.000000, 1.000000);
      colors["dark-green"]        = glm::vec4(0.000000, 0.392157, 0.000000, 1.000000);
      colors["spring-green"]      = glm::vec4(0.000000, 1.000000, 0.498039, 1.000000);
      colors["forest-green"]      = glm::vec4(0.133333, 0.545098, 0.133333, 1.000000);
      colors["sea-green"]         = glm::vec4(0.180392, 0.545098, 0.341176, 1.000000);
      colors["blue"]              = glm::vec4(0.000000, 0.000000, 1.000000, 1.000000);
      colors["dark-blue"]         = glm::vec4(0.000000, 0.000000, 0.545098, 1.000000);
      colors["midnight-blue"]     = glm::vec4(0.098039, 0.098039, 0.439216, 1.000000);
      colors["navy"]              = glm::vec4(0.000000, 0.000000, 0.501961, 1.000000);
      colors["medium-blue"]       = glm::vec4(0.000000, 0.000000, 0.803922, 1.000000);
      colors["skyblue"]           = glm::vec4(0.529412, 0.807843, 0.921569, 1.000000);
      colors["cyan"]              = glm::vec4(0.000000, 1.000000, 1.000000, 1.000000);
      colors["magenta"]           = glm::vec4(1.000000, 0.000000, 1.000000, 1.000000);
      colors["dark-turquoise"]    = glm::vec4(0.000000, 0.807843, 0.819608, 1.000000);
      colors["dark-pink"]         = glm::vec4(1.000000, 0.078431, 0.576471, 1.000000);
      colors["coral"]             = glm::vec4(1.000000, 0.498039, 0.313725, 1.000000);
      colors["light-coral"]       = glm::vec4(0.941176, 0.501961, 0.501961, 1.000000);
      colors["orange-red"]        = glm::vec4(1.000000, 0.270588, 0.000000, 1.000000);
      colors["salmon"]            = glm::vec4(0.980392, 0.501961, 0.447059, 1.000000);
      colors["dark-salmon"]       = glm::vec4(0.913725, 0.588235, 0.478431, 1.000000);
      colors["khaki"]             = glm::vec4(0.941176, 0.901961, 0.549020, 1.000000);
      colors["dark-khaki"]        = glm::vec4(0.741176, 0.717647, 0.419608, 1.000000);
      colors["dark-goldenrod"]    = glm::vec4(0.721569, 0.525490, 0.043137, 1.000000);
      colors["beige"]             = glm::vec4(0.960784, 0.960784, 0.862745, 1.000000);
      colors["olive"]             = glm::vec4(0.627451, 0.501961, 0.125490, 1.000000);
      colors["orange"]            = glm::vec4(1.000000, 0.647059, 0.000000, 1.000000);
      colors["violet"]            = glm::vec4(0.933333, 0.509804, 0.933333, 1.000000);
      colors["dark-violet"]       = glm::vec4(0.580392, 0.000000, 0.827451, 1.000000);
      colors["plum"]              = glm::vec4(0.866667, 0.627451, 0.866667, 1.000000);
      colors["dark-plum"]         = glm::vec4(0.866667, 0.313725, 0.250980, 1.000000);
      colors["dark-olivegreen"]   = glm::vec4(0.333333, 0.419608, 0.184314, 1.000000);
      colors["orangered4"]        = glm::vec4(0.501961, 0.078431, 0.000000, 1.000000);
      colors["brown4"]            = glm::vec4(0.501961, 0.078431, 0.078431, 1.000000);
      colors["sienna4"]           = glm::vec4(0.501961, 0.250980, 0.078431, 1.000000);
      colors["orchid4"]           = glm::vec4(0.501961, 0.250980, 0.501961, 1.000000);
      colors["mediumpurple3"]     = glm::vec4(0.501961, 0.376471, 0.752941, 1.000000);
      colors["slateblue1"]        = glm::vec4(0.501961, 0.376471, 1.000000, 1.000000);
      colors["yellow4"]           = glm::vec4(0.501961, 0.501961, 0.000000, 1.000000);
      colors["sienna1"]           = glm::vec4(1.000000, 0.501961, 0.250980, 1.000000);
      colors["tan1"]              = glm::vec4(1.000000, 0.627451, 0.250980, 1.000000);
      colors["sandybrown"]        = glm::vec4(1.000000, 0.627451, 0.376471, 1.000000);
      colors["light-salmon"]      = glm::vec4(1.000000, 0.627451, 0.439216, 1.000000);
      colors["pink"]              = glm::vec4(1.000000, 0.752941, 0.752941, 1.000000);
      colors["khaki1"]            = glm::vec4(1.000000, 1.000000, 0.501961, 1.000000);
      colors["lemonchiffon"]      = glm::vec4(1.000000, 1.000000, 0.752941, 1.000000);
      colors["bisque"]            = glm::vec4(0.803922, 0.717647, 0.619608, 1.000000);
      colors["honeydew"]          = glm::vec4(0.941176, 1.000000, 0.941176, 1.000000);
      colors["slategrey"]         = glm::vec4(0.627451, 0.713725, 0.803922, 1.000000);
      colors["seagreen"]          = glm::vec4(0.756863, 1.000000, 0.756863, 1.000000);
      colors["antiquewhite"]      = glm::vec4(0.803922, 0.752941, 0.690196, 1.000000);
      colors["chartreuse"]        = glm::vec4(0.486275, 1.000000, 0.250980, 1.000000);
      colors["greenyellow"]       = glm::vec4(0.627451, 1.000000, 0.125490, 1.000000);
      colors["gray"]              = glm::vec4(0.745098, 0.745098, 0.745098, 1.000000);
      colors["light-gray"]        = glm::vec4(0.827451, 0.827451, 0.827451, 1.000000);
      colors["light-grey"]        = glm::vec4(0.827451, 0.827451, 0.827451, 1.000000);
      colors["dark-gray"]         = glm::vec4(0.627451, 0.627451, 0.627451, 1.000000);
      colors["slategray"]         = glm::vec4(0.627451, 0.713725, 0.803922, 1.000000);
      colors["gray0"]             = glm::vec4(0.000000, 0.000000, 0.000000, 1.000000);
      colors["gray10"]            = glm::vec4(0.101961, 0.101961, 0.101961, 1.000000);
      colors["gray20"]            = glm::vec4(0.200000, 0.200000, 0.200000, 1.000000);
      colors["gray30"]            = glm::vec4(0.301961, 0.301961, 0.301961, 1.000000);
      colors["gray40"]            = glm::vec4(0.400000, 0.400000, 0.400000, 1.000000);
      colors["gray50"]            = glm::vec4(0.498039, 0.498039, 0.498039, 1.000000);
      colors["gray60"]            = glm::vec4(0.600000, 0.600000, 0.600000, 1.000000);
      colors["gray70"]            = glm::vec4(0.701961, 0.701961, 0.701961, 1.000000);
      colors["gray80"]            = glm::vec4(0.800000, 0.800000, 0.800000, 1.000000);
      colors["gray90"]            = glm::vec4(0.898039, 0.898039, 0.898039, 1.000000);
      colors["gray100"]           = glm::vec4(1.000000, 1.000000, 1.000000, 1.000000);
      
      isInited = true;

    }
    
  }; /* class glColors */

  std::map<std::string, glm::vec4> glColors::colors = std::map<std::string, glm::vec4>();
  
  bool glColors::isInited = false;

} /* namespace ogl */

#endif /* _H_OGL_OBJECT_H_ */
