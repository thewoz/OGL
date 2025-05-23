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

#ifndef _H_OGL_GLSHADER_H_
#define _H_OGL_GLSHADER_H_

#include <cstdlib>
#include <cstdio>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glShader
  //****************************************************************************/
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
    
    enum STYLE { PLAIN, PLAIN2D, MODEL, SPHERE, TEXT, ADVANCED };

    int style;
    
    //****************************************************************************/
    // glShader
    //****************************************************************************/
    glShader() : isInited(false), isInitedInGpu(false) { }
    glShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath = "") : isInitedInGpu(false) { init(vertexPath,fragmentPath, geometryPath); }
    
    //****************************************************************************/
    // ~glShader
    //****************************************************************************/
    ~glShader() { if(program != -1) glDeleteProgram(program); }
    
    //****************************************************************************/
    // initModel
    //****************************************************************************/
    void initModel() {
      init("/usr/local/include/ogl/shader/model.vs", "/usr/local/include/ogl/shader/model.fs");
      style = STYLE::MODEL;
    }
    
    //****************************************************************************/
    // initPlain
    //****************************************************************************/
    void initPlain() {
      init("/usr/local/include/ogl/shader/plain.vs", "/usr/local/include/ogl/shader/plain.fs");
      style = STYLE::PLAIN;
    }

    //****************************************************************************/
    // initPlain2D
    //****************************************************************************/
    void initPlain2D() {
      init("/usr/local/include/ogl/shader/plain2D.vs", "/usr/local/include/ogl/shader/plain2D.fs");
      style = STYLE::PLAIN2D;
    }
    
    //****************************************************************************/
    // initAdvanced
    //****************************************************************************/
    void initAdvanced() {
      init("/usr/local/include/ogl/shader/advanced.vs", "/usr/local/include/ogl/shader/advanced.fs");
      style = STYLE::ADVANCED;
    }
    
    //****************************************************************************/
    // initSphere
    //****************************************************************************/
    void initSphere() {
      init("/usr/local/include/ogl/shader/sphere.vs", "/usr/local/include/ogl/shader/sphere.fs");
      style = STYLE::SPHERE;
    }
    
    //****************************************************************************/
    // initText
    //****************************************************************************/
    void initText() {
      init("/usr/local/include/ogl/shader/text.vs", "/usr/local/include/ogl/shader/text.fs");
      style = STYLE::TEXT;
    }
    
    //****************************************************************************/
    // init - Constructor generates the shader on the fly
    //****************************************************************************/
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
      ogl::io::expandPath(vertexPath);
      ogl::io::expandPath(fragmentPath);

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
          
          ogl::io::expandPath(geometryPath);

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
            
    }
    
    //****************************************************************************/
    // use() - use the current shader
    //****************************************************************************/
    inline void use() {
      
      DEBUG_LOG("glShader::glUseProgram(" + name + ")");

      if(isToInitInGpu()) initInGpu();
                 
      glUseProgram(program);
            
    }
    
    //****************************************************************************/
    // get() - get the current shader program
    //****************************************************************************/
    inline GLuint get() const { return program; }
    
    //****************************************************************************/
    // setUniform
    //****************************************************************************/
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
    
    //****************************************************************************/
    // initInGpu
    //****************************************************************************/
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
    
    //****************************************************************************/
    // setName() -
    //****************************************************************************/
    inline void setName(std::string _name) { name = _name; }
    
  private:
    
    //****************************************************************************/
    // isToInitInGpu
    //****************************************************************************/
    inline bool isToInitInGpu() const {
      
      DEBUG_LOG("glShader::isToInitInGpu(" + name + ")");
            
      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) { return true; }
      
      return false;
      
    }
    
    inline void setUniform(GLint location, const unsigned int   & value) const { glUniform1i(location, value); }
    inline void setUniform(GLint location, const int   & value) const { glUniform1ui(location, value); }
    inline void setUniform(GLint location, const float & value) const { glUniform1f(location, value); }
    inline void setUniform(GLint location, const glm::vec3 & value) const { glUniform3fv(location, 1, &value[0]); }
    
    inline void setUniform(GLint location, const glm::vec4 & value) const { glUniform4fv(location, 1, &value[0]); }

    inline void setUniform(GLint location, const glm::vec2 & value) const { glUniform2fv(location, 1, &value[0]); }
    inline void setUniform(GLint location, const glm::mat4 & value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLSHADER_H_ */
