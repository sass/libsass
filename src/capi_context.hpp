#ifndef SASS_SASS_CONTEXT_HPP
#define SASS_SASS_CONTEXT_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "sass/base.h"
#include "sass/context.h"
#include "source_map.hpp"

// Case 1: create no source-maps
// Case 2: create source-maps, but no reference in css
// Case 3: create source-maps, reference to file in css
// Case 4: create source-maps, embed the json in the css
// Note: Writing source-maps to disk depends on implementor

struct SassSrcMapOptions {

  enum SassSrcMapMode mode;

  // Flag to embed full sources
  // Ignored for SASS_SRCMAP_NONE
  bool embed_contents;

  // create file URLs for sources
  bool file_urls;

  // Directly inserted in source maps
  sass::string root;

  // Path where source map is saved
  sass::string path;

  // Path to file that loads us
  sass::string origin;

  // Init everything to false
  SassSrcMapOptions() :
    mode(SASS_SRCMAP_NONE),
    embed_contents(true),
    file_urls(false)
  {}

};

#endif
