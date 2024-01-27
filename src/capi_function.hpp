/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CAPI_FUNCTION_HPP
#define SASS_CAPI_FUNCTION_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <sass/function.h>

// Struct for custom functions
struct SassFunction {

  // The C function to be invoked
  SassFunctionLambda lambda;

  // Signature of function arguments
  sass::string signature;

  // Arbitrary data cookie
  void* cookie;

};

#endif
