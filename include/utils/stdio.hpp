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

#include <string>
#include <filesystem>
#include <system_error>

#include <climits>
#include <unistd.h>
#include <pwd.h>

//*****************************************************************************
// ogl::io
//*****************************************************************************
namespace ogl::io {

  //*****************************************************************************
  // expandPath - resolve a path to an absolute, normalized form:
  //   - a leading '~' expands to the user's home directory;
  //   - relative paths (with or without a leading '.') are resolved against
  //     the current working directory;
  //   - '.', '..' and redundant separators are folded; symlinks are resolved
  //     for the part of the path that exists (weakly_canonical).
  // Note: unlike the old wordexp()-based implementation, environment
  // variables and shell globs are NOT expanded — a path is data, not code.
  //*****************************************************************************
  inline std::string expandPath(const std::string & srcPath) {

    namespace fs = std::filesystem;

    if(srcPath.empty()) return srcPath;

    std::string s = srcPath;

    // '~' -> home directory
    if(s[0] == '~') {
      struct passwd * passwdEnt = getpwuid(getuid());
      if(passwdEnt == NULL) {
        fprintf(stderr, "ERROR [getpwuid]: cannot resolve home directory\n");
        abort();
      }
      s = std::string(passwdEnt->pw_dir) + s.substr(1);
    }

    std::error_code ec;

    fs::path abs = fs::absolute(fs::path(s), ec);
    if(ec) abs = fs::path(s);

    // Resolve symlinks on the existing prefix and fold '.'/'..'; falls back
    // to a purely lexical normalization when nothing of the path exists.
    fs::path canon = fs::weakly_canonical(abs, ec);
    if(ec || canon.empty()) canon = abs.lexically_normal();

    return canon.string();

  }

  //*****************************************************************************
  // expandPath - overloads kept for source compatibility. Every char* output
  // buffer must hold at least PATH_MAX+1 bytes; longer results are a hard
  // error instead of a buffer overflow.
  //*****************************************************************************
  inline void expandPath(const char * srcPath, char * destPath) {

    std::string out = expandPath(std::string(srcPath));

    if(out.size() > PATH_MAX) {
      fprintf(stderr, "ERROR [expandPath]: path too long: '%s'\n", srcPath);
      abort();
    }

    memcpy(destPath, out.c_str(), out.size() + 1);

  }

  inline void expandPath(char * path) {

    char buff[PATH_MAX+1];

    expandPath(path, buff);

    strcpy(path, buff);

  }

  inline void expandPath(std::string & path) {

    path = expandPath(std::string(path));

  }

  inline void expandPath(const std::string & srcPath, std::string & destPath) {

    destPath = expandPath(srcPath);

  }

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
