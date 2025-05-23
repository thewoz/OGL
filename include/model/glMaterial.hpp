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

#ifndef _H_OGL_GLMATERIAL_H_
#define _H_OGL_GLMATERIAL_H_

#include <cstdlib>
#include <cstdio>

#include <vector>
#include <string>

//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {
  
  //****************************************************************************//
  // glMaterial
  //****************************************************************************//
  class glMaterial {
    
  private:
    
    // material name
    std::string name;
    
    glm::vec3 ke = {0, 0, 0}; // emission color
    glm::vec3 ka = {0, 0, 0}; // ambient color
    glm::vec3 kd = {0, 0, 0}; // diffuse color
    glm::vec3 ks = {0, 0, 0}; // specular color
    glm::vec3 kr = {0, 0, 0}; // reflection color
    glm::vec3 kt = {0, 0, 0}; // transmission color
    
    float d  = 1.0f;  // opacity
    float ns = 1.0f;  // phong exponent for ks
    //float ni = 0.98f; // index of refraction
    
    // material textures
    std::vector<int> textures;
    
    bool haveDiffuseTexture      = false;
    bool haveSpecularTexture     = false;
    bool haveAmbientTexture      = false;
    bool haveEmissiveTexture     = false;
    bool haveHeightTexture       = false;
    bool haveNormalsTexture      = false;
    bool haveShininessTexture    = false;
    bool haveOpacityTexture      = false;
    bool haveDisplacementTexture = false;
    bool haveLightTexture        = false;
    bool haveReflectionTexture   = false;
    
    bool isInited;
    bool isInitedInGpu;
    
  public:
    
    //****************************************************************************//
    // glMaterial - Empty constructor
    //****************************************************************************//
    glMaterial() : isInited(false), isInitedInGpu(false) { }
    
    //****************************************************************************//
    // glMaterial - Assimp material constructor
    //****************************************************************************//
    glMaterial(const aiMaterial * material, const std::string & path) : isInitedInGpu(false) {
      
      aiString tmpName;
      
      material->Get(AI_MATKEY_NAME, tmpName);
      
      name = tmpName.C_Str();
      
      material->Get(AI_MATKEY_COLOR_EMISSIVE,    ke[0]);
      material->Get(AI_MATKEY_COLOR_AMBIENT,     ka[0]);
      material->Get(AI_MATKEY_COLOR_DIFFUSE,     kd[0]);
      material->Get(AI_MATKEY_COLOR_SPECULAR,    ks[0]);
      material->Get(AI_MATKEY_COLOR_REFLECTIVE,  kr[0]);
      material->Get(AI_MATKEY_COLOR_TRANSPARENT, kt[0]);
      
      material->Get(AI_MATKEY_SHININESS, ns);
      
      material->Get(AI_MATKEY_OPACITY, d);
      
      // 1. Diffuse maps
      loadTextures(material, aiTextureType_DIFFUSE, "diffuseTexture", path);
      
      // 2. Specular maps
      loadTextures(material, aiTextureType_SPECULAR, "specularTexture", path);
      
      // 3. Ambient maps
      loadTextures(material, aiTextureType_AMBIENT, "ambientTexture", path);
      
      // 4. Emissive maps
      loadTextures(material, aiTextureType_EMISSIVE, "emissimeTexture", path);
      
      // 5. Height maps
      //loadTextures(material, aiTextureType_HEIGHT, "heightTexture", path);
      
      // 6. Normals maps
      loadTextures(material, aiTextureType_NORMALS, "normalsTexture", path);
      
      // 7. Shininess maps
      loadTextures(material, aiTextureType_SHININESS, "shininessTexture", path);
      
      // 8. Opacity maps
      loadTextures(material, aiTextureType_OPACITY, "opacityTexture", path);
      
      // 9. Displacement maps
      //loadTextures(material, aiTextureType_DISPLACEMENT, "displactemenTexture", path);
      
      // 10. Light maps
      loadTextures(material, aiTextureType_LIGHTMAP, "lightTexture", path);
      
      // 11. Reflection maps
      loadTextures(material, aiTextureType_REFLECTION, "reflectionTexture", path);
      
      //fprintf(stderr, "DEBUG CREATE MATERIAL '%s'\n", name.c_str());
      
      isInited = true;
      
    }
    
    //****************************************************************************//
    // ~glMaterial -
    //****************************************************************************//
    ~glMaterial() { cleanInGpu(); }
    
    //****************************************************************************//
    // setInShader
    //****************************************************************************//
    void setInShader(const ogl::glShader & shader) {
      
      if(!isInitedInGpu) {
        fprintf(stderr, "Error glMaterial: must be initialized in the GPU before set in the shader\n");
        abort();
      }
      
      // set available textures
      shader.setUniform("material.haveDiffuseTexture",      haveDiffuseTexture);
      shader.setUniform("material.haveSpecularTexture",     haveSpecularTexture);
      shader.setUniform("material.haveAmbientTexture",      haveAmbientTexture);
      shader.setUniform("material.haveEmissiveTexture",     haveEmissiveTexture);
      shader.setUniform("material.haveNormalsTexture",      haveNormalsTexture);
      shader.setUniform("material.haveShininessTexture",    haveShininessTexture);
      shader.setUniform("material.haveOpacityTexture",      haveOpacityTexture);
      shader.setUniform("material.haveLightTexture",        haveLightTexture);
      shader.setUniform("material.haveReflectionTexture",   haveReflectionTexture);
      
      // set colors
      shader.setUniform("material.emissiveColor",   ke);
      shader.setUniform("material.ambientColor",    ka);
      shader.setUniform("material.diffuseColor",    kd);
      shader.setUniform("material.specularColor",   ks);
      shader.setUniform("material.reflectiveColor", kr);
      shader.setUniform("material.trasparentColor", kt);
      
      // set
      shader.setUniform("material.shininess",  ns);
      shader.setUniform("material.opacity",     d);
            
      // TODO: questo rallenta molto
      for(GLuint i=0; i<textures.size(); i++)
        glTextures::get(textures[i]).setInShader(shader, i);
      
      glCheckError();
      
    }
    
    //****************************************************************************//
    // setInGpu() -
    //****************************************************************************//
    void setInGpu() {
      
      if(!isInited) {
        fprintf(stderr, "Error glMaterial: material must be inizialized before set in Gpu\n");
        abort();
      }
      
      for(size_t i=0; i<textures.size(); ++i)
        glTextures::get(textures[i]).setInGpu();
      
      glCheckError();

      isInitedInGpu = true;
      
    }
    
    //****************************************************************************//
    // print() -
    //****************************************************************************//
    void print(FILE * output = stdout) const {
      
      fprintf(output, "%s\n", name.c_str());
      
      fprintf(output, "Ke %f %f %f\n", ke[0], ke[1], ke[2]);
      fprintf(output, "Ka %f %f %f\n", ka[0], ka[1], ka[2]);
      fprintf(output, "Kd %f %f %f\n", kd[0], kd[1], kd[2]);
      fprintf(output, "Ks %f %f %f\n", ks[0], ks[1], ks[2]);
      fprintf(output, "Kr %f %f %f\n", kr[0], kr[1], kr[2]);
      fprintf(output, "Kt %f %f %f\n", kt[0], kt[1], kt[2]);
      
      fprintf(output, "Ns %f\n", ns);
      
      fprintf(output, "d %f\n", d);
            
      fprintf(output, "\n");
      
    }
    
    //****************************************************************************//
    // cleanInGpu() -
    //****************************************************************************//
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        for(size_t i=0; i<textures.size(); ++i) {
          glTextures::get(textures[i]).cleanInGpu();
        }
        
        isInitedInGpu = false;
        
      }
      
    }
    
  private:
    
    //****************************************************************************//
    // loadTextures() -
    //****************************************************************************//
    // Checks all material textures of a given type and loads the textures if they're not loaded yet.
    // The required info is returned as a Texture struct.
    void loadTextures(const aiMaterial * material, aiTextureType type, const std::string & typeName, const std::string & path) {
      
      if(material->GetTextureCount(type) > 0) {
        
        aiString filename;
        
        //NOTE: prendo solo la prima
        material->GetTexture(type, 0, &filename);
        
        textures.push_back(ogl::glTextures::load(typeName, filename.C_Str(), path));
        
        if(typeName == "diffuseTexture")      haveDiffuseTexture      = true;
        if(typeName == "specularTexture")     haveSpecularTexture     = true;
        if(typeName == "ambientTexture")      haveAmbientTexture      = true;
        if(typeName == "emissiveTexture")     haveEmissiveTexture     = true;
        if(typeName == "normalsTexture")      haveNormalsTexture      = true;
        if(typeName == "shininessTexture")    haveShininessTexture    = true;
        if(typeName == "opacityTexture")      haveOpacityTexture      = true;
        if(typeName == "lightTexture")        haveLightTexture        = true;
        if(typeName == "reflectionTexture")   haveReflectionTexture   = true;
        
      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLMATERIAL_H_ */
