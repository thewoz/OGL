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

#ifndef _H_OGL_STDIO_H_
#define _H_OGL_STDIO_H_

#include <cstdlib>
#include <cstdio>

//*****************************************************************************
// ogl::io
//*****************************************************************************
namespace ogl::io {

  //*****************************************************************************
  // util
  //*****************************************************************************
  namespace util {

    //*****************************************************************************
    // appendCwd
    //*****************************************************************************
    inline void appendCwd(const char * path, char * dst) {
      
      if(path[0]=='/') {
        strcpy(dst, path);
      } else if(path[0]=='.') {
        if(getcwd(dst, PATH_MAX)==NULL) {
          fprintf(stderr, "error getcwd(): (%d) %s\n", errno, strerror(errno));
          abort();
        }
        strcat(dst, "/");
        strcat(dst, path);
      } else if(path[0]=='~') {
        struct passwd * passwdEnt = getpwuid(getuid());
        strcpy(dst, passwdEnt->pw_dir);
        strcat(dst, &path[1]);
      } else {
        dst[0] = '/'; dst[0] = '\0';
        strcat(dst, path);
      }
      
    }

    //*****************************************************************************
    // removeJunk
    //*****************************************************************************
    inline void removeJunk(char * begin, char * end) {
      while(*end!=0) { *begin++ = *end++; }
      *begin = 0;
    }

    //*****************************************************************************
    // manualPathFold
    //*****************************************************************************
    inline char * manualPathFold(char * path) {
      
      char *s, *priorSlash;
      
      while ((s=strstr(path, "/../"))!=NULL) {
        *s = 0;
        if ((priorSlash = strrchr(path, '/'))==NULL) { /* oops */ *s = '/'; break; }
        removeJunk(priorSlash, s+3);
      }
      
      while ((s=strstr(path, "/./"))!=NULL) { removeJunk(s, s+2); }
      while ((s=strstr(path, "//"))!=NULL) { removeJunk(s, s+1); }
      
      s = path + (strlen(path)-1);
      
      if (s!=path && *s=='/') { *s=0; }
      
      return path;
      
    }

  } // end namespace util

  //*****************************************************************************
  // expandPath
  //*****************************************************************************
  inline void expandPath(const char * srcPath, char * destPath) {
    
    char buff[PATH_MAX+1];
    
    wordexp_t p;
    
    if(wordexp(srcPath, &p, 0)==0){
      util::appendCwd(p.we_wordv[0], buff);
      wordfree(&p);
    } else {
      util::appendCwd(srcPath, buff);
    }
    
    if(realpath(buff, destPath)==NULL)
      strcpy(destPath, util::manualPathFold(buff));
    
  }

  //*****************************************************************************
  // expandPath
  //*****************************************************************************
  inline void expandPath(char * path) {
    
    char buff[PATH_MAX+1];
    
    expandPath(path, buff);
    
    strcpy(path, buff);
    
  }

  //*****************************************************************************
  // expandPath
  //*****************************************************************************
  inline void expandPath(std::string & path) {
    
    char buff[PATH_MAX+1];
    
    expandPath(path.c_str(), buff);
    
    path = buff;
    
  }

  //*****************************************************************************
  // expandPath
  //*****************************************************************************
  inline void expandPath(const std::string & srcPath, std::string & destPath) {
    
    char buff[PATH_MAX+1];
    
    expandPath(srcPath.c_str(), buff);
    
    destPath = buff;
    
  }

  //*****************************************************************************
  // expandPath
  //*****************************************************************************
  inline void expandPath(const std::string & srcPath, char * destPath) {
    
    expandPath(srcPath.c_str(), destPath);
    
  }

  //*****************************************************************************
  // basename
  //*****************************************************************************
  inline const char * basename(const char * filename) {
    
    const char * p = strrchr(filename, '/');
    
    return p ? p + 1 : (char *) filename;
    
  }

  //*****************************************************************************
  // basename
  //*****************************************************************************
  inline const std::string basename(const std::string & filename) {
    
    const char * p = strrchr(filename.c_str(), '/');
    
    if(p) return std::string(p + 1);
    else  return filename;
    
    return filename;
    
  }

  //*****************************************************************************
  // name
  //*****************************************************************************
  inline const std::string name(const std::string & filename) {
    
    std::string str = basename(filename);
    
    size_t lastindex = str.find_last_of(".");
    
    return str.substr(0, lastindex);
    
  }

}

#endif /* _H_OGL_STDIO_H_ */
