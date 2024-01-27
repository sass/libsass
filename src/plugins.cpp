/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "plugins.hpp"

#include <cstring>
#include <sstream>
#include "unicode.hpp"
#include "compiler.hpp"
#include "string_utils.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>
#endif

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Constructor
  Plugins::Plugins(Compiler& compiler) :
    compiler(compiler) {}

  // check if plugin is compatible with this version
  // plugins may be linked static against libsass
  // we try to be compatible between major versions
  inline bool compatibility(const char* their_version)
  {
    // first check if anyone has an unknown version
    const char* our_version = libsass_version();
    if (!strcmp(their_version, "[na]")) return false;
    if (!strcmp(our_version, "[na]")) return false;

    // find the position of the second dot
    size_t pos = sass::string(our_version).find('.', 0);
    if (pos != sass::string::npos) pos = sass::string(our_version).find('.', pos + 1);

    // if we do not have two dots we fall back to compare complete string
    if (pos == sass::string::npos) { return strcmp(their_version, our_version) ? 0 : 1; }
    // otherwise only compare up to the second dot (major versions)
    else { return strncmp(their_version, our_version, pos) ? 0 : 1; }

  }

  // load one specific plugin
  bool Plugins::load_plugin (const sass::string& path)
  {

    typedef const char* (*__plugin_version__)(void);
    typedef const char* (*__plugin_set_seed__)(uint32_t);
    // typedef struct SassFunctionList* (*__plugin_load_fns__)(void);
    // typedef struct SassImporterList* (*__plugin_load_imps__)(void);

    typedef void(*__plugin_init__)(struct SassCompiler* compiler);

    // Pass seed by env variable until further notice
    sass::sstream strm; strm << getHashSeed();
    // SET_ENV("SASS_HASH_SEED", strm.str().c_str());

    if (LOAD_LIB(plugin, path))
    {
      // try to load initial function to query libsass version support
      if (LOAD_LIB_FN(__plugin_version__, plugin_version, "libsass_get_version"))
      {
        // get the libsass version of the plugin
        if (!compatibility(plugin_version())) return false;
        // try to get import address for "plugin_set_seed_function"
        if (LOAD_LIB_FN(__plugin_set_seed__, plugin_set_seed_function, "libsass_set_seed_function"))
        {
          plugin_set_seed_function(getHashSeed());
        }

        // try to get import address for "plugin_init" function
        if (LOAD_LIB_FN(__plugin_init__, plugin_init_function, "libsass_init_plugin"))
        {
          plugin_init_function(compiler.wrap());
        }

        return true;
      }
      else
      {
        // print debug message to stderr (should not happen)
        std::cerr << "failed loading 'libsass_support' in <" << path << ">" << STRMLF;
        if (const char* dlsym_error = dlerror()) std::cerr << dlsym_error << STRMLF;
        CLOSE_LIB(plugin);
      }
    }
    else
    {
      // print debug message to stderr (should not happen)
      std::cerr << "failed loading plugin <" << path << ">" << STRMLF;
      if (const char* dlopen_error = dlerror()) std::cerr << dlopen_error << STRMLF;
    }

    return false;

  }

  size_t Plugins::load_plugins(const sass::string& path)
  {

    // count plugins
    size_t loaded = 0;

    #ifdef _WIN32

      try
      {

        // use wchar (utf16)
        WIN32_FIND_DATAW data;
        // trailing slash is guaranteed
        sass::string globsrch(path + "*.dll");
        // convert to wide chars (utf16) for system call
        sass::wstring wglobsrch(Unicode::utf8to16(globsrch));
        HANDLE hFile = FindFirstFileW(wglobsrch.c_str(), &data);
        // check if system called returned a result
        // ToDo: maybe we should print a debug message
        if (hFile == INVALID_HANDLE_VALUE) return -1;

        // read directory
        while (true)
        {
          try
          {
            // the system will report the filenames with wide chars (utf16)
            sass::string entry = Unicode::utf16to8(data.cFileName);
            // check if file ending matches exactly
            if (!StringUtils::endsWith(entry, ".dll", 4)) continue;
            // load the plugin and increase counter
            if (load_plugin(path + entry)) ++ loaded;
            // check if there should be more entries
            if (GetLastError() == ERROR_NO_MORE_FILES) break;
            // load next entry (check for return type)
            if (!FindNextFileW(hFile, &data)) break;
          }
          catch (...)
          {
            // report the error to the console (should not happen)
            // seems like we got strange data from the system call?
            std::cerr << "filename in plugin path has invalid utf8?" << STRMLF;
          }
        }
      }
      catch (utf8::invalid_utf8&)
      {
        // report the error to the console (should not happen)
        // implementors should make sure to provide valid utf8
        std::cerr << "plugin path contains invalid utf8" << STRMLF;
      }

    #else

      DIR *dp;
      struct dirent *dirp;
      using namespace StringUtils;
      if((dp  = opendir(path.c_str())) == NULL) return -1;
      while ((dirp = readdir(dp)) != NULL) {
        #if __APPLE__
          if (!endsWithIgnoreCase(dirp->d_name, ".dylib", 6)) continue;
        #else
          if (!endsWithIgnoreCase(dirp->d_name, ".so", 3)) continue;
        #endif
        if (load_plugin(path + dirp->d_name)) ++ loaded;
      }
      closedir(dp);

    #endif
    return loaded;

  }

}
