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

#include <cstdlib>
#include <cstdio>

#include <opencv2/opencv.hpp>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif


/*****************************************************************************/
// opengl
/*****************************************************************************/
namespace opengl {
  
  
  /*****************************************************************************/
  // glOrto
  /*****************************************************************************/
  template <typename T>
  inline cv::Mat glOrto(T left, T right, T bottom, T top, T znear, T zfar) {
    
    assert(left   != right);
    assert(bottom != top);
    assert(znear  != zfar);
    
    const T p_fn = zfar + znear;
    const T m_nf = znear - zfar; // ~ -m_fn
    
    const T p_rl = right + left;
    const T m_rl = right - left;
    const T p_tb = top + bottom;
    const T m_tb = top - bottom;
    
    const T m_lr = -m_rl;
    const T m_bt = -m_tb;
    
    return ( cv::Mat_<T>(4,4) << T(2)/m_rl,           T(0),           T(0),      p_rl/m_lr,
                                      T(0),      T(2)/m_tb,           T(0),      p_tb/m_bt,
                                      T(0),           T(0),      T(2)/m_nf,      p_fn/m_nf,
                                      T(0),           T(0),           T(0),          T(1));
    
  }
  
  
  /*****************************************************************************/
  // glFrustum
  /*****************************************************************************/
  template <typename T>
  inline cv::Mat glFrustum(T left, T right, T bottom, T top, T znear, T zfar) {
    
    assert(znear > T(0));
    assert(zfar  > T(0));
    assert(left   != right);
    assert(bottom != top);
    assert(znear  != zfar);
    
    const T x_2n = znear + znear;
    const T x_2nf = T(2) * znear * zfar;
    
    const T p_fn = zfar + znear;
    const T m_nf = znear - zfar; // ~ -m_fn
    
    const T p_rl = right + left;
    const T m_rl = right - left;
    const T p_tb = top + bottom;
    const T m_tb = top - bottom;
   
    return ( cv::Mat_<float>(4,4) << x_2n/m_rl,          T(0),     p_rl/m_rl,           T(0),
                                      T(0),     x_2n/m_tb,     p_tb/m_tb,           T(0),
                                      T(0),          T(0),     p_fn/m_nf,     x_2nf/m_nf,
                                      T(0),          T(0),         T(-1),           T(0));
    
  }
  
   
  /*****************************************************************************/
  // gluPerspective
  /*****************************************************************************/
  template <typename T>
  inline cv::Mat gluPerspective(T fovy_deg, T aspect, T znear, T zfar) {
    
    assert(znear > T(0));
    assert(zfar  > T(0));
    assert(aspect != T(0));
    assert(znear  != zfar);
    
    const T half_fovy_rad = T(M_PI) * fovy_deg / T(360);
    
    const T si = sin(half_fovy_rad);
    const T co = cos(half_fovy_rad);
    
    assert(si != T(0));
    
    const T c = co / si; // cotangent
    const T a = aspect;
    
    const T x_2nf = T(2) * znear * zfar;
    const T p_fn = zfar + znear;
    const T m_nf = znear - zfar;
    
    return ( cv::Mat_<T>(4,4) << c/a,    T(0),         T(0),          T(0),
                                T(0),       c,         T(0),          T(0),
                                T(0),    T(0),    p_fn/m_nf,    x_2nf/m_nf,
                                T(0),    T(0),        T(-1),          T(0));
    
  }
  
#if(0)
  
  //****************************************************************************//
  // gl
  //****************************************************************************//
  namespace gl {
    
    //****************************************************************************//
    // LookAt
    //****************************************************************************//
    void lookAt(const cv::Vec3f & eye, const cv::Vec3f & center, const cv::Vec3f & up) {
      
      cv::Vec3f f; cv::normalize(eye-center,  f, 1.0, 0.0, cv::NORM_L2);
      
      cv::Vec3f s; cv::normalize(f.cross(up), s, 1.0, 0.0, cv::NORM_L2);
      
      cv::Vec3f u = s.cross(f);
      
      // in openGL la matrice modelview in memoria e' salvata per colonne
      float result[16] = {         s[0],        u[0],      -f[0], 0.0,
        s[1],        u[1],      -f[1], 0.0,
        s[2],        u[2],      -f[2], 0.0,
        -s.dot(eye), -u.dot(eye), f.dot(eye), 1.0 };
      
      glMultMatrixf(&result[0]);
      
    }
    
    //****************************************************************************//
    // perspective
    //****************************************************************************//
    void perspective(float angleFOV, float aspect, float zNear, float zFar) {
      
      float fH = tan( angleFOV / 360.0 * M_PI ) * zNear;
      float fW = fH * aspect;
      
      glFrustum(-fW, fW, -fH, fH, zNear, zFar);
      
    }
  
#endif
  
} /* namespace opengl */


#endif /* _H_OGL_H_ */

