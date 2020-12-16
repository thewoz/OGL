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

#include <cstdlib>
#include <cstdio>

#include <string>

#include "freeglut/fg_font_data.hpp"

/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {

  //****************************************************************************//
  // string 3D
  //****************************************************************************//
  inline void glPrint(const std::string & string, GLfloat x, GLfloat y, GLfloat z, int font = BITMAP_HELVETICA_18, const glm::vec3 & color = glm::vec3(1.0f)){
    
    glColor3fv(glm::value_ptr(color));
    
    glRasterPos3f(x, y, z);
    
    glutBitmapString(font, string.c_str());
    
  }

  //****************************************************************************//
  // interger 3D
  //****************************************************************************//
  inline void glPrint(int value, GLfloat x, GLfloat y, GLfloat z, int font = BITMAP_HELVETICA_18, const glm::vec3 & color = glm::vec3(1.0f)){
    
    char string[PATH_MAX]; sprintf(string, "%d", value);
    
    glColor3fv(glm::value_ptr(color));

    glRasterPos3f(x, y, z);

    glutBitmapString(font, string);

  }

  //****************************************************************************//
  // float 3D
  //****************************************************************************//
  inline void glPrint(double value, GLfloat x, GLfloat y, GLfloat z, int font = BITMAP_HELVETICA_18, const glm::vec3 & color = glm::vec3(1.0f)){
    
    char string[PATH_MAX]; sprintf(string, "%f", value);
    
    glColor3fv(glm::value_ptr(color));

    glRasterPos3f(x, y, z);
    
    glutBitmapString(font, string);
    
  }

  //****************************************************************************//
  // string 2D
  //****************************************************************************//
  inline void glPrint(const std::string & string, GLfloat x, GLfloat y, int font = BITMAP_HELVETICA_18, const glm::vec3 & color = glm::vec3(1.0f)){
    
    glColor3fv(glm::value_ptr(color));

    glRasterPos2f(x, y);
    
    glutBitmapString(font, string.c_str());
    
  }

  //****************************************************************************//
  // interger 2D
  //****************************************************************************//
  inline void glPrint(int value, GLfloat x, GLfloat y, int font = BITMAP_HELVETICA_18, const glm::vec3 & color = glm::vec3(1.0f)){
    
    char string[PATH_MAX]; sprintf(string, "%d", value);
    
    glColor3fv(glm::value_ptr(color));

    glRasterPos2f(x, y);
    
    glutBitmapString(font, string);
    
  }

  //****************************************************************************//
  // float 2D
  //****************************************************************************//
  inline void glPrint(double value, double x, double y, int font = BITMAP_HELVETICA_18, const glm::vec3 & color = glm::vec3(1.0f)){
    
    char string[PATH_MAX]; sprintf(string, "%f", value);
    
    glColor3fv(glm::value_ptr(color));

    glRasterPos2f(x, y);
    
    glutBitmapString(font, string);
    
  }


} /* namespace ogl */

#endif /* _H_OGL_PRINT_H_ */
