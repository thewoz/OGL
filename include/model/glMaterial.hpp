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

#ifndef _H_OGL_GLMATERIAL_H_
#define _H_OGL_GLMATERIAL_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>

#include <vector>
#include <string>
#include <utility>

//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {

  //****************************************************************************//
  // glMaterial
  //****************************************************************************//
  // A small, classic Phong material: a handful of colors, two scalars
  // (shininess and opacity) and the most common texture maps. The goal here is
  // to stay simple and predictable rather than to model every Assimp feature.
  //
  // Supported textures: diffuse, specular, ambient, emissive, normals, opacity.
  // Each texture, when present, overrides the matching color in the shader.
  //****************************************************************************//
  class glMaterial {

  private:

    // Material name (as reported by Assimp).
    std::string name;

    // Phong colors (the "k" naming follows the Wavefront .mtl convention).
    glm::vec3 ke = {0, 0, 0}; // emissive color
    glm::vec3 ka = {0, 0, 0}; // ambient  color
    glm::vec3 kd = {0, 0, 0}; // diffuse  color
    glm::vec3 ks = {0, 0, 0}; // specular color

    float ns = 32.0f; // specular exponent (shininess)
    float d  = 1.0f;  // opacity (1 = fully opaque)

    // Texture handles (indices into the global glTextures store).
    std::vector<int> textures;

    // Which texture maps this material actually provides.
    bool haveDiffuseTexture  = false;
    bool haveSpecularTexture = false;
    bool haveAmbientTexture  = false;
    bool haveEmissiveTexture = false;
    bool haveNormalsTexture  = false;
    bool haveOpacityTexture  = false;

    bool isInited      = false;
    bool isInitedInGpu = false;

  public:

    //****************************************************************************//
    // glMaterial - empty constructor
    //****************************************************************************//
    glMaterial() { }

    //****************************************************************************//
    // glMaterial - build the material from an Assimp material
    //****************************************************************************//
    glMaterial(const aiMaterial * material, const std::string & path) {

      aiString tmpName;
      material->Get(AI_MATKEY_NAME, tmpName);
      name = tmpName.C_Str();

      // Colors (left at zero when the model does not provide them).
      aiColor3D tmpColor;
      if(material->Get(AI_MATKEY_COLOR_EMISSIVE, tmpColor) == AI_SUCCESS) ke = glm::vec3(tmpColor.r, tmpColor.g, tmpColor.b);
      if(material->Get(AI_MATKEY_COLOR_AMBIENT,  tmpColor) == AI_SUCCESS) ka = glm::vec3(tmpColor.r, tmpColor.g, tmpColor.b);
      if(material->Get(AI_MATKEY_COLOR_DIFFUSE,  tmpColor) == AI_SUCCESS) kd = glm::vec3(tmpColor.r, tmpColor.g, tmpColor.b);
      if(material->Get(AI_MATKEY_COLOR_SPECULAR, tmpColor) == AI_SUCCESS) ks = glm::vec3(tmpColor.r, tmpColor.g, tmpColor.b);

      // Scalars.
      material->Get(AI_MATKEY_SHININESS, ns);
      material->Get(AI_MATKEY_OPACITY,   d);

      // Texture maps (only the first map of each type is used).
      loadTexture(material, aiTextureType_DIFFUSE,  "diffuseTexture",  path, haveDiffuseTexture);
      loadTexture(material, aiTextureType_SPECULAR, "specularTexture", path, haveSpecularTexture);
      loadTexture(material, aiTextureType_AMBIENT,  "ambientTexture",  path, haveAmbientTexture);
      loadTexture(material, aiTextureType_EMISSIVE, "emissiveTexture", path, haveEmissiveTexture);
      loadTexture(material, aiTextureType_NORMALS,  "normalsTexture",  path, haveNormalsTexture);
      loadTexture(material, aiTextureType_OPACITY,  "opacityTexture",  path, haveOpacityTexture);

      isInited = true;

    }

    //****************************************************************************//
    // ~glMaterial
    //****************************************************************************//
    ~glMaterial() { cleanInGpu(); }

    //****************************************************************************//
    // A material releases its textures in the destructor, so it must be movable
    // and not copyable: when glMesh is moved the source material has to be
    // neutralized, otherwise it would free textures the destination still uses.
    //****************************************************************************//
    glMaterial(const glMaterial &) = delete;
    glMaterial & operator = (const glMaterial &) = delete;

    glMaterial(glMaterial && other) noexcept { moveFrom(std::move(other)); }

    glMaterial & operator = (glMaterial && other) noexcept {
      if(this != &other) { cleanInGpu(); moveFrom(std::move(other)); }
      return *this;
    }

    //****************************************************************************//
    // setInShader - upload colors, flags and bind textures
    //****************************************************************************//
    void setInShader(const ogl::glShader & shader) {

      if(!isInitedInGpu) {
        fprintf(stderr, "ERROR [glMaterial]: must be initialized in GPU before setting in shader\n");
        abort();
      }

      // Tell the shader which texture maps are available.
      shader.setUniform("material.haveDiffuseTexture",  haveDiffuseTexture);
      shader.setUniform("material.haveSpecularTexture", haveSpecularTexture);
      shader.setUniform("material.haveAmbientTexture",  haveAmbientTexture);
      shader.setUniform("material.haveEmissiveTexture", haveEmissiveTexture);
      shader.setUniform("material.haveNormalsTexture",  haveNormalsTexture);
      shader.setUniform("material.haveOpacityTexture",  haveOpacityTexture);

      // Colors.
      shader.setUniform("material.emissiveColor", ke);
      shader.setUniform("material.ambientColor",  ka);
      shader.setUniform("material.diffuseColor",  kd);
      shader.setUniform("material.specularColor", ks);

      // Scalars.
      shader.setUniform("material.shininess", ns);
      shader.setUniform("material.opacity",   d);

      // Bind every texture to its own unit (the unit is also assigned to the
      // matching sampler uniform inside glTexture::setInShader).
      for(GLuint i=0; i<textures.size(); ++i)
        glTextures::get(textures[i]).setInShader(shader, i);

      glCheckError();

    }

    //****************************************************************************//
    // setInGpu - upload the material textures to the GPU
    //****************************************************************************//
    void setInGpu() {

      if(!isInited) {
        fprintf(stderr, "ERROR [glMaterial]: must be initialized before uploading to GPU\n");
        abort();
      }

      for(size_t i=0; i<textures.size(); ++i)
        glTextures::get(textures[i]).setInGpu();

      glCheckError();

      isInitedInGpu = true;

    }

    //****************************************************************************//
    // print - dump the material to a stream (debug helper)
    //****************************************************************************//
    void print(FILE * output = stdout) const {

      fprintf(output, "%s\n", name.c_str());

      fprintf(output, "Ke %f %f %f\n", ke[0], ke[1], ke[2]);
      fprintf(output, "Ka %f %f %f\n", ka[0], ka[1], ka[2]);
      fprintf(output, "Kd %f %f %f\n", kd[0], kd[1], kd[2]);
      fprintf(output, "Ks %f %f %f\n", ks[0], ks[1], ks[2]);

      fprintf(output, "Ns %f\n", ns);
      fprintf(output, "d %f\n",  d);

      fprintf(output, "\n");

    }

    //****************************************************************************//
    // cleanInGpu - release the material textures from the GPU
    //****************************************************************************//
    void cleanInGpu() {

      if(isInitedInGpu) {

        for(size_t i=0; i<textures.size(); ++i)
          glTextures::get(textures[i]).cleanInGpu();

        isInitedInGpu = false;

      }

    }

  private:

    //****************************************************************************//
    // moveFrom() - transfer the material and neutralize the source object
    //****************************************************************************//
    void moveFrom(glMaterial && other) {

      name                = std::move(other.name);
      ke                  = other.ke;
      ka                  = other.ka;
      kd                  = other.kd;
      ks                  = other.ks;
      ns                  = other.ns;
      d                   = other.d;
      textures            = std::move(other.textures);
      haveDiffuseTexture  = other.haveDiffuseTexture;
      haveSpecularTexture = other.haveSpecularTexture;
      haveAmbientTexture  = other.haveAmbientTexture;
      haveEmissiveTexture = other.haveEmissiveTexture;
      haveNormalsTexture  = other.haveNormalsTexture;
      haveOpacityTexture  = other.haveOpacityTexture;
      isInited            = other.isInited;
      isInitedInGpu       = other.isInitedInGpu;

      // the moved-from material must not release the textures we just took over
      other.isInitedInGpu = false;

    }

    //****************************************************************************//
    // loadTexture - load the first texture of a given type, if any
    //****************************************************************************//
    void loadTexture(const aiMaterial * material, aiTextureType type, const std::string & typeName, const std::string & path, bool & haveFlag) {

      if(material->GetTextureCount(type) == 0) return;

      aiString filename;

      // Only the first map of this type is taken into account.
      material->GetTexture(type, 0, &filename);

      textures.push_back(ogl::glTextures::load(typeName, filename.C_Str(), path));

      haveFlag = true;

    }

  };

} /* namespace ogl */

#endif /* _H_OGL_GLMATERIAL_H_ */
