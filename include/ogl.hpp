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

#ifndef _H_OGL_H_
#define _H_OGL_H_

#define GL_SILENCE_DEPRECATION

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <SOIL2/SOIL2.h>

#ifndef OGL_WITHOUT_IMGUI
  #define IMGUI_DEFINE_MATH_OPERATORS
  #include <imgui/imgui.hpp>
  #ifndef OGL_WITHOUT_ICONS_FONT_AWESOME
    #include <ogl/utils/IconsFontAwesome6.h>
  #endif
#endif

// Debug
//#define DEBUG_OGL
#include <ogl/utils/debug.hpp>

// Utils
#include <ogl/utils/stdio.hpp>
#include <ogl/utils/glfw.hpp>
#include <ogl/utils/snapshot.hpp>
#include <ogl/utils/error.hpp>
#include <ogl/utils/colors.hpp>

// Core
#include <ogl/core/glCamera.hpp>
#include <ogl/core/glWindow.hpp>
#include <ogl/core/glShader.hpp>
#include <ogl/core/glTexture.hpp>
#include <ogl/core/glObject.hpp>
#include <ogl/core/glColors.hpp>
//#include "glShadow.hpp"

// Model
#include <ogl/model/glLight.hpp>
#include <ogl/model/glMaterial.hpp>
#include <ogl/model/glMesh.hpp>
#include <ogl/model/glModel.hpp>

// Objects
#include <ogl/objects/glPrint2D.hpp>
#include <ogl/objects/glPrint3D.hpp>
#include <ogl/objects/glLine.hpp>
#include <ogl/objects/glQuad.hpp>
#include <ogl/objects/glQuad2D.hpp>
#include <ogl/objects/glLines.hpp>
#include <ogl/objects/glSphere.hpp>
#include <ogl/objects/glEllipse.hpp>
#include <ogl/objects/glGrid.hpp>
#include <ogl/objects/glBox.hpp>
#include <ogl/objects/glCuboid.hpp>
#include <ogl/objects/glAxes.hpp>
#include <ogl/objects/glReferenceAxes.hpp>
#include <ogl/objects/glPoints.hpp>


#endif /* _H_OGL_H_ */


