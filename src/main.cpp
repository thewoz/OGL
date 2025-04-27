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

#include <cstdio>
#include <cstdlib>




#if(1)

//#define DEBUG_OGL
#define OGL_WITHOUT_IMGUI
#include <ogl/ogl.hpp>

//*****************************************************************************/
// main
//*****************************************************************************/
int main(int argc, char * const argv []) {

  ogl::glWindow window;
  
  window.create(800, 600);

  window.setCursorInputMode(GLFW_CURSOR_DISABLED);
  
  window.getCurrentCamera()->setPosition(3, 1.5, 0);
  window.getCurrentCamera()->setYaw(180);
  window.getCurrentCamera()->setPitch(-20);

  ogl::glAxes axes;
  ogl::glReferenceAxes referenceAxes;
  ogl::glGrid grid(10, 10, 0.5, ogl::glColors::cyan);
  ogl::glModel model("/usr/local/include/ogl/data/model/Trex/Trex.fbx"); model.setLight(glm::vec3(1.0), glm::vec3(-1.0));
  ogl::glPrint2D text(10, 10, ogl::glColors::white, 0.5, "FPS: ");
  
  while(!window.shouldClose()) {
    
    window.renderBegin();
    
      axes.render(window.getCurrentCamera());
      grid.render(window.getCurrentCamera());
      referenceAxes.render(window.getCurrentCamera());
      model.render(window.getCurrentCamera());
      text.render(window.getCurrentCamera(), "FPS: " + std::to_string(window.getFPS()));
    
    window.renderEnd();
        
  }
  
  return 0;
  
}

#else

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

#include <tiffio.h>
  
  //****************************************************************************//
  // snapshot
  //****************************************************************************//
  void snapshot(int width, int height, std::string outputfile) {
    
    TIFF * file = TIFFOpen(outputfile.c_str(), "w");
    
    if(file==NULL){
      abort();
    }
    
    GLubyte * image = (GLubyte*) malloc(width * height * sizeof(GLubyte) * 3);
    
    if(image==NULL){
      abort();
    }
   
    /* OpenGL's default 4 byte pack alignment would leave extra bytes at the
     end of each image row so that each full row contained a number of bytes
     divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit componets would
     be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
     just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
     the rows are packed as tight as possible (no row padding), set the pack
     alignment to 1. */
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
        
    TIFFSetField(file, TIFFTAG_IMAGEWIDTH,  width);
    TIFFSetField(file, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, "");
    
    // mi serve per non scordarmi il valore del puntatore originale
    GLubyte * p = image;
    
    for(int i=height - 1; i>=0; --i) {
      
      if(TIFFWriteScanline(file, p, i, 0) < 0) {
        fprintf(stderr, "file '%s' line %d function '%s': error in TIFFWriteScanline\n", __FILE__, __LINE__, __func__);
        abort();
      }
 
      p += width * sizeof(GLubyte) * 3;
      
    }
    
    TIFFClose(file);
    
    free(image);
    
  }

void CheckStatus( GLuint obj, bool isShader )
{
    GLint status = GL_FALSE, log[ 1 << 11 ] = { 0 };
    ( isShader ? glGetShaderiv : glGetProgramiv )( obj, isShader ? GL_COMPILE_STATUS : GL_LINK_STATUS, &status );
    if( status == GL_TRUE ) return;
    ( isShader ? glGetShaderInfoLog : glGetProgramInfoLog )( obj, sizeof( log ), NULL, (GLchar*)log );
    std::cerr << (GLchar*)log << "\n";
    std::exit( EXIT_FAILURE );
}

void AttachShader( GLuint program, GLenum type, const char* src )
{
    GLuint shader = glCreateShader( type );
    glShaderSource( shader, 1, &src, NULL );
    glCompileShader( shader );
    CheckStatus( shader, true );
    glAttachShader( program, shader );
    glDeleteShader( shader );
}

const char* const vert = 1 + R"GLSL(
#version 330 core
layout ( location = 0 ) in vec2 Position;
layout ( location = 1 ) in vec3 Color;
out VertexData
{
    vec3 Color;
} vsOutput;
void main()
{
    gl_Position = vec4( Position, 0.0, 1.0 );
    vsOutput.Color = Color;
}
)GLSL";

const char* const frag = 1 + R"GLSL(
#version 330 core
in VertexData
{
    vec3 Color;
} fsInput;
out vec4 outColor;
void main()
{
    outColor = vec4( fsInput.Color, 1.0 );
}
)GLSL";

int main( int, char** )
{
    glfwSetErrorCallback( []( int, const char* desc ) { std::cerr << desc << "\n"; std::exit( EXIT_FAILURE ); } );
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    GLFWwindow* window = glfwCreateWindow( 640, 480, "GLFW", NULL, NULL );
    glfwMakeContextCurrent( window );
    gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress );

    GLuint vbo = 0;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    struct Vertex
    {
        glm::vec2 mPosition;
        glm::vec3 mColor;
    };
    const std::vector< Vertex > verts =
    {
        { glm::vec2( -0.5f, -0.5f ), glm::vec3( 1.0f, 0.0f, 0.0f ) },
        { glm::vec2(  0.5f, -0.5f ), glm::vec3( 0.0f, 1.0f, 0.0f ) },
        { glm::vec2(  0.0f,  0.5f ), glm::vec3( 0.0f, 0.0f, 1.0f ) },
    };
    glBufferData( GL_ARRAY_BUFFER, sizeof( Vertex ) * verts.size(), verts.data(), GL_STATIC_DRAW );

    GLuint vao = 0;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0 , 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)offsetof( Vertex, mPosition ) );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1 , 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), (GLvoid*)offsetof( Vertex, mColor ) );

    GLuint prog = glCreateProgram();
    AttachShader( prog, GL_VERTEX_SHADER, vert );
    AttachShader( prog, GL_FRAGMENT_SHADER, frag );
    glLinkProgram( prog );
    CheckStatus( prog, false );
    glUseProgram( prog );

    bool snap = true;
  
    while( !glfwWindowShouldClose( window ) )
    {
        glfwPollEvents();
        int w, h;
        glfwGetFramebufferSize( window, &w, &h );
        glViewport( 0, 0, w, h );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
      
      if(snap) {
        snapshot(w, h, home + "/Desktop/test.tif");
          snap = false;
      }
      
        glfwSwapBuffers( window );
      

      
    }
  
    glfwTerminate();
}

#endif
