/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CAPI_COMPILER_HPP
#define SASS_CAPI_COMPILER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"
#include "compiler.hpp"

#include <sass/compiler.h>

namespace Sass {

  // Promote and format error onto compiler with given status, message and traces
  int handle_error(Compiler& compiler, int status,
    const char* what = nullptr, StackTraces* traces = nullptr);

  // Wrap Structured Exceptions for MSVC (void on no MSVC compilers)
  template <class T, typename ...ARGS> void sass_wrap_msvc_exception(
    T& compiler, void (*fn)(T& compiler, ARGS...), ARGS... args);

  // Wrap C++ exceptions and add to logger if any occur
  template <class T, typename ...ARGS> void sass_wrap_exception(
    T& compiler, void (*fn)(T& compiler, ARGS...), ARGS... args);

}

#endif
