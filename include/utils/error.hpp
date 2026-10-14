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

#ifndef _H_OGL_ERROR_H_
#define _H_OGL_ERROR_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

#include <string>
#include <iostream>

//****************************************************************************/
// glCheckError() -
//****************************************************************************/
inline GLenum glCheckError_(const char *file, int line) {

#ifndef NDEBUG

  GLenum errorCode = GL_NO_ERROR;

  // The loop drains the whole error queue; remember the first code so the
  // caller does not always get GL_NO_ERROR back.
  GLenum firstError = GL_NO_ERROR;

  bool getError = false;

  while ((errorCode = glGetError()) != GL_NO_ERROR) {

    if(firstError == GL_NO_ERROR) firstError = errorCode;

    std::string error = "Unknown";

    switch (errorCode) {
      case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
      case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
      case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
      case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
    }

    // Any non-GL_NO_ERROR code is fatal, including codes not in the switch
    // above: an "Unknown" error must not be silently ignored.
    getError = true;

    fprintf(stderr, "ERROR [GL]: %s | %s (%d)\n", error.c_str(), file, line);

  }

  if(getError) { fflush(stderr); abort(); }

  return firstError;

#else

  return GL_NO_ERROR;

#endif



}

#define glCheckError() glCheckError_(__FILE__, __LINE__)
 
//****************************************************************************/
// GL debug output - the driver reports errors and warnings through a callback.
// It needs OpenGL 4.3 or the KHR_debug extension, so two things must hold:
//   - the installed glad must have been generated with it: if not, it does not
//     define GL_DEBUG_OUTPUT and glDebugMessageCallback, and the code that uses
//     them would not compile (older Ubuntu installs, macOS);
//   - the driver must provide it at run time: glad leaves the function pointer
//     null when it does not (macOS stops at OpenGL 4.1), and calling it would
//     crash.
// enableGlDebugOutput() checks both and otherwise does nothing.
//****************************************************************************/
#ifdef GL_DEBUG_OUTPUT
inline void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    (void) source; (void) id; (void) length; (void) userParam;
    fprintf(stderr, "ERROR [GL callback]: type=0x%x severity=0x%x message=%s\n", type, severity, message);
}
#endif

inline void enableGlDebugOutput() {
#ifdef GL_DEBUG_OUTPUT
    if(glDebugMessageCallback == nullptr) return;
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, 0);
#endif
}

#endif /* _H_OGL_ERROR_H_ */
