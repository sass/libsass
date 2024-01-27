/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_BASE_H
#define SASS_BASE_H

#ifdef _MSC_VER
  #pragma warning(disable : 4503)
  #ifndef _SCL_SECURE_NO_WARNINGS
    #define _SCL_SECURE_NO_WARNINGS
  #endif
  #ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
  #endif
  #ifndef _CRT_NONSTDC_NO_DEPRECATE
    #define _CRT_NONSTDC_NO_DEPRECATE
  #endif
#endif

// Work around lack of `noexcept` keyword support in VS2013
#if defined(_MSC_VER) && (_MSC_VER <= 1800) && !defined(_ALLOW_KEYWORD_MACROS)
#define _ALLOW_KEYWORD_MACROS 1
#define noexcept throw( )
#endif

// Load some POD types
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Include forward declarations
#include <sass/fwdecl.h>

// Include enumerations
#include <sass/enums.h>

#ifdef __GNUC__
  #define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
  #define DEPRECATED(func) __declspec(deprecated) func
#else
  #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
  #define DEPRECATED(func) func
#endif

#ifdef _WIN32

/* You should define ADD_EXPORTS *only* when building the DLL. */
  #ifdef ADD_EXPORTS
    #define ADDAPI __declspec(dllexport)
    #define ADDCALL __cdecl
  #else
    #define ADDAPI
    #define ADDCALL
  #endif

#else /* _WIN32 not defined. */

  /* Define with no value on non-Windows OSes. */
  #define ADDAPI
  #define ADDCALL

#endif

#ifdef __cplusplus
extern "C" {
#endif

  // Change the virtual current working directory
  ADDAPI void ADDCALL sass_chdir(const char* path);

  // Prints message to stderr with color for windows
  ADDAPI void ADDCALL sass_print_stdout(const char* message);
  ADDAPI void ADDCALL sass_print_stderr(const char* message);

  // Return implemented sass language version
  ADDAPI const char* ADDCALL libsass_version(void);

  // Return the compiled libsass language (hard-coded)
  // This is hard-coded with the library on compilation!
  ADDAPI const char* ADDCALL libsass_language_version(void);

  // Allocate a memory block on the heap of (at least) [size].
  // Make sure to release to acquired memory at some later point via
  // `sass_free_memory`. You need to go through my utility function in
  // case your code and my main program don't use the same memory manager.
  ADDAPI void* ADDCALL sass_alloc_memory(size_t size);

  // Allocate a memory block on the heap and copy [string] into it.
  // Make sure to release to acquired memory at some later point via
  // `sass_free_memory`. You need to go through my utility function in
  // case your code and my main program don't use the same memory manager.
  ADDAPI char* ADDCALL sass_copy_c_string(const char* str);

  // Deallocate libsass heap memory
  ADDAPI void ADDCALL sass_free_memory(void* ptr);
  ADDAPI void ADDCALL sass_free_c_string(char* ptr);

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
