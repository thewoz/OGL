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

#ifndef _H_OGL_TIFF_H_
#define _H_OGL_TIFF_H_

#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

//#ifdef OPENCV_ALL_HPP
#ifdef OGL_TEST_SNAPSHOT

//****************************************************************************//
// namespace cv
//****************************************************************************//
namespace cv {
  
  //****************************************************************************//
  // snapshot
  //****************************************************************************//
  void snapshot(int width, int height, const char * outputfile) {
    
    cv::Mat image(width, height, CV_8UC3);
    
    glPixelStorei(GL_PACK_ALIGNMENT, (image.step & 3) ? 1 : 4);
    
    glPixelStorei(GL_PACK_ROW_LENGTH, (int)image.step/image.elemSize());
    
    glReadPixels(0, 0, image.cols, image.rows, GL_BGR, GL_UNSIGNED_BYTE, image.data);
    
    cv::flip(image, image, 0);

    cv::imwrite(outputfile, image);
    
  }
  
}

#else


//****************************************************************************//
// namespace tiff
//****************************************************************************//
namespace tiff {
  
#include <tiffio.h>
  
  //****************************************************************************//
  // snapshot
  //****************************************************************************//
  void snapshot(int width, int height, const char * outputfile) {
    
    TIFF * file = TIFFOpen(outputfile, "w");
    
    if(file==NULL){
      fprintf(stderr, "file '%s' line %d function '%s': error in opening file '%s'\n", __FILE__, __LINE__, __func__, outputfile);
      exit(EXIT_FAILURE);
    }
    
    GLubyte * image = (GLubyte *) malloc(width * height * sizeof (GLubyte) * 3);
    
    if(image==NULL){
      fprintf(stderr, "file '%s' line %d function '%s': error in malloc %s\n", __FILE__, __LINE__, __func__, strerror(errno));
      abort();
    }
    
    /* OpenGL's default 4 byte pack alignment would leave extra bytes at the
     end of each image row so that each full row contained a number of bytes
     divisible by 4.  Ie, an RGB row with 3 pixels and 8-bit componets would
     be laid out like "RGBRGBRGBxxx" where the last three "xxx" bytes exist
     just to pad the row out to 12 bytes (12 is divisible by 4). To make sure
     the rows are packed as tight as possible (no row padding), set the pack
     alignment to 1. */
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
    
    // TODO: controllare il valore di ritorno di glReadPixels
    
    TIFFSetField(file, TIFFTAG_IMAGEWIDTH, (uint32) width);
    TIFFSetField(file, TIFFTAG_IMAGELENGTH, (uint32) height);
    TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
    TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, 1);
    TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, "");
    
    // mi serve per non scordarmi il valore del puntatore originale
    GLubyte * p = image;
    
    for(int i=height - 1; i>=0; --i) {
      
      if(TIFFWriteScanline(file, p, i, 0) < 0) {
        fprintf(stderr, "file '%s' line %d function '%s': error in TIFFWriteScanline\n", __FILE__, __LINE__, __func__);
        abort();
      }
      
      p += width * sizeof (GLubyte) * 3;
      
    }
    
    TIFFClose(file);
    
    free(image);
    
  }
  
} /* namespace tiff */

#endif

#endif /* _H_OGL_TIFF_H_ */
