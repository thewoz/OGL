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

#ifndef _H_OGL_GLMODEL_H_
#define _H_OGL_GLMODEL_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

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
  // A renderable 3D model loaded from disk through Assimp. A model is a list of
  // meshes (each with its own material) sharing a single light and the "model"
  // shader. On construction the file is imported, triangulated and (optionally)
  // normalized so that its bounding radius equals 'normalizeTo'.
  //****************************************************************************/
  class glModel : public glObject {

  private:

    // The meshes that make up the model.
    std::vector<glMesh> meshes;

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

    glModel(glModel &&) noexcept = default;
    glModel & operator = (glModel &&) noexcept = default;
    
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
                                               | aiProcess_GenSmoothNormals
                                               | aiProcess_FlipUVs
                                               );

      // Check for errors - if is Not Zero
      if(!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode) {
        fprintf(stderr, "ERROR [glModel]: Assimp error: %s\n", importer.GetErrorString());
        abort();
      }
      
      // Retrieve the directory path of the filepath
      std::string directory = path.substr(0, path.find_last_of('/'));
      
      // Process ASSIMP's root node recursively
      processNode(scene->mRootNode, scene, directory);
      
      // isInited must be set before normalize() so getBounds() doesn't abort.
      isInited = true;

      if(normalizeTo !=0) normalize(normalizeTo);
      
    }
    
    //****************************************************************************/
    // render() - Render the model, and thus all its meshes
    //****************************************************************************/
    void render(const glCamera & camera, const glScene * scene = nullptr) {

      renderBegin(camera, scene);

      for(std::size_t i=0; i<meshes.size(); ++i) meshes[i].render(shader);

      renderEnd();

      glCheckError();

    }

    //****************************************************************************/
    // renderDepth() - draw every mesh into the scene shadow map (depth only)
    //****************************************************************************/
    void renderDepth(const glShader & depthShader) override {

      if(!isInited) return;

      if(isToInitInGpu()) initInGpu();

      depthShader.setUniform("model", modelMatrix);

      for(std::size_t i=0; i<meshes.size(); ++i) meshes[i].renderDepth();

    }

    //****************************************************************************/
    // renderBegin()
    //****************************************************************************/
    void renderBegin(const glCamera & camera, const glScene * scene = nullptr){

      DEBUG_LOG("glModel::render(" + name + ")");

      if(!isInited){
        fprintf(stderr, "ERROR [glModel]: must be initialized before rendering\n");
        abort();
      }

      if(isToInitInGpu()) initInGpu();

      shader.use();

      shader.setUniform("projection", camera.getProjection());
      shader.setUniform("view",       camera.getView());
      shader.setUniform("model",      modelMatrix);

      // Lighting (and shadows) come from the scene; without one, fall back to a
      // camera head light and disable shadow sampling.
      if(scene) scene->setInShader(shader, camera.getView());
      else { glLight().setInShader(shader, camera.getView()); shader.setUniform("useShadow", false); }

      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LEQUAL);

    }


    //****************************************************************************/
    // renderEnd()
    //****************************************************************************/
    void renderEnd() { glDisable(GL_CULL_FACE); }
    
    //****************************************************************************/
    // getBounds() - Compute the bounds of the model (center, size, radius)
    //****************************************************************************/
    void getBounds(glm::vec3 & center, glm::vec3 & size, float & radius) const {
          
      if(!isInited) {
        fprintf(stderr, "ERROR [glModel]: must be initialized before calling getBounds\n");
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

      for(std::size_t i=0; i<meshes.size(); ++i) meshes[i].setInGpu();
      
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

      if(radius <= 0.0f) return; // degenerate model (e.g. single point): nothing to normalize

      double scalingFactor = normalizeTo / radius;

      for(std::size_t i=0; i<meshes.size(); ++i)
        meshes[i].scale(scalingFactor, glm::vec3(0.0f));
      
    }
    
    //****************************************************************************/
    // processNode() - Processes a node in a recursive fashion. Processes each individual mesh
    //                 located at the node and repeats this process on its children nodes (if any)
    //****************************************************************************/
    void processNode(const aiNode * node, const aiScene * scene, const std::string & path) {
            
      // Process each mesh attached to this node.
      for(GLuint i=0; i<node->mNumMeshes; ++i) {

        // Nodes only hold indices; the actual mesh data lives in the scene.
        aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
        
        meshes.push_back(glMesh(mesh, scene, path));
        
      }
      
      // Recurse into children.
      for(GLuint i=0; i<node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene, path);
      }
      
    }
    
    //****************************************************************************/
    // cleanInGpu() -
    //****************************************************************************/
    void cleanInGpu() {
      
      if(isInitedInGpu) {

        for(std::size_t i=0; i<meshes.size(); ++i) meshes[i].cleanInGpu();

        isInitedInGpu = false;
        
      }
      
    }
    
  };
  
} /* namespace ogl */

#endif /* _H_OGL_GLMODEL_H_ */
