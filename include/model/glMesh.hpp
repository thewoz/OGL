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

#include <cstdlib>
#include <cstdio>

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
    
    GLuint id;
    
    /* Render data */
    GLuint vao, vbo, ebo;
    
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
        if(mesh->mTextureCoords[0]) { // Does the mesh contain texture coordinates?
          
          glm::vec2 vec;
          
          // A vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
          // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
          vec.x = mesh->mTextureCoords[0][i].x;
          vec.y = mesh->mTextureCoords[0][i].y;
          
          vertex.TexCoords = vec;
          
        } else { vertex.TexCoords = glm::vec2(0.0f, 0.0f); }
        
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
      
      material = ogl::glMaterial(scene->mMaterials[mesh->mMaterialIndex], path);
      
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
        fprintf(stderr, "model must be inited before render\n");
        abort();
      }
      
      if(!isInitedInGpu) { setInGpu(); }

      setInShader(shader);
              
      glBindVertexArray(vao);
      
      glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);
      
      glBindVertexArray(0);
      
      glCheckError();
            
    }
    
    //****************************************************************************//
    // bounds
    //****************************************************************************//
    void bounds(glm::vec3 & center, glm::vec3 & size, float & radius) const {

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
        fprintf(stderr, "model must be inited before set in gpu\n");
        abort();
      }
      
      // Create buffers/arrays
      glGenVertexArrays(1, &vao);
      
      glGenBuffers(1, &vbo);
      glGenBuffers(1, &ebo);
      
      glBindVertexArray(vao);
      
      // Load data into vertex buffers
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      // glVertex is a plain struct: sequential layout, safe to pass directly to the GPU.
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ogl::glVertex), vertices.data(), GL_STATIC_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
      
      std::size_t offset = 0;
      
      // Vertex Positions
      glVertexAttribPointer(/* index     = */ 0,
                            /* size      = */ 3,
                            /* type      = */ GL_FLOAT,
                            /* normalize = */ GL_FALSE,
                            /* stride    = */ sizeof(ogl::glVertex),
                            /* offset    = */ reinterpret_cast<void *>(offset));
      
      glEnableVertexAttribArray(0);
            
      offset += sizeof(float) * 3;
      
      // Vertex Normals
      glVertexAttribPointer(/* index     = */ 1,
                            /* size      = */ 3,
                            /* type      = */ GL_FLOAT,
                            /* normalize = */ GL_FALSE,
                            /* stride    = */ sizeof(ogl::glVertex),
                            /* offset    = */ reinterpret_cast<void *>(offset));
      
      glEnableVertexAttribArray(1);
      
      offset += sizeof(float) * 3;
      
      // Vertex Texture Coords
      glVertexAttribPointer(/* index     = */ 2,
                            /* size      = */ 2,
                            /* type      = */ GL_FLOAT,
                            /* normalize = */ GL_FALSE,
                            /* stride    = */ sizeof(ogl::glVertex),
                            /* offset    = */ reinterpret_cast<void *>(offset));
      
      glEnableVertexAttribArray(2);
      
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
      
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

        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);

        glDeleteVertexArrays(1, &vao);

        material.cleanInGpu();

        isInitedInGpu = false;

      }

    }

  private:

    //****************************************************************************//
    // moveFrom() - transfer ownership and neutralize the source object
    //****************************************************************************//
    void moveFrom(glMesh && other) {

      id            = other.id;
      vao           = other.vao;
      vbo           = other.vbo;
      ebo           = other.ebo;
      vertices      = std::move(other.vertices);
      indices       = std::move(other.indices);
      material      = std::move(other.material);
      isInited      = other.isInited;
      isInitedInGpu = other.isInitedInGpu;
      name          = std::move(other.name);

      // the moved-from object must not delete the GPU buffers we just took over
      other.isInitedInGpu = false;

    }

  };
  
  inline GLuint glMesh::globalId = 0;

} /* namespace ogl */

#endif /* _H_OGL_GLMESH_H_ */

