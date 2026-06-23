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

#ifndef _H_OGL_GLTEXTURE_H_
#define _H_OGL_GLTEXTURE_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

#include <string>
#include <utility>
#include <unordered_map>

//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {

  //****************************************************************************/
  // glTexture
  //****************************************************************************/
  class glTexture {
    
  private:
    
    /* texture id */
    GLuint id;
    
    /* texture name */
    std::string name;
    
    /* texture init flags */
    bool isInited;
    bool isInitedInGpu;
        
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
    // glTexture
    //****************************************************************************/
    glTexture() : isInited(false), isInitedInGpu(false) { }
    
    //****************************************************************************/
    // glTexture
    //****************************************************************************/
    glTexture(const std::string & _type, const std::string & filename, const std::string & directory) : isInited(false), isInitedInGpu(false) {
      init(_type, filename, directory);
    }
    
    //****************************************************************************/
    // ~glTexture
    //****************************************************************************/
    ~glTexture() { cleanInGpu(); }

    //****************************************************************************/
    // glTexture owns a GL texture handle — movable, not copyable. Required
    // because glTextures stores them in a std::vector, whose reallocation must
    // move (not copy) the elements or the moved-from copy would delete the
    // texture still in use.
    //****************************************************************************/
    glTexture(const glTexture &) = delete;
    glTexture & operator = (const glTexture &) = delete;

    glTexture(glTexture && other) noexcept { moveFrom(std::move(other)); }

    glTexture & operator = (glTexture && other) noexcept {
      if(this != &other) { cleanInGpu(); moveFrom(std::move(other)); }
      return *this;
    }

    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(const std::string & _type, const std::string & filename, const std::string & directory){
      
      // Generate texture ID and load texture data
      
      name = filename;
      
      path = directory + '/' + filename;
            
      unsigned char * tmpImage = SOIL_load_image(path.c_str(), &width, &height, NULL, SOIL_LOAD_RGB);
    
      if(tmpImage == NULL){
        fprintf(stderr, "ERROR [glTexture]: failed to load texture\n");
        abort();
      }
            
      image = std::vector<unsigned char>(tmpImage, tmpImage + (width*height*3));

      SOIL_free_image_data(tmpImage);
      
      // Prepend "material." so the type becomes the exact sampler uniform name
      // used by the model shader (e.g. "material.diffuseTexture").
      type = "material." + _type;

      isInited = true;
      
    }
    
    //****************************************************************************/
    // setInGpu
    //****************************************************************************/
    void setInGpu() {
      
      if(!isInited){
         fprintf(stderr, "ERROR [glTexture]: must be initialized before uploading to GPU\n");
         abort();
       }
      
      glGenTextures(1, &id);
                
      // Assign texture to ID
      glBindTexture(GL_TEXTURE_2D, id);
            
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, &image[0]);
      
      glGenerateMipmap(GL_TEXTURE_2D);
  
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
      
      glBindTexture(GL_TEXTURE_2D, 0);
     
      isInitedInGpu = true;
      
      glCheckError();
      
    }
    
    //****************************************************************************/
    // setInShader
    //****************************************************************************/
    inline void setInShader(const ogl::glShader & shader, GLenum unit) {

      if(!isInitedInGpu) {
        fprintf(stderr, "ERROR [glTexture]: must be initialized in GPU before use\n");
        abort();
      }

      // Active proper texture unit before binding
      glActiveTexture(GL_TEXTURE0 + unit);

      // And finally bind the texture
      glBindTexture(GL_TEXTURE_2D, id);

      // Tell the matching sampler (e.g. "material.diffuseTexture") to read from
      // this texture unit. Without this every sampler would default to unit 0.
      shader.setUniform(type, (int)unit);

      glCheckError();

    }
    
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
    
    //****************************************************************************/
    // getType
    //****************************************************************************/
    inline std::string getType() const { return type; }

  private:

    //****************************************************************************/
    // moveFrom() - transfer ownership and neutralize the source object
    //****************************************************************************/
    void moveFrom(glTexture && other) {

      id            = other.id;
      name          = std::move(other.name);
      isInited      = other.isInited;
      isInitedInGpu = other.isInitedInGpu;
      type          = std::move(other.type);
      path          = std::move(other.path);
      width         = other.width;
      height        = other.height;
      image         = std::move(other.image);

      // the moved-from object must not delete the GPU texture we just took over
      other.isInitedInGpu = false;

    }

  };


  //****************************************************************************/
  // glTextures
  //****************************************************************************/
  class glTextures {
    
    private:

      static std::vector<glTexture> textures;
    
      static std::unordered_map<std::string, int> textureMap;
    
    public:
    
      //****************************************************************************/
      // load()
      //****************************************************************************/
      static int load(const std::string & type, const std::string & filename, const std::string & directory) {

        std::string path = directory + '/' + filename;
        
        if(textureMap.count(path)) return textureMap[path];
                
        textures.push_back(glTexture(type, filename, directory));
        
        textureMap[path] = (int)textures.size() - 1;
        
        return textureMap[path];
        
      }
    
      //****************************************************************************/
      // get()
      //****************************************************************************/
      static glTexture & get(size_t index) { return textures[index]; }
    
  };

inline std::unordered_map<std::string, int> glTextures::textureMap = std::unordered_map<std::string, int>();
inline std::vector<glTexture>               glTextures::textures   = std::vector<glTexture>();

} /* namespace ogl */

#endif /* _H_OGL_GLTEXTURE_H_ */
