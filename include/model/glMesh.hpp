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

#ifndef _H_OGL_GLMESH_H_
#define _H_OGL_GLMESH_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>
#include <cfloat>
#include <cstddef>

#include <string>
#include <sstream>
#include <vector>
#include <utility>


//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {
  
  //****************************************************************************//
  // glVertex - a single vertex: position, normal and texture coordinates
  //****************************************************************************//
  struct glVertex {

    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;

  };

  //****************************************************************************//
  // glMesh
  //****************************************************************************//
  // A single mesh of a model: a vertex/index buffer plus its material. It owns
  // its GPU buffers (vao/vbo/ebo), so it is movable but not copyable. The mesh
  // is uploaded to the GPU lazily on the first render() call.
  //****************************************************************************//
  class glMesh {
    
  private:
    
    static GLuint globalId;
    
    GLuint id = 0;

    /* Render data: vertex array, vertex buffer (0) and index buffer (1) */
    glBuffers buffers;
    
    /* Mesh Data */
    std::vector<glVertex> vertices;
    std::vector<GLuint> indices;
    
    /* Material Data */
    glMaterial material;
    
    bool isInited;
    bool isInitedInGpu;
    
  public:

    /* mesh name */
    std::string name;

    //****************************************************************************//
    // glMesh - Constructor
    //****************************************************************************//
    glMesh(const aiMesh * mesh, const aiScene * scene, const std::string & path) : isInitedInGpu(false) {
      
      name = mesh->mName.C_Str();

      id = globalId++;
      
      // Walk through each of the mesh's vertices
      for(GLuint i=0; i<mesh->mNumVertices; i++) {
        
        glVertex vertex;
        
        // Placeholder vector to convert to assimp vector to glm
        glm::vec3 vector;
        
        // Positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        
        // Normals
        if(mesh->HasNormals()) {
          vector.x = mesh->mNormals[i].x;
          vector.y = mesh->mNormals[i].y;
          vector.z = mesh->mNormals[i].z;
          vertex.Normal = vector;
        } else { vertex.Normal = glm::vec3(0.0f); }
                
        // Texture Coordinates
        if(mesh->mTextureCoords[0]) {
          vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
          vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
        } else { vertex.TexCoords = glm::vec2(0.0f, 0.0f); }

        // Tangents and bitangents (present when aiProcess_CalcTangentSpace is used)
        if(mesh->HasTangentsAndBitangents()) {
          vertex.Tangent.x   = mesh->mTangents[i].x;
          vertex.Tangent.y   = mesh->mTangents[i].y;
          vertex.Tangent.z   = mesh->mTangents[i].z;
          vertex.Bitangent.x = mesh->mBitangents[i].x;
          vertex.Bitangent.y = mesh->mBitangents[i].y;
          vertex.Bitangent.z = mesh->mBitangents[i].z;
        } else {
          vertex.Tangent   = glm::vec3(0.0f);
          vertex.Bitangent = glm::vec3(0.0f);
        }

        vertices.push_back(vertex);
        
      }
      
      // Walk through each face and collect the vertex indices.
      for(GLuint i=0; i<mesh->mNumFaces; i++) {
        
        aiFace face = mesh->mFaces[i];
        
        // Retrieve all indices of the face and store them in the indices vector
        for(GLuint j=0; j<face.mNumIndices; j++) {
          indices.push_back(face.mIndices[j]);
        }
        
      }
      
      // Guard against malformed files: an out-of-range material index would be
      // undefined behaviour. Fall back to the default material and warn.
      if(mesh->mMaterialIndex < scene->mNumMaterials) {
        material = ogl::glMaterial(scene->mMaterials[mesh->mMaterialIndex], path);
      } else {
        fprintf(stderr, "WARNING [glMesh]: material index %u out of range (%u materials), using default material\n",
                mesh->mMaterialIndex, scene->mNumMaterials);
      }
      
      isInited = true;
      
    }
    
     
    //****************************************************************************//
    // ~glMesh -
    //****************************************************************************//
    ~glMesh() { cleanInGpu(); }

    //****************************************************************************//
    // glMesh owns GPU handles (vao/vbo/ebo) — movable, not copyable.
    // Without this, vector<glMesh> reallocation would copy the handle values and
    // both the old and new object would call glDelete* on the same buffers.
    //****************************************************************************//
    glMesh(const glMesh &) = delete;
    glMesh & operator = (const glMesh &) = delete;

    glMesh(glMesh && other) noexcept { moveFrom(std::move(other)); }

    glMesh & operator = (glMesh && other) noexcept {
      if(this != &other) { cleanInGpu(); moveFrom(std::move(other)); }
      return *this;
    }
    
    
    //****************************************************************************//
    // render
    //****************************************************************************//
    void render(const glShader & shader) {
                 
      if(!isInited){
        fprintf(stderr, "ERROR [glMesh]: must be initialized before rendering\n");
        abort();
      }
      
      if(!isInitedInGpu) { setInGpu(); }

      setInShader(shader);
              
      buffers.bind();

      glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);

      glBindVertexArray(0);

      glCheckError();

    }

    //****************************************************************************//
    // renderDepth - draw the geometry only (no material), for the shadow map.
    // The depth shader and its 'model' uniform are set by the caller (glModel).
    //****************************************************************************//
    void renderDepth() {

      if(!isInited) return;

      if(!isInitedInGpu) setInGpu();

      buffers.bind();
      glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);

      glCheckError();

    }

    //****************************************************************************//
    // bounds
    //****************************************************************************//
    void bounds(glm::vec3 & center, glm::vec3 & size, float & radius) const {

      // no vertices, no extent: the min/max below would be left at +-FLT_MAX
      if(vertices.empty()) {
        center = glm::vec3(0.0f);
        size   = glm::vec3(0.0f);
        radius = 0.0f;
        return;
      }

      float xMin =  FLT_MAX, xMax = -FLT_MAX;
      float yMin =  FLT_MAX, yMax = -FLT_MAX;
      float zMin =  FLT_MAX, zMax = -FLT_MAX;

      for(std::size_t i=0; i<vertices.size(); ++i) {
        const glm::vec3 & pos = vertices[i].Position;
        if(pos.x < xMin) xMin = pos.x;  if(pos.x > xMax) xMax = pos.x;
        if(pos.y < yMin) yMin = pos.y;  if(pos.y > yMax) yMax = pos.y;
        if(pos.z < zMin) zMin = pos.z;  if(pos.z > zMax) zMax = pos.z;
      }

      center.x = (xMin + xMax) * 0.5f;
      center.y = (yMin + yMax) * 0.5f;
      center.z = (zMin + zMax) * 0.5f;

      size.x = xMax - xMin;
      size.y = yMax - yMin;
      size.z = zMax - zMin;
        
      radius = 0.5f * glm::length(size);
      
    }

    
    //****************************************************************************//
    // scale
    //****************************************************************************//
    void scale(double scaleFactor, glm::vec3 offset) {
      
      for(std::size_t i=0; i<vertices.size(); ++i) {
        vertices[i].Position += offset;
        vertices[i].Position *= scaleFactor;
      }
      
    }
    
    //****************************************************************************//
    // setInShader
    //****************************************************************************//
    void setInShader(const ogl::glShader & program) { material.setInShader(program); }
    
    //****************************************************************************//
    // setInGpu - Initializes all the buffer objects/arrays
    //****************************************************************************//
    void setInGpu() {
      
      if(!isInited){
        fprintf(stderr, "ERROR [glMesh]: must be initialized before uploading to GPU\n");
        abort();
      }
      
      // Fresh names: glModel uploads its meshes again after a context change,
      // and the old ones are left to their own context.
      buffers.create(2);
      buffers.bind();

      // glVertex is a plain struct: sequential layout, safe to pass directly to the GPU.
      buffers.data(0, vertices.data(), vertices.size() * sizeof(ogl::glVertex));
      buffers.elements(1, indices.data(), indices.size() * sizeof(GLuint));

      // position, normal, texture coordinates, tangent, bitangent
      buffers.pointer(0, 3, sizeof(ogl::glVertex), offsetof(ogl::glVertex, Position));
      buffers.pointer(1, 3, sizeof(ogl::glVertex), offsetof(ogl::glVertex, Normal));
      buffers.pointer(2, 2, sizeof(ogl::glVertex), offsetof(ogl::glVertex, TexCoords));
      buffers.pointer(3, 3, sizeof(ogl::glVertex), offsetof(ogl::glVertex, Tangent));
      buffers.pointer(4, 3, sizeof(ogl::glVertex), offsetof(ogl::glVertex, Bitangent));

      glBuffers::unbind();
      
      glCheckError();

      material.setInGpu();
      
      isInitedInGpu = true;
      
      glCheckError();

    }
    
    //****************************************************************************//
    // getVertices
    //****************************************************************************//
    const std::vector<glVertex> & getVertices() const { return vertices; }
    
    //****************************************************************************//
    // cleanInGpu() -
    //****************************************************************************//
    void cleanInGpu() {

      if(isInitedInGpu) {

        buffers.destroy();

        material.cleanInGpu();

        isInitedInGpu = false;

      }

    }

    //****************************************************************************//
    // forgetInGpu() - drop the GPU state without deleting the buffers, for
    // when the context that owns them is not the current one (see glObject)
    //****************************************************************************//
    void forgetInGpu() {

      buffers.forget();

      material.cleanInGpu();

      isInitedInGpu = false;

    }

  private:

    //****************************************************************************//
    // moveFrom() - transfer ownership and neutralize the source object
    //****************************************************************************//
    void moveFrom(glMesh && other) {

      id            = other.id;
      buffers       = other.buffers;
      vertices      = std::move(other.vertices);
      indices       = std::move(other.indices);
      material      = std::move(other.material);
      isInited      = other.isInited;
      isInitedInGpu = other.isInitedInGpu;
      name          = std::move(other.name);

      // the moved-from object must not delete the GPU buffers we just took over
      other.isInitedInGpu = false;
      other.buffers.forget();

    }

  };
  
  inline GLuint glMesh::globalId = 0;

} /* namespace ogl */

#endif /* _H_OGL_GLMESH_H_ */

