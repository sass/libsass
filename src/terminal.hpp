/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
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

  // Query number of available console columns
  // Useful to shorten our output to fit nicely
  short getColumns(bool error = false);

  // Check if we are actually printing to the console
  // In all other cases we want monochrome ASCII output
  bool isConsoleAttached(bool error = false);

  // Check that we print to a terminal with unicode support
  bool hasUnicodeSupport(bool error = false);

  // Check that we print to a terminal with color support
  bool hasColorSupport(bool error = false);

  // This function is able to print a line with colors
  // It translates the ANSI terminal codes to windows
  void print(const char* output, bool error = false);

  // Count number of printable bytes/characters
  size_t count_printable(const char* string);

}

#endif
