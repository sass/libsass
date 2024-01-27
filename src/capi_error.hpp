/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CAPI_ERROR_HPP
#define SASS_CAPI_ERROR_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <sass/error.h>
#include "backtrace.hpp"

// Struct for errors
struct SassError {

  // Error status
  int status;

  // Error message
  sass::string what;

  // Traces leading up to error
  Sass::StackTraces traces;

  // Formatted error string, may contain
  // unicode and/or ANSI color escape codes
  sass::string formatted;

  // Constructor
  SassError() :
    status(0)
  {}

  // Return json string to pass down-stream.
  // You must free the returned data yourself.
  // Do so by calling `sass_free_memory(ptr)`.
  char* getJson(bool include_sources) const;

  // Getter for error status as css
  // In order to show error in browser.
  char* getCss() const;

  // Write error style-sheet so errors are shown
  // in the browser if the stylesheet is loaded.
  void writeCss(std::ostream& css) const;

};

#endif
