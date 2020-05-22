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

#ifndef _H_OGL_GLSHADER_H_
#define _H_OGL_GLSHADER_H_

#include <cstdlib>
#include <cstdio>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <mpl/stdio.hpp>

/*****************************************************************************/
// namespace ogl
/*****************************************************************************/
namespace ogl {
  
  /*****************************************************************************/
  // glShader
  /*****************************************************************************/
  class glShader {
    
  private:
    
    uint32_t windowID;

    GLuint program = -1;
    
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    
    bool isInited;
    bool isInitedInGpu;
    
    std::string name = "";

  public:
    
    enum STYLE { PLAIN, MODEL, SPHERE, TEXT, ADVANCED, CUSTOM };

    int style;
    
    /*****************************************************************************/
    // glShader
    /*****************************************************************************/
    glShader() : isInited(false), isInitedInGpu(false) { }
    glShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath = "") : isInitedInGpu(false) { init(vertexPath,fragmentPath, geometryPath); }
    
    /*****************************************************************************/
     // ~glShader
     /*****************************************************************************/
    ~glShader() { if(program != -1) glDeleteProgram(program); }
    
    
    /*****************************************************************************/
    // initModel -
    /*****************************************************************************/
    void initModel() {
      init("/usr/local/include/ogl/shader/model.vs", "/usr/local/include/ogl/shader/model.fs");
      style = STYLE::MODEL;
    }
    
    /*****************************************************************************/
    // initPlain -
    /*****************************************************************************/
    void initPlain() {
      init("/usr/local/include/ogl/shader/plain.vs", "/usr/local/include/ogl/shader/plain.fs");
      style = STYLE::PLAIN;
    }

    /*****************************************************************************/
    // initSphere -
    /*****************************************************************************/
    void initSphere() {
      init("/usr/local/include/ogl/shader/sphere.vs", "/usr/local/include/ogl/shader/sphere.fs");
      style = STYLE::SPHERE;
    }
    
        
    /*****************************************************************************/
    // initText -
    /*****************************************************************************/
    void initAdvanced() {
      init("/usr/local/include/ogl/shader/advanced.vs", "/usr/local/include/ogl/shader/advanced.fs");
      style = STYLE::ADVANCED;
    }
    
    /*****************************************************************************/
    // initText -
    /*****************************************************************************/
    void initText() {
      init("/usr/local/include/ogl/shader/text.vs", "/usr/local/include/ogl/shader/text.fs");
      style = STYLE::TEXT;
    }
    
    /*****************************************************************************/
    // initText3D -
    /*****************************************************************************/
    void initText3D() {
      init("/usr/local/include/ogl/shader/text3D.vs", "/usr/local/include/ogl/shader/text.fs");
      style = STYLE::TEXT;
    }
    
    /*****************************************************************************/
    // init - Constructor generates the shader on the fly
    /*****************************************************************************/
    void init(std::string vertexPath, std::string fragmentPath, std::string geometryPath = "") {

      // 1. Retrieve the vertex/fragment source code from filePath
      std::ifstream vShaderFile;
      std::ifstream fShaderFile;
      std::ifstream gShaderFile;
      
      // ensures ifstream objects can throw exceptions:
      vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

      // expand path file
      mpl::io::expandPath(vertexPath);
      mpl::io::expandPath(fragmentPath);

      try {
        
        // Open files
        vShaderFile.open(vertexPath.c_str());
        fShaderFile.open(fragmentPath.c_str());
        
        std::stringstream vShaderStream, fShaderStream;
        
        // Read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        
        // Convert stream into string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        
        // if geometry shader path is present, also load a geometry shader
        if(!geometryPath.empty()) {
          
          mpl::io::expandPath(geometryPath);

          gShaderFile.open(geometryPath.c_str());
          std::stringstream gShaderStream;
          gShaderStream << gShaderFile.rdbuf();
          gShaderFile.close();
          geometryCode = gShaderStream.str();
          
        }
        
      } catch (std::system_error & e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << strerror(errno) << std::endl;
        abort();
      }
      
      isInited = true;
      
      style = STYLE::CUSTOM;
      
    }
    
    /* ****************************************************************************/
    // use - use the current shader
    /* ****************************************************************************/
    inline void use() {
      
      DEBUG_LOG("glShader::glUseProgram(" + name + ")");

      if(isToInitInGpu()) initInGpu();
                 
      glUseProgram(program);
            
    }
    
    /*****************************************************************************/
    // get - get the current shader program
    /*****************************************************************************/
    inline GLuint get() const { return program; }
    
    /*****************************************************************************/
    // setUniform
    /*****************************************************************************/
    template <typename T>
    inline void setUniform(const std::string & name, const T & value) const {
      
      if(!isInitedInGpu){
        fprintf(stderr, "shader must be inited in gpu before use in render\n");
        abort();
      }
      
      GLint location = glGetUniformLocation(program, name.c_str());
      
      if(location == -1 &&  glGetError() != 0){
        fprintf(stderr, "error in get uniform location \"%s\" in program %d: %s\n", name.c_str(), program, "");
        abort();
      }
            
      setUniform(location, value);
      
    }
    
    /*****************************************************************************/
    // initInGpu
    /*****************************************************************************/
    void initInGpu() {
    
      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;

      DEBUG_LOG("glShader::initInGpu(" + name + ") on windowID " + std::to_string(windowID));

      if(!isInited){
        fprintf(stderr, "shader must be inited before set in GPU\n");
        abort();
      }

      const GLchar * vShaderCode = vertexCode.c_str();
      const GLchar * fShaderCode = fragmentCode.c_str();
      
      // 2. Compile shaders
      GLuint vertex, fragment;
      GLint success;
      GLchar infoLog[512];
      
      // Vertex Shader
      vertex = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(vertex, 1, &vShaderCode, NULL);
      glCompileShader(vertex);
      
      // Print compile errors if any
      glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
      
      if(!success) {
        glGetShaderInfoLog( vertex, 512, NULL, infoLog );
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        abort();
      }
            
      // Fragment Shader
      fragment = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragment, 1, &fShaderCode, NULL);
      glCompileShader(fragment);
      
      // Print compile errors if any
      glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
      
      if(!success){
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        abort();
      }
            
      // if geometry shader is given, compile geometry shader
      GLuint geometry = 0;
      if(!geometryCode.empty()) {

        const GLchar * gShaderCode = geometryCode.c_str();

        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
         
        if(!success) {
          glGetShaderInfoLog(vertex, 512, NULL, infoLog);
          std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
          abort();
        }
                  
      }
       
      // Shader Program
      program = glCreateProgram();
             
      glAttachShader(program, vertex);
      glAttachShader(program, fragment);
       
      if(!geometryCode.empty()) glAttachShader(program, geometry);
       
      glLinkProgram(program);
             
      // Print linking errors if any
      glGetProgramiv(program, GL_LINK_STATUS, &success);
       
      if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        abort();
      }
       
      // Delete the shaders as they're linked into our program now and no longer necessery
      glDeleteShader(vertex);
      glDeleteShader(fragment);
              
      if(!geometryCode.empty()) glDeleteShader(geometry);
      
      isInitedInGpu = true;
            
    }
    
    /* ****************************************************************************/
    // setName() -
    /* ****************************************************************************/
    inline void setName(std::string _name) {
  
      name = _name;
      
    }
    
  private:
    
    /*****************************************************************************/
    // isToInitInGpu
    /*****************************************************************************/
    inline bool isToInitInGpu() const {
      
      DEBUG_LOG("glShader::isToInitInGpu(" + name + ")");
            
      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) { return true; }
      
      return false;
      
    }
    
    inline void setUniform(GLint location, const unsigned int   & value) const { glUniform1i(location, value); }
    inline void setUniform(GLint location, const int   & value) const { glUniform1ui(location, value); }
    inline void setUniform(GLint location, const float & value) const { glUniform1f(location, value); }
    inline void setUniform(GLint location, const glm::vec3 & value) const { glUniform3fv(location, 1, &value[0]); }
    inline void setUniform(GLint location, const glm::mat4 & value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLSHADER_H_ */
