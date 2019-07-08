/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_SASS_ERROR_HPP
#define SASS_SASS_ERROR_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// #include "sass/base.h"
#include "backtrace.hpp"
#include "source_span.hpp"
#include "capi_functions.hpp"

struct SassTraces;

struct SassError {

public:

  // Error status
  int status;

  // Specific error message
  sass::string what;

  // Traces leading up to error
  Sass::StackTraces traces;

  // Streams from logger
  // Also when status is 0

  // sass::string messages86;

  // Contains all @debug and deprecation warnings
  // Must be all in one to keep the output order
  // sass::string warnings86;

  sass::string formatted;

  // Constructor
  SassError() :
    status(0)
  {}

  char* getJson(bool include_sources) const;

};

#endif
