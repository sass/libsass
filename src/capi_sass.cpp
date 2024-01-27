/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_sass.hpp"

#include <cstring>
#include "terminal.hpp"
#include "file.hpp"

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Sass;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Change the current working directory
  // LibSass will fetch this once initially
  // Underlying `CWD` is a thread-local var
  void ADDCALL sass_chdir(const char* path)
  {
    if (path != nullptr) {
      set_cwd(File::rel2abs(path, CWD()) + "/");
    }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // LibSass function to print to stderr terminal output
  // This function is able to print a line with colors
  // It translates Unix/ANSI terminal codes to windows
  void ADDCALL sass_print_stderr(const char* message)
  {
    Terminal::print(message, true);
  }

  // LibSass function to print to stdout terminal output
  // This function is able to print a line with colors
  // It translates Unix/ANSI terminal codes to windows
  void ADDCALL sass_print_stdout(const char* message)
  {
    Terminal::print(message, false);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Allocate a memory block on the heap of (at least) [size].
  // Caller must ensure to release acquired memory at some later point via
  // `sass_free_memory`. You need to go through this utility function in
  // case your code and LibSass use different memory manager implementations.
  // See https://stackoverflow.com/questions/1518711/how-does-free-know-how-much-to-free
  void* ADDCALL sass_alloc_memory(size_t size)
  {
    void* ptr = malloc(size);
    if (ptr == NULL) {
      std::cerr << "Out of memory.\n";
      exit(EXIT_FAILURE);
    }
    return ptr;
  }

  // Allocate a new memory block and copy the passed string into it.
  // Caller must ensure to release acquired memory at some later point via
  // `sass_free_c_string`. You need to go through this utility function in
  // case your code and LibSass use different memory manager implementations.
  // See https://stackoverflow.com/questions/1518711/how-does-free-know-how-much-to-free
  char* ADDCALL sass_copy_c_string(const char* string)
  {
    if (string == nullptr) return nullptr;
    size_t len = ::strlen(string) + 1;
    char* cpy = (char*)sass_alloc_memory(len);
    ::memcpy(cpy, string, len);
    return cpy;
  }

  // Deallocate libsass heap memory
  void ADDCALL sass_free_memory(void* ptr)
  {
    if (ptr) free(ptr);
  }

  // Deallocate libsass heap memory
  void ADDCALL sass_free_c_string(char* ptr)
  {
    if (ptr) free(ptr);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Get compiled libsass version (hard-coded)
  const char* ADDCALL libsass_version(void)
  {
    return LIBSASS_VERSION;
  }

  // Return compiled libsass language (hard-coded)
  const char* ADDCALL libsass_language_version(void)
  {
    return LIBSASS_LANGUAGE_VERSION;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
