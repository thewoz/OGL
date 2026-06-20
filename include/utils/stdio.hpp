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

#ifndef _H_OGL_STDIO_H_
#define _H_OGL_STDIO_H_


#ifndef _H_OGL_H_
  #error "Do not include this header directly; include <ogl/ogl.hpp> instead."
#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <climits>
#include <unistd.h>
#include <pwd.h>
#include <wordexp.h>

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
        if(passwdEnt == NULL) {
          fprintf(stderr, "error getpwuid(): cannot resolve home directory\n");
          abort();
        }
        strcpy(dst, passwdEnt->pw_dir);
        strcat(dst, &path[1]);
      } else {
        // bare relative path: resolve it against the current working directory
        if(getcwd(dst, PATH_MAX)==NULL) {
          fprintf(stderr, "error getcwd(): (%d) %s\n", errno, strerror(errno));
          abort();
        }
        strcat(dst, "/");
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

      size_t len = strlen(path);
      if(len == 0) return path; // empty path: nothing to fold (avoid pointer underflow below)

      s = path + (len-1);

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
      if(p.we_wordc > 0) util::appendCwd(p.we_wordv[0], buff);
      else               util::appendCwd(srcPath, buff);
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
