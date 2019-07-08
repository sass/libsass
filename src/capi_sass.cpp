// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <cstring>
#include "source.hpp"
#include "compiler.hpp"
#include <sass/version.h>

namespace Sass {
extern "C" {

  // Allocate a memory block on the heap of (at least) [size].
  // Make sure to release to acquired memory at some later point via
  // `sass_free_memory`. You need to go through my utility function in
  // case your code and my main program don't use the same memory manager.
  void* ADDCALL sass_alloc_memory(size_t size)
  {
    void* ptr = malloc(size);
    if (ptr == NULL) {
      std::cerr << "Out of memory.\n";
      exit(EXIT_FAILURE);
    }
    return ptr;
  }

  // Allocate a memory block on the heap and copy [string] into it.
  // Make sure to release to acquired memory at some later point via
  // `sass_free_memory`. You need to go through my utility function in
  // case your code and my main program don't use the same memory manager.
  char* ADDCALL sass_copy_c_string(const char* string)
  {
    if (string == nullptr) return nullptr;
    size_t len = ::strlen(string) + 1;
    char* cpy = (char*)sass_alloc_memory(len);
    ::memcpy(cpy, string, len);
    return cpy;
  }

  // Deallocate libsass heap memory
  void ADDCALL sass_free_c_string(char* ptr)
  {
    if (ptr) free(ptr);
  }

  // Deallocate libsass heap memory
  void ADDCALL sass_free_memory(void* ptr)
  {
    if (ptr) free(ptr);
  }

  char* ADDCALL sass_compiler_find_include (const char* file, struct SassCompiler* compiler)
  {
    std::cerr << "YEP, find include\n";
    /*
    // get the last import entry to get current base directory
    SassImportPtr import = sass_compiler_get_last_import(compiler);
    const sass::vector<sass::string>& incs = compiler->cpp_ctx->includePaths;
    // create the vector with paths to lookup
    sass::vector<sass::string> paths(1 + incs.size());
    paths.emplace_back(File::dir_name(import->srcdata->getAbsPath()));
    paths.insert( paths.end(), incs.begin(), incs.end() );
    // now resolve the file path relative to lookup paths
    sass::string resolved(File::find_include(file,
      compiler->cpp_ctx->CWD, paths, compiler->cpp_ctx->fileExistsCache));
    return sass_copy_c_string(resolved.c_str());
    */
    return 0;
  }

  char* ADDCALL sass_compiler_find_file (const char* file, struct SassCompiler* compiler)
  {
    sass::string path(file);
    path = Compiler::unwrap(compiler).findFile(path);
    return path.empty() ? nullptr : sass_copy_c_string(path.c_str());
  }

  // Get compiled libsass version
  const char* ADDCALL libsass_version(void)
  {
    return LIBSASS_VERSION;
  }

  // Get compiled libsass version
  const char* ADDCALL sass2scss_version(void)
  {
    return "obsolete";
  }

  // Get compiled libsass version
  const char* ADDCALL libsass_language_version(void)
  {
    return LIBSASS_LANGUAGE_VERSION;
  }

}
}

namespace Sass {

  double round32(double val, int precision)
  {
    // https://github.com/sass/sass/commit/4e3e1d5684cc29073a507578fc977434ff488c93
    if (std::fmod(val, 1) - 0.5 > -std::pow(0.1, precision + 1)) return std::ceil(val);
    else if (std::fmod(val, 1) - 0.5 > std::pow(0.1, precision)) return std::floor(val);
    // Work around some compiler issue
    // Cygwin has it not defined in std
    using namespace std;
    return ::round(val);
  }

  // helper to aid dreaded MSVC debug mode
  char* sass_copy_string(sass::string str)
  {
    // In MSVC the following can lead to segfault:
    // sass_copy_c_string(stream.str().c_str());
    // Reason is that the string returned by str() is disposed before
    // sass_copy_c_string is invoked. The string is actually a stack
    // object, so indeed nobody is holding on to it. So it seems
    // perfectly fair to release it right away. So the const char*
    // by c_str will point to invalid memory. I'm not sure if this is
    // the behavior for all compiler, but I'm pretty sure we would
    // have gotten more issues reported if that would be the case.
    // Wrapping it in a functions seems the cleanest approach as the
    // function must hold on to the stack variable until it's done.
    return sass_copy_c_string(str.c_str());
  }

}
