/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PLUGINS_HPP
#define SASS_PLUGINS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"

#ifdef _WIN32

  #define LOAD_LIB(var, path) HMODULE var = LoadLibraryW(Unicode::utf8to16(path).c_str())
  #define LOAD_LIB_WCHR(var, path_wide_str) HMODULE var = LoadLibraryW(path_wide_str.c_str())
  #define LOAD_LIB_FN(type, var, name) type var = (type)(void*)GetProcAddress(plugin, name)
  #define CLOSE_LIB(var) FreeLibrary(var)

  #ifndef dlerror
  #define dlerror() 0
  #endif

#else

  #define LOAD_LIB(var, path) void* var = dlopen(path.c_str(), RTLD_LAZY)
  #define LOAD_LIB_FN(type, var, name) type var = (type) dlsym(plugin, name)
  #define CLOSE_LIB(var) dlclose(var)

#endif

namespace Sass {

  class Plugins {

  private:
    // Associated compiler
    Compiler& compiler;

  public:
    // Value constructor
    Plugins(Compiler& compiler);
    // Load one specific plugin
    bool load_plugin(const sass::string& path);
    // Load all plugins from a directory
    size_t load_plugins(const sass::string& path);

  };

}

#endif
