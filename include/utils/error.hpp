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

#ifndef _H_OGL_ERROR_H_
#define _H_OGL_ERROR_H_

#include <cstdlib>
#include <cstdio>

#include <string>
#include <iostream>


//****************************************************************************/
// glCheckError() -
//****************************************************************************/
GLenum glCheckError_(const char *file, int line) {
  
  GLenum errorCode = GL_NO_ERROR;
  
  bool getError = false;
  
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    
    std::string error;
    
    switch (errorCode)
    {
      case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; getError = true; break;
      case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; getError = true; break;
      case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; getError = true; break;
      case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; getError = true; break;
      case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; getError = true; break;
      case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; getError = true; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; getError = true; break;
    }
    
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    
  }
  
  if(getError) { fflush(stdout); abort(); }
  
  return errorCode;
  
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * userParam) {
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
          ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
          type, severity, message );
}

#endif /* _H_OGL_ERROR_H_ */
