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

#ifndef _H_OGL_GLSHADER_H_
#define _H_OGL_GLSHADER_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glShader
  //****************************************************************************/
  // Manages a GLSL program (vertex + optional geometry + fragment).
  // The source is loaded from disk at init() time; compilation and linking are
  // deferred to the first use() call (when a GL context is guaranteed active).
  // Each shader instance tracks the window it was compiled for; if the context
  // changes, it recompiles automatically on the next use().
  //****************************************************************************/
  class glShader {
    
  private:
    
    uint32_t windowID = 0;

    GLuint program = 0; // 0 is the OpenGL null handle (no program compiled yet)
    
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    
    bool isInited;
    bool isInitedInGpu;

    std::string name = "";

    // Cache of uniform name -> location for the current program. Avoids a
    // glGetUniformLocation (string hash + driver lookup) on every setUniform call.
    // Inactive uniforms are cached as -1 too, so they are skipped cheaply.
    // Invalidated whenever the program is (re)created.
    mutable std::unordered_map<std::string, GLint> uniformLocationCache;

  public:
    
    enum STYLE { SOLID, WIREFRAME, LINE, POINTS, TEXT, MODEL, PLAIN2D, SHADOW };

    int style = STYLE::SOLID;
    
    //****************************************************************************/
    // glShader
    //****************************************************************************/
    glShader() : isInited(false), isInitedInGpu(false) { }
    glShader(std::string vertexPath, std::string fragmentPath, std::string geometryPath = "") : isInitedInGpu(false) { init(vertexPath,fragmentPath, geometryPath); }
    
    //****************************************************************************/
    // ~glShader
    //****************************************************************************/
    ~glShader() { if(program != 0) glDeleteProgram(program); }

    //****************************************************************************/
    // A shader owns a GL program handle, so it is non-copyable: copying would
    // duplicate the handle and double-delete it. (glShader instances live as
    // members of the non-copyable glObject, never in containers by value.)
    //****************************************************************************/
    glShader(const glShader &) = delete;
    glShader & operator = (const glShader &) = delete;

    glShader(glShader && o) noexcept
      : windowID(o.windowID), program(o.program),
        vertexCode(std::move(o.vertexCode)), fragmentCode(std::move(o.fragmentCode)),
        geometryCode(std::move(o.geometryCode)),
        isInited(o.isInited), isInitedInGpu(o.isInitedInGpu),
        name(std::move(o.name)),
        uniformLocationCache(std::move(o.uniformLocationCache)), style(o.style) {
      // Neutralize the source: with program == 0 but isInitedInGpu still true, a
      // reused moved-from shader would silently glUseProgram(0).
      o.program       = 0;
      o.isInitedInGpu = false;
      o.windowID      = 0;
    }

    glShader & operator = (glShader && o) noexcept {
      if(this != &o) {
        if(program != 0) glDeleteProgram(program);
        windowID     = o.windowID;
        program      = o.program;      o.program = 0;
        vertexCode   = std::move(o.vertexCode);
        fragmentCode = std::move(o.fragmentCode);
        geometryCode = std::move(o.geometryCode);
        isInited     = o.isInited;
        isInitedInGpu = o.isInitedInGpu;
        name         = std::move(o.name);
        uniformLocationCache = std::move(o.uniformLocationCache);
        style        = o.style;
        o.isInitedInGpu = false;
        o.windowID      = 0;
      }
      return *this;
    }
    
    //****************************************************************************/
    // initModel
    //****************************************************************************/
    void initModel() {
      init("/usr/local/include/ogl/shader/model.vs", "/usr/local/include/ogl/shader/model.fs");
      style = STYLE::MODEL;
    }
    
    //****************************************************************************/
    // initSolid
    //****************************************************************************/
    void initSolid() {
      init("/usr/local/include/ogl/shader/solid.vs", "/usr/local/include/ogl/shader/solid.fs");
      style = STYLE::SOLID;
    }

    //****************************************************************************/
    // initShadow - depth-only program used to render the scene shadow map
    //****************************************************************************/
    void initShadow() {
      init("/usr/local/include/ogl/shader/shadow.vs", "/usr/local/include/ogl/shader/shadow.fs");
      style = STYLE::SHADOW;
    }

    //****************************************************************************/
    // initPlain2D
    //****************************************************************************/
    void initPlain2D() {
      init("/usr/local/include/ogl/shader/plain2D.vs", "/usr/local/include/ogl/shader/plain2D.fs");
      style = STYLE::PLAIN2D;
    }

    //****************************************************************************/
    // initWireframe
    //****************************************************************************/
    void initWireframe() {
      init("/usr/local/include/ogl/shader/wireframe.vs", "/usr/local/include/ogl/shader/wireframe.fs", "/usr/local/include/ogl/shader/wireframe.gs");
      style = STYLE::WIREFRAME;
    }

    //****************************************************************************/
    // initLine
    //****************************************************************************/
    void initLine() {
      init("/usr/local/include/ogl/shader/line.vs", "/usr/local/include/ogl/shader/line.fs", "/usr/local/include/ogl/shader/line.gs");
      style = STYLE::LINE;
    }
    
    //****************************************************************************/
    // initPoints
    //****************************************************************************/
    void initPoints() {
      init("/usr/local/include/ogl/shader/points.vs", "/usr/local/include/ogl/shader/points.fs");
      style = STYLE::POINTS;
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

      if(program != 0) {
        glDeleteProgram(program);
        program = 0;
      }
      uniformLocationCache.clear();
      isInitedInGpu = false;

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
        fprintf(stderr, "ERROR [glShader]: file not successfully read: %s\n", strerror(errno));
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
    // setUniform - upload a typed value to a named uniform.
    // The location is looked up once and cached (see getUniformLocation).
    // Inactive uniforms (location == -1, e.g. optimized-out by the GLSL compiler)
    // are silently skipped; this avoids spurious aborts on unused uniforms.
    //****************************************************************************/
    template <typename T>
    inline void setUniform(const std::string & name, const T & value) const {

      if(!isInitedInGpu) {
        fprintf(stderr, "ERROR [glShader]: must be initialized in GPU before use\n");
        abort();
      }

      GLint location = getUniformLocation(name);

      if(location == -1) return; // inactive / optimized-out uniform

      setUniform(location, value);

    }
    
    //****************************************************************************/
    // initInGpu
    //****************************************************************************/
    void initInGpu() {
    
      windowID = ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id;

      DEBUG_LOG("glShader::initInGpu(" + name + ") on windowID " + std::to_string(windowID));

      if(!isInited){
        fprintf(stderr, "ERROR [glShader]: must be initialized before uploading to GPU\n");
        abort();
      }

      // Recompiling (e.g. after a context change): drop the previous program so
      // its handle is not leaked, and invalidate the stale uniform locations.
      if(program != 0) { glDeleteProgram(program); program = 0; }
      uniformLocationCache.clear();

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
        fprintf(stderr, "ERROR [glShader]: vertex shader compilation failed\n%s\n", infoLog);
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
        fprintf(stderr, "ERROR [glShader]: fragment shader compilation failed\n%s\n", infoLog);
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
          glGetShaderInfoLog(geometry, 512, NULL, infoLog);
          fprintf(stderr, "ERROR [glShader]: geometry shader compilation failed\n%s\n", infoLog);
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
        fprintf(stderr, "ERROR [glShader]: program linking failed\n%s\n", infoLog);
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
    // getUniformLocation - cached glGetUniformLocation. Misses (including
    // inactive uniforms, location == -1) are cached too, so a name is queried
    // from the driver at most once per compiled program.
    //****************************************************************************/
    inline GLint getUniformLocation(const std::string & uniformName) const {

      auto it = uniformLocationCache.find(uniformName);
      if(it != uniformLocationCache.end()) return it->second;

      GLint location = glGetUniformLocation(program, uniformName.c_str());
      uniformLocationCache[uniformName] = location;
      return location;

    }

    //****************************************************************************/
    // isToInitInGpu
    //****************************************************************************/
    inline bool isToInitInGpu() const {
      
      DEBUG_LOG("glShader::isToInitInGpu(" + name + ")");
            
      if(windowID != ((glWindow*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->id || !isInitedInGpu) { return true; }
      
      return false;
      
    }
    
    inline void setUniform(GLint location, const unsigned int   & value) const { glUniform1ui(location, value); }
    inline void setUniform(GLint location, const int   & value) const { glUniform1i(location, value); }
    inline void setUniform(GLint location, const float & value) const { glUniform1f(location, value); }
    inline void setUniform(GLint location, const glm::vec3 & value) const { glUniform3fv(location, 1, &value[0]); }
    
    inline void setUniform(GLint location, const glm::vec4 & value) const { glUniform4fv(location, 1, &value[0]); }

    inline void setUniform(GLint location, const glm::vec2 & value) const { glUniform2fv(location, 1, &value[0]); }
    inline void setUniform(GLint location, const glm::mat4 & value) const { glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)); }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLSHADER_H_ */
