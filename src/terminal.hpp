#ifndef SASS_TERMINAL_HPP
#define SASS_TERMINAL_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#ifdef _WIN32
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <wincon.h>
#endif

#include "constants.hpp"

// Minimal terminal abstraction for cross compatibility.
// Its main purpose is to let us print stuff with colors.
namespace Terminal {

  // Import constant terminal color definition
  using namespace Sass::Constants::Terminal;

  short getColumns(bool error = false);

  // Check if we are actually printing to the console
  // In all other cases we want monochrome ASCII output
  bool isConsoleAttached(bool error = false);
  bool hasUnicodeSupport(bool error = false);
  bool hasColorSupport(bool error = false);

  void print(const char* output, bool error = false);


#ifdef _WIN32
  /*
  */
#endif

}

#endif
