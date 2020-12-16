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

#ifndef _H_OGL_GLMESH_H_
#define _H_OGL_GLMESH_H_

#include <cstdlib>
#include <cstdio>

#include <string>
#include <sstream>
#include <vector>


//****************************************************************************//
// namespace ogl
//****************************************************************************//
namespace ogl {
  
  //****************************************************************************//
  // glVertex
  //****************************************************************************//
  struct glVertex {
    
    // Position
    glm::vec3 Position;
    // Normal
    glm::vec3 Normal;
    // TexCoords
    glm::vec2 TexCoords;
    
  };
  
  //****************************************************************************//
  // glMesh
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
      
      //printf("%s\n", name.c_str());
      
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
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        
        //if(name == "cella_7") printf("%f %f %f\n", vertex.Normal.x, vertex.Normal.y, vertex.Normal.z);
        
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
      
      // Now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
      for(GLuint i=0; i<mesh->mNumFaces; i++) {
        
        aiFace face = mesh->mFaces[i];
        
        // Retrieve all indices of the face and store them in the indices vector
        for(GLuint j=0; j<face.mNumIndices; j++) {
          indices.push_back(face.mIndices[j]);
        }
        
      }
      
      //fprintf(stderr, "DEBUG CREATE MESH %d\n", id);
      
      material = ogl::glMaterial(scene->mMaterials[mesh->mMaterialIndex], path);
      
      isInited = true;
      
    }
    
     
    //****************************************************************************//
    // ~glMesh -
    //****************************************************************************//
    ~glMesh() { cleanInGpu(); }
    
    
    //****************************************************************************//
    // render
    //****************************************************************************//
    void render(const glShader & shader, bool withMaterials = true) {
            
      //fprintf(stderr, "DEBUG DRAW MESH (%d) VAO: %d %s\n", id, VAO, (withMaterials) ? "with materials" : "without materials");
      
      if(!isInited){
        fprintf(stderr, "model must be inited before render\n");
        abort();
      }
      
      if(!isInitedInGpu) setInGpu();
      
      if(withMaterials) {
        material.setInShader(shader);
        material.bindTextures(shader);
      }
            
      glBindVertexArray(vao);
      
      glDrawElements(GL_TRIANGLES, (int)indices.size(), GL_UNSIGNED_INT, 0);
      
      glBindVertexArray(0);
      
      if(withMaterials) {
        material.unbindTexture(shader.get());
      }
      
    }
    
    //****************************************************************************//
    // bounds
    //****************************************************************************//
    void bounds(glm::vec3 & center, glm::vec3 & size, float & radius) const {
      
      double xMax = std::numeric_limits<double>::lowest();
      double yMax = std::numeric_limits<double>::lowest();
      double zMax = std::numeric_limits<double>::lowest();
      
      double xMin = std::numeric_limits<double>::max();
      double yMin = std::numeric_limits<double>::max();
      double zMin = std::numeric_limits<double>::max();
      
      for(std::size_t i=0; i<vertices.size(); ++i) {
        
        if(vertices[i].Position.x < xMin) xMin = vertices[i].Position.x;
        if(vertices[i].Position.x > xMax) xMax = vertices[i].Position.x;
        
        if(vertices[i].Position.y < yMin) yMin = vertices[i].Position.y;
        if(vertices[i].Position.y > yMax) yMax = vertices[i].Position.y;
        
        if(vertices[i].Position.z < zMin) zMin = vertices[i].Position.z;
        if(vertices[i].Position.z > zMax) zMax = vertices[i].Position.z;
        
      }
      
      center.x = (xMin + xMax) * 0.5f;
      center.y = (yMin + yMax) * 0.5f;
      center.z = (zMin + zMax) * 0.5f;
      
      size.x = xMax - xMin;
      size.y = yMax - yMin;
      size.z = zMax - zMin;
      
      //radius = sqrt(size.x*size.x + size.y*size.y + size.z+size.z);
      //radius = std::max(std::max(size.x, size.y), size.z);
      radius = std::max(std::max(size.x+center.x, size.y+center.y), size.z+center.z);

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
    void setInShader(const ogl::glShader & program) const { material.setInShader(program); }
    
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
      
      //fprintf(stderr, "DEBUG MESH (%d) VAO: %d VBO: %d EBO: %d\n", id, VAO, VBO, EBO);
      
      glBindVertexArray(vao);
      
      // Load data into vertex buffers
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      
      // A great thing about structs is that their memory layout is sequential for all its items.
      // The effect is that we can siogly pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
      // again translates to 3/2 floats which translates to a byte array.
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ogl::glVertex), &vertices[0], GL_STATIC_DRAW);
      
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
      
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
      
      glBindVertexArray(0);
      
      material.initInGpu();
      
      isInitedInGpu = true;

    }
    
    //****************************************************************************//
    // getVertices
    //****************************************************************************//
    const std::vector<glVertex> & getVertices() { return vertices; }
    
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
    
  };
  
  GLuint glMesh::globalId = 0;
  
} /* namspace ogl */

#endif /* _H_OGL_GLMESH_H_ */

