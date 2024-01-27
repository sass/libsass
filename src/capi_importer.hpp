/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CAPI_IMPORTER_HPP
#define SASS_CAPI_IMPORTER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <sass/importer.h>

// Struct to hold importer callback
struct SassImporter {

  // The C function to be invoked
  SassImporterLambda lambda;

  // Invocation priority (order)
  double priority;

  // Arbitrary data cookie
  void* cookie;

};

#endif
