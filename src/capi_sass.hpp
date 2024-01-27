/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CAPI_SASS_HPP
#define SASS_CAPI_SASS_HPP

// This include must be the first in all compile units!
// Otherwise you may run into issues with other headers!

// #define DEBUG_SHARED_PTR

// Undefine extensions macro to tell sys includes
// that we do not want any macros to be exported
// mainly fixes an issue on SmartOS (SEC macro)
#undef __EXTENSIONS__

#ifdef _MSC_VER
#pragma warning(disable : 26812)
#endif

// applies to MSVC and MinGW
#ifdef _WIN32
// we do not want the ERROR macro
# ifndef NOGDI
#  define NOGDI
# endif
// we do not want the min/max macro
# ifndef NOMINMAX
#  define NOMINMAX
# endif
// we do not want the IN/OUT macro
# ifndef _NO_W32_PSEUDO_MODIFIERS
#  define _NO_W32_PSEUDO_MODIFIERS
# endif
#endif

// should we be case insensitive
// when dealing with files or paths
#ifndef FS_CASE_SENSITIVITY
# ifdef _WIN32
#  define FS_CASE_SENSITIVITY 0
# else
#  define FS_CASE_SENSITIVITY 1
# endif
#endif

// path separation char
#ifndef PATH_SEP
# ifdef _WIN32
#  define PATH_SEP ';'
# else
#  define PATH_SEP ':'
# endif
#endif

// OS specific line feed
// since std::endl flushes
#ifndef STRMLF
# ifdef _WIN32
#  define STRMLF '\n'
# else
#  define STRMLF '\n'
# endif
#endif

// Include C-API headers
#include "sass/base.h"
#include "sass/version.h"

// Include allocator
#include "memory.hpp"

// Include random seed
#include "randomize.hpp"

// Include unordered map implementation
#ifdef USE_TSL_HOPSCOTCH
#include "tessil/hopscotch_map.h"
#include "tessil/hopscotch_set.h"
#define UnorderedMap tsl::hopscotch_map
#define UnorderedSet tsl::hopscotch_set
#else
#include <unordered_map>
#include <unordered_set>
#define UnorderedMap std::unordered_map
#define UnorderedSet std::unordered_set
#endif

// Always use tessil implementation
#include "tessil/ordered_map.h"
#define OrderedMap tsl::ordered_map

// Small helper to avoid typing
#define NPOS std::string::npos

// output behaviors
namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  const double PI = std::acos(-1);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // sass inspect options
  class InspectOptions
  {
  public:

    // Change default input syntax for entry point
    // Only applied if entry point has AUTO syntax
    enum SassImportSyntax input_syntax;

    // Output style for the generated CSS code
    // A value from above SASS_STYLE_* constants
    enum SassOutputStyle output_style;

    // Precision for fractional numbers
    int precision;

    // Number format for sprintf.
    // Cached to speed up output.
    char nr_sprintf[32];

    // Update precision and epsilon etc.
    void setPrecision(int precision)
    {
      this->precision = precision;
      // Update sprintf format to match precision
      snprintf(this->nr_sprintf, 32, "%%.%df", precision);
    }

    // initialization list (constructor with defaults)
    InspectOptions(
      enum SassOutputStyle style = SASS_STYLE_NESTED,
      int precision = SassDefaultPrecision) :
      input_syntax(SASS_IMPORT_AUTO),
      output_style(style),
      precision(precision)
    {
      // Update sprintf format to match precision
      snprintf(nr_sprintf, 32, "%%.%df", precision);
    }

  };
  // EO class InspectOptions

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // sass source-map options
  class SrcMapOptions
  {
  public:

    // Case 1: create no source-maps
    // Case 2: create source-maps, but no reference in css
    // Case 3: create source-maps, reference to file in css
    // Case 4: create source-maps, embed the json in the css
    // Note: Writing source-maps to disk depends on implementor
    enum SassSrcMapMode mode;

    // Flag to embed full sources
    // Ignored for SASS_SRCMAP_NONE
    bool embed_contents;

    // create file URLs for sources
    bool file_urls;

    // Flags to enable more details
    bool enable_openers;
    bool enable_closers;

    // Directly inserted in source maps
    sass::string root;

    // Path where source map is saved
    sass::string path;

    // Path to file that loads us
    sass::string origin;

    // Init everything to false
    SrcMapOptions() :
      mode(SASS_SRCMAP_NONE),
      embed_contents(false),
      file_urls(false),
      enable_openers(false),
      enable_closers(false)
    {}

  };
  // EO class SrcMapOptions

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // sass output and inspect options
  class OutputOptions : public InspectOptions
  {
  public:
    // String to be used for indentation
    const char* indent;
    // String to be used to for line feeds
    const char* linefeed;

    // Emit comments in the generated CSS indicating
    // the corresponding source line.
    bool source_comments;

    // Enable to not print anything on stderr (quiet mode)
    bool suppress_stderr = false;

    // Sourcemap related options
    SrcMapOptions mapopt;

    // initialization list (constructor with defaults)
    OutputOptions(const InspectOptions& opt,
      const char* indent = "  ",
      const char* linefeed = "\n",
      bool source_comments = false) :
      InspectOptions(opt),
      indent(indent), linefeed(linefeed),
      source_comments(source_comments)
    { }

    // initialization list (constructor with defaults)
    OutputOptions(SassOutputStyle style = SASS_STYLE_NESTED,
      int precision = SassDefaultPrecision,
      const char* indent = "  ",
      const char* linefeed = "\n",
      bool source_comments = false) :
      InspectOptions(style, precision),
      indent(indent), linefeed(linefeed),
      source_comments(source_comments)
    { }

  };
  // EO class OutputOptions

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // helper to aid dreaded MSVC debug mode
  // see implementation for more details
  inline char* sass_copy_string(sass::string str) {
    // In MSVC the following can lead to segfault:
    // sass_copy_c_string(stream.str().c_str());
    // Reason is that the string returned by str() is disposed before
    // sass_copy_c_string is invoked. The string is actually a stack
    // object, so indeed nobody is holding on to it. So it seems
    // perfectly fair to release it right away. So the const char*
    // by c_str will point to invalid memory. I'm not sure if this is
    // the behavior for all compiler, but I'm pretty sure we would
    // have gotten more issues reported if that would be the case.
    // Wrapping it in a functions seems the cleanest approach as the
    // function must hold on to the stack variable until it's done.
    return sass_copy_c_string(str.c_str());
  }
  // EO sass_copy_string

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#define SASS_ASSERT(cond, msg) ((void)0)
#else
#ifdef DEBUG
#define SASS_ASSERT(cond, msg) assert(cond && msg)
#else
#define SASS_ASSERT(cond, msg) ((void)0)
#endif
#endif

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#endif
