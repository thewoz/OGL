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

#ifndef _H_OGL_GLMODEL_H_
#define _H_OGL_GLMODEL_H_

#include <cstdio>
#include <cstdlib>

#include <vector>
#include <string>


//****************************************************************************/
// namespace ogl
//****************************************************************************/
namespace ogl {
  
  //****************************************************************************/
  // glModel
  //****************************************************************************/
  class glModel : public glObject {
    
  private:
    
    std::vector<glMesh> meshes;
  
    ogl::glLight light;

  public:
    
    //****************************************************************************/
    // glModel() - Empty constructor
    //****************************************************************************/
    glModel() : glObject() { }
    
    //****************************************************************************/
    // glModel() - Constructor, expects a filepath to a 3D model
    //****************************************************************************/
    glModel(const std::string & path, GLfloat sizeFactor = 1.0) : glObject() { init(path, sizeFactor); }
    
    //****************************************************************************/
    // ~glModel() -
    //****************************************************************************/
    ~glModel() { cleanInGpu(); }
    
    //****************************************************************************/
    // init
    //****************************************************************************/
    void init(std::string path, GLfloat normalizeTo = 1.0f) {
                
      name = ogl::io::name(path);
      
      DEBUG_LOG("glModel::init(" + name + ")");

      shader.setName(name);
      shader.initModel();
      
      ogl::io::expandPath(path);

      // ASSIMP reader file
      Assimp::Importer importer;
      
      // Load the model via ASSIMP
      const aiScene *scene = importer.ReadFile(path, aiProcess_CalcTangentSpace
                                               | aiProcess_Triangulate
                                               | aiProcess_JoinIdenticalVertices
                                               | aiProcess_FlipUVs
                                               );
      
      // Check for errors - if is Not Zero
      if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        fprintf(stderr, "ERROR::ASSIMP:: %s\n", importer.GetErrorString());
        abort();
      }
      
      // Retrieve the directory path of the filepath
      std::string directory = path.substr(0, path.find_last_of('/'));
      
      // Process ASSIMP's root node recursively
      processNode(scene->mRootNode, scene, directory);
      
      // messo qui senno normalize si incavola che non vede l'init
      isInited = true;

      if(normalizeTo !=0) normalize(normalizeTo);
      
    }
    
    //****************************************************************************/
    // setLight() - Set the light
    //****************************************************************************/
    void setLight(const glm::vec3 & _position, const glm::vec3 & _direction) {
      
      light.setPosition(_position);
      light.setDirection(_direction);

    }
    
    //****************************************************************************/
    // render() - Render the model, and thus all its meshes
    //****************************************************************************/
    void render(const glCamera * camera) {
            
      renderBegin(camera);
      
      for(std::size_t i=0; i<meshes.size(); ++i) meshes[i].render(shader);
      
      renderEnd();
      
      glCheckError();
      
    }

    //****************************************************************************/
    // renderBegin()
    //****************************************************************************/
    void renderBegin(const glCamera * camera){
              
      DEBUG_LOG("glModel::render(" + name + ")");
      
      if(!isInited){
        fprintf(stderr, "glModel must be inited before render\n");
        abort();
      }
      
      if(isToInitInGpu()) initInGpu();
      
      shader.use();
      
      shader.setUniform("projection", camera->getProjection());
      shader.setUniform("view",       camera->getView());
      shader.setUniform("model",      modelMatrix);
      
      shader.setUniform("withShadow", false);
      
      light.setInShader(shader, camera->getView());
    
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);
      
    }
    
    
    //****************************************************************************/
    // renderEnd()
    //****************************************************************************/
    void renderEnd() { glDisable(GL_DEPTH_TEST); }
    
    //****************************************************************************/
    // getBounds() - Compute the bounds of the model (center, size, radius)
    //****************************************************************************/
    void getBounds(glm::vec3 & center, glm::vec3 & size, float & radius) const {
          
      if(!isInited) {
        fprintf(stderr, "glModel must be initialized before calling getBounds\n");
        abort();
      }

      if(meshes.empty()) {
        center = glm::vec3(0.0f);
        size = glm::vec3(0.0f);
        radius = 0.0f;
        return;
      }

      glm::vec3 min_bound(+FLT_MAX);
      glm::vec3 max_bound(-FLT_MAX);

      for(std::size_t i=0; i<meshes.size(); ++i) {

        glm::vec3 tmp_center, tmp_size;
        float tmp_radius;

        meshes[i].bounds(tmp_center, tmp_size, tmp_radius);

        glm::vec3 half_size = tmp_size * 0.5f;
        glm::vec3 local_min = tmp_center - half_size;
        glm::vec3 local_max = tmp_center + half_size;

        min_bound = glm::min(min_bound, local_min);
        max_bound = glm::max(max_bound, local_max);
        
      }

      center = (min_bound + max_bound) * 0.5f;
      size   = (max_bound - min_bound);
      radius = glm::length(size) * 0.5f;
      
    }

    
    //****************************************************************************/
    // getRadius() - Compute the radius of the model
    //****************************************************************************/
    float getRadius() const {
         
      glm::vec3 center; glm::vec3 size; float radius;
      
      getBounds(center, size, radius);
      
      return radius;
      
    }
    
    //****************************************************************************/
    // setInGpu() - Copy the model into the GPU
    //****************************************************************************/
    void setInGpu() {
      
      DEBUG_LOG("glModel::setInGpu(" + name + ")");
      
      _setInGpu();
      
      for(int i=0; i<meshes.size(); ++i) meshes[i].setInGpu();
      
    }
    
    //****************************************************************************/
    // size() -
    //****************************************************************************/
    std::size_t size() const { return meshes.size(); }
    
    //****************************************************************************/
    // operator [] -
    //****************************************************************************/
    glMesh & operator [] (std::size_t index) { return meshes[index]; }
    
    
  private:
        
    //****************************************************************************/
    // normalize() - Normalize and set the center of the model
    //****************************************************************************/
    void normalize(double normalizeTo) {
            
      glm::vec3 center; glm::vec3 size; float radius = 0.0f;
      
      getBounds(center, size, radius);
            
      double scalingFactor = normalizeTo / radius;
      
      for(std::size_t i=0; i<meshes.size(); ++i)
        meshes[i].scale(scalingFactor, glm::vec3(0.0f));
      
    }
    
    //****************************************************************************/
    // processNode() - Processes a node in a recursive fashion. Processes each individual mesh
    //                 located at the node and repeats this process on its children nodes (if any)
    //****************************************************************************/
    void processNode(const aiNode * node, const aiScene * scene, const std::string & path) {
            
      // Process each mesh located at the current node
      for(GLuint i=0; i<node->mNumMeshes; ++i) {
        
        // The node object only contains indices to index the actual objects in the scene.
        // The scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
        
        meshes.push_back(glMesh(mesh, scene, path));
        
      }
      
      // After we've processed all of the meshes (if any) we then recursively process each of the children nodes
      for(GLuint i=0; i<node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, path);
      }
      
    }
    
  private:
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {
        
        for(int i=0; i<meshes.size(); ++i) meshes[i].cleanInGpu();
        
        isInitedInGpu = false;
        
      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_MPL_GLMODEL_H_ */
