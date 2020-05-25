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
