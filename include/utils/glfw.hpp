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

#ifndef _H_OGL_GLFW_H_
#define _H_OGL_GLFW_H_

#include <cstdlib>
#include <cstdio>

//****************************************************************************//
// glfw
//****************************************************************************//
namespace glfw {
  
  //****************************************************************************//
  // glfwErrorCallback
  //****************************************************************************//
  static void glfwErrorCallback(int error, const char * description) {
    
    fprintf(stderr, "GLFW error (%d): %s\n", error, description);
    
  }
  
  static bool inited = false;
  
  //****************************************************************************//
  // init
  //****************************************************************************//
  void init() {
    
    if(!inited) {
      
      DEBUG_LOG("glfw::init(true)");

      // Set GLFW error callback
      glfwSetErrorCallback(glfwErrorCallback);
      
      // Init GLFW
      if(!glfwInit()) {
        fprintf(stderr, "GLFW init error\n");
        exit(EXIT_FAILURE);
      }
      
      // Set all the required options for GLFW
      
#ifdef __APPLE__
      
      // for OpenGL 2.0 context on macOS
      // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
      // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
      
      // for OpenGL 3.0+ context on macOS
      //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
      
      // for OpenGL 4.1 context on macOS
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // This telling the mac to deprecate everything before it basically
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // This telling the mac to use the core profile
      
      glfwWindowHint(GLFW_SAMPLES, 4); // Abilito il Multi-Sample Anti-Aliasing

#else
      
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      
#endif
      
      inited = true;
      
    } else { DEBUG_LOG("glfw::init(false)"); }
    
  }
  
  //****************************************************************************//
  // terminate
  //****************************************************************************//
  void terminate() {
    
    DEBUG_LOG("glfw::terminate()");

    inited = false;
      
    glfwTerminate();
    
  }
  
} /* namespace glfw */

#endif /* _H_OGL_GLFW_H_ */
