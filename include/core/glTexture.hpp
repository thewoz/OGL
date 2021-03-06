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

#ifndef _H_OGL_GLTEXTURE_H_
#define _H_OGL_GLTEXTURE_H_

#include <cstdlib>
#include <cstdio>

#include <string>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glTexture
  //****************************************************************************/
  class glTexture {
    
  protected:
    
    /* texture name */
    std::string name;
    
    /* texture init flags */
    bool isInited;
    bool isInitedInGpu;
    
    /* texture id */
    GLuint id;
    
    //****************************************************************************/
    // glTexture - Empty constructor
    //****************************************************************************/
    glTexture() : isInited(false), isInitedInGpu(false) { }
    
  public:
    
    inline GLuint getId() const { return id; }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        DEBUG_LOG("glTexture::cleanInGpu(" + name + ")");

        glDeleteTextures(1, &id);
        
        isInitedInGpu = false;
        
      }
      
    }
    
  protected:
    
    //****************************************************************************/
    // activate
    //****************************************************************************/
    inline void _activate(GLenum target, GLenum unit) const {
      
      // Active proper texture unit before binding
      glActiveTexture(GL_TEXTURE0 + unit);
      
      //And finally bind the texture
      glBindTexture(target, id);
      
    }
    
  };
  
  //****************************************************************************/
  // glTexture2D
  //****************************************************************************/
  class glTexture2D : public glTexture {
    
  private:
    
    /* texture type */
    std::string type;
    
    /* texture path */
    std::string path;
    
    /* texture size */
    int width, height;
    
    /* texture data */
    std::vector<unsigned char> image;
    
  public:
    
    //****************************************************************************/
    // glTexture2D
    //****************************************************************************/
    glTexture2D() { }
    
    //****************************************************************************/
    // glTexture2D
    //****************************************************************************/
    glTexture2D(const std::string & _type, const std::string & filename, const std::string & directory) {
      init(_type, filename, directory);
    }
    
    //****************************************************************************/
    // ~glTexture2D
    //****************************************************************************/
    ~glTexture2D() { cleanInGpu(); }
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(const std::string & _type, const std::string & filename, const std::string & directory){
      
      //Generate texture ID and load texture data
      
      name  = filename;
      
      path = directory + '/' + filename;
            
      unsigned char * tmpImage = SOIL_load_image(path.c_str(), &width, &height, NULL, SOIL_LOAD_RGB);
    
      if(tmpImage == NULL){
        fprintf(stderr, "Error glTexture: error in load the texture\n");
        abort();
      }
            
      image = std::vector<unsigned char>(tmpImage, tmpImage + (width*height*3));

      SOIL_free_image_data(tmpImage);
      
      type = "material." + _type;
      
      //fprintf(stderr, "DEBUG TEXTURE create %s texture '%s' id %d\n", type.c_str(), name.c_str(), id);
      
      isInited = true;
      
    }
    
    //****************************************************************************/
    // initInGpu
    //****************************************************************************/
    void initInGpu() {
      
      if(!isInited){
         fprintf(stderr, "Error glTexture: the texture must be initialized before being set into the GPU\n");
         abort();
       }
      
      glGenTextures(1, &id);
                
      // Assign texture to ID
      glBindTexture(GL_TEXTURE_2D, id);
            
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image[0]);
      
      glGenerateMipmap(GL_TEXTURE_2D);
      
      // Parameters
      //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
      
      glBindTexture(GL_TEXTURE_2D, 0);
     
      isInitedInGpu = true;
      
    }
    
    //****************************************************************************/
    // activate
    //****************************************************************************/
    inline void activate(GLenum unit) const {
      
      if(!isInitedInGpu) {
        fprintf(stderr, "Error glTexture: the texture must be initialized in the GPU before being used\n");
        abort();
      }
      
      _activate(GL_TEXTURE_2D, unit);
      
      
    }
    
    //****************************************************************************/
    // getType
    //****************************************************************************/
    inline std::string getType() const { return type; }
    
  };


  
  /*****************************************************************************/
  // glTextureDepthMap
  /*****************************************************************************/
  class glTextureDepthMap : public glTexture {
    
  private:
    
    GLuint width;
    GLuint height;
        
  public:
    
    /*****************************************************************************/
    // glTextureDepthMap
    /*****************************************************************************/
    glTextureDepthMap() { }
    
    /*****************************************************************************/
    // TextureDepthMap
    /*****************************************************************************/
    glTextureDepthMap(GLuint _width, GLuint _height) { init(_width, _height); }
    
    /*****************************************************************************/
    // ~glTextureDepthMap
    /*****************************************************************************/
    ~glTextureDepthMap() { cleanInGpu(); }
    
    /*****************************************************************************/
    // init
    /*****************************************************************************/
    void init(GLuint _width, GLuint _height) {
      
      width  = _width;
      height = _height;
      
      glGenTextures(1, &id);
      
      glBindTexture(GL_TEXTURE_2D, id);
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
      
      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, id, 0);
      
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        abort();
      }

      glDrawBuffer(GL_NONE);
      
      isInited = true;
      
    }
    
    /*****************************************************************************/
    // activate
    /*****************************************************************************/
    inline void activate(GLenum unit) const {
      
      if(!isInited) {
        fprintf(stderr, "error: the TextureDepthMap must initialize before use it\n");
        abort();
      }
      
      _activate(GL_TEXTURE_2D, unit);
      
      //fprintf(stderr, "DEBUG TEXTURE activate GL_TEXTURE_2D id %d on unit %d\n", id, unit);
      
    }
    
  };



  
  /*****************************************************************************/
  // glTextureCubeMap
  /*****************************************************************************/
  class glTextureCubeMap : public glTexture {
    
  private:
    
    GLuint width;
    GLuint height;
        
  public:
    
    /*****************************************************************************/
    // glTextureCubeMap
    /*****************************************************************************/
    glTextureCubeMap() { }
    
    /*****************************************************************************/
    // glTextureCubeMap
    /*****************************************************************************/
    glTextureCubeMap(GLuint _width, GLuint _height) { init(_width, _height); }
    
    /*****************************************************************************/
    // ~glTextureCubeMap
    /*****************************************************************************/
    ~glTextureCubeMap() { cleanInGpu(); }
    
    /*****************************************************************************/
    // init
    /*****************************************************************************/
    void init(GLuint _width, GLuint _height) {
      
      width  = _width;
      height = _height;
      
      glGenTextures(1, &id);
      
      glBindTexture(GL_TEXTURE_CUBE_MAP, id);
      
      for(GLuint i=0; i<6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
      
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      
      //fprintf(stderr, "DEBUG TEXTURE create GL_TEXTURE_CUBE_MAP texture id %d\n", id);
      
      isInited = true;
      
    }
    
    /*****************************************************************************/
    // activate
    /*****************************************************************************/
    inline void activate(GLenum unit) const {
      
      if(!isInited) {
        fprintf(stderr, "error: the TextureCubeMap must initialize before use it\n");
        abort();
      }
      
      _activate(GL_TEXTURE_CUBE_MAP, unit);
      
      //fprintf(stderr, "DEBUG TEXTURE activate GL_TEXTURE_CUBE_MAP id %d on unit %d\n", id, unit);
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLTEXTURE_H_ */
