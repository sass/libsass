#ifndef SASS_PLUGINS_HPP
#define SASS_PLUGINS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <string>
#include <vector>
#include "unicode.hpp"
#include "sass/functions.h"

#ifdef _WIN32

  #define LOAD_LIB(var, path) HMODULE var = LoadLibraryW(Unicode::utf8to16(path).c_str())
  #define LOAD_LIB_WCHR(var, path_wide_str) HMODULE var = LoadLibraryW(path_wide_str.c_str())
  #define LOAD_LIB_FN(type, var, name) type var = (type) GetProcAddress(plugin, name)
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

    public: // c-tor
      Plugins();
      ~Plugins();

    public: // methods
      // load one specific plugin
      bool load_plugin(const sass::string& path);
      // load all plugins from a directory
      size_t load_plugins(const sass::string& path);

    public: // public accessors
      void consume_headers(sass::vector<struct SassImporter*>& destination) {
        destination.insert(destination.end(),
          std::make_move_iterator(headers.begin()),
          std::make_move_iterator(headers.end()));
        headers.clear();
      }
      void consume_importers(sass::vector<struct SassImporter*>& destination) {
        destination.insert(destination.end(),
          std::make_move_iterator(importers.begin()),
          std::make_move_iterator(importers.end()));
        importers.clear();
      }
      void consume_functions(sass::vector<struct SassFunction*>& destination) {
        destination.insert(destination.end(),
          std::make_move_iterator(functions.begin()),
          std::make_move_iterator(functions.end()));
        functions.clear();
      }

  private: // private vars
      sass::vector<struct SassImporter*> headers;
      sass::vector<struct SassImporter*> importers;
      sass::vector<struct SassFunction*> functions;

  };

}

#endif
