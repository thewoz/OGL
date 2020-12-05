/*
 * MIT License
 *
 * Copyright © 2017
 * Created by Leonardo Parisi (leonardo.parisi[at]gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

#include <mpl/stdio.hpp>

// Debug
//#define DEBUG_OGL
#include <ogl/utils/debug.hpp>

// Utils
#include <ogl/utils/glfw.hpp>
#include <ogl/utils/tiff.hpp>
#include <ogl/utils/error.hpp>

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
#include <ogl/objects/glLine.hpp>
#include <ogl/objects/glQuad.hpp>
#include <ogl/objects/glLines.hpp>
#include <ogl/objects/glSphere.hpp>
#include <ogl/objects/glEllipse.hpp>
#include <ogl/objects/glGrid.hpp>
#include <ogl/objects/glBox.hpp>
#include <ogl/objects/glCuboid.hpp>
#include <ogl/objects/glAxes.hpp>
#include <ogl/objects/glReferenceAxes.hpp>
#include <ogl/objects/glPoints.hpp>
#include <ogl/objects/glPrint.hpp>
#include <ogl/objects/glPrint3D.hpp>

#endif /* _H_OGL_H_ */


