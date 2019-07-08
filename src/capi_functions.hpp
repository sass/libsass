#ifndef SASS_SASS_FUNCTIONS_HPP
#define SASS_SASS_FUNCTIONS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// Struct to hold custom function callback
struct SassFunction {
  char* signature;
  SassFunctionLambda function;
  void* cookie;
};

class SourceDataObj;

// Struct to hold importer callback
struct SassImporter {
  SassImporterLambda importer;
  double           priority;
  void*            cookie;
};

#endif
