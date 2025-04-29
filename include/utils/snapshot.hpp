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
// snapshot()
//****************************************************************************//
void snapshot(int width, int height, const char * outputfile, bool compress = false) {
  
    static GLubyte * image = nullptr;

    size_t requiredSize = width * height * 3;
  
    image = (GLubyte*)realloc(image, requiredSize);
    if(image == nullptr) {
      fprintf(stderr, "snapshot error: realloc failed\n");
      abort();
    }

    TIFF * file = TIFFOpen(outputfile, "w");
    if(!file) {
      fprintf(stderr, "snapshot error: could not open '%s'\n", outputfile);
      abort();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
    glReadBuffer(GL_FRONT);
  
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
  
    glCheckError();
    
    TIFFSetField(file, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(file, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, height);

    if(compress) {
      TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    } else {
      TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    }
  
    for(int i=0; i<height; ++i) {
      GLubyte * p = image + (width * 3 * (height - 1 - i));
      if(TIFFWriteScanline(file, p, i, 0) < 0) {
        fprintf(stderr, "snapshot error: TIFFWriteScanline failed\n");
        abort();
      }
    }

    TIFFClose(file);
  
}


//****************************************************************************//
// snapshotPBO()
//****************************************************************************//
void snapshotPBO(int width, int height, const char* outputfile, bool compress = false) {
  
    // 1. Creazione del PBO
    GLuint pbo = 0;
    glGenBuffers(1, &pbo);

    // 2. Bind del PBO e allocazione spazio
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, width * height * 3, NULL, GL_STREAM_READ);

    // 3. Impostazioni corrette
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // 4. Leggi i pixel direttamente nel PBO
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0); // <<=== GL_RGB, non RGBA

    // 5. Mappiamo il buffer
    GLubyte * ptr = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if(!ptr) {
      fprintf(stderr, "snapshotPBO() error: glMapBuffer failed\n");
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      glDeleteBuffers(1, &pbo);
      abort();
    }

    // 6. Creiamo il file TIFF
    TIFF * file = TIFFOpen(outputfile, "w");
    if(!file) {
      fprintf(stderr, "snapshotPBO() error: cannot open '%s'\n", outputfile);
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      glDeleteBuffers(1, &pbo);
      abort();
    }

    TIFFSetField(file, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(file, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    TIFFSetField(file, TIFFTAG_ROWSPERSTRIP, height);

    if(compress) {
      TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    } else {
      TIFFSetField(file, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    }

    // 7. Scriviamo il file (invertendo le righe)
    for(int i=0; i<height; ++i) {
      const GLubyte* p = ptr + (width * 3 * (height - 1 - i)); // 3 bytes per pixel (RGB)
      if(TIFFWriteScanline(file, (void*)p, i, 0) < 0) {
        fprintf(stderr, "snapshotPBO() error: TIFFWriteScanline failed at row %d\n", i);
        abort();
      }
    }

    TIFFClose(file);

    // 8. Cleanup
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glDeleteBuffers(1, &pbo);
  
}



  
} /* namespace tiff */

#endif

#endif /* _H_OGL_TIFF_H_ */
