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
    // init
    //****************************************************************************/
    void init(const std::string & _type, const std::string & filename, const std::string & directory){
      
      // Generate texture ID and load texture data
      
      name = filename;
      
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
    // setInGpu
    //****************************************************************************/
    void setInGpu() {
      
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
      
      glCheckError();
      
    }
    
    //****************************************************************************/
    // setInShader
    //****************************************************************************/
    inline void setInShader(const ogl::glShader & shader, GLenum unit) const {
      
      if(!isInitedInGpu) {
        fprintf(stderr, "Error glTexture: the texture must be initialized in the GPU before being used\n");
        abort();
      }
            
      // Active proper texture unit before binding
      glActiveTexture(GL_TEXTURE0 + unit);
      
      // And finally bind the texture
      glBindTexture(GL_TEXTURE_2D, id);
      
      shader.setUniform(type, unit);

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

std::unordered_map<std::string, int> glTextures::textureMap = std::unordered_map<std::string, int>();
std::vector<glTexture>               glTextures::textures   = std::vector<glTexture>();

} /* namespace ogl */

#endif /* _H_OGL_GLTEXTURE_H_ */
