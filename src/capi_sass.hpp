// must be the first include in all compile units
#ifndef SASS_SASS_H
#define SASS_SASS_H

// #define DEBUG_SHARED_PTR

// Undefine extensions macro to tell sys includes
// that we do not want any macros to be exported
// mainly fixes an issue on SmartOS (SEC macro)
#undef __EXTENSIONS__

#ifdef _MSC_VER
#pragma warning(disable : 4005)
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
#ifndef FS_CASE_SENSITIVE
# ifdef _WIN32
#  define FS_CASE_SENSITIVE 0
# else
#  define FS_CASE_SENSITIVE 1
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

// include C-API header
#include "sass/base.h"

// Include allocator
#include "memory.hpp"

// Include random seed
#include "randomize.hpp"

// #include "../../parallel-hashmap/parallel_hashmap/phmap.h"
// #include "../../ordered-map/include/tsl/ordered_set.h"
// #include "robin_hood.hpp"

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

  // helper to aid dreaded MSVC debug mode
  // see implementation for more details
  char* sass_copy_string(sass::string str);

}


// simple linked list
//struct string_list {
//  string_list* next;
//  char* string;
//};

// sass config options structure
struct Sass_Inspect_Options {

  // Output style for the generated CSS code
  // A value from above SASS_STYLE_* constants
  enum SassOutputStyle output_style;

  // Precision for fractional numbers
  int precision;

  // initialization list (constructor with defaults)
  Sass_Inspect_Options(SassOutputStyle style = SASS_STYLE_NESTED,
                       int precision = 10)
  : output_style(style), precision(precision)
  { }

};

// sass config options structure
struct SassOutputOptionsCpp : Sass_Inspect_Options {

  // String to be used for indentation
  const char* indent;
  // String to be used to for line feeds
  const char* linefeed;

  // Emit comments in the generated CSS indicating
  // the corresponding source line.
  bool source_comments;

  // initialization list (constructor with defaults)
  SassOutputOptionsCpp(struct Sass_Inspect_Options& opt,
                      const char* indent = "  ",
                      const char* linefeed = "\n",
                      bool source_comments = false)
  : Sass_Inspect_Options(opt),
    indent(indent), linefeed(linefeed),
    source_comments(source_comments)
  { }

  // initialization list (constructor with defaults)
  SassOutputOptionsCpp(SassOutputStyle style = SASS_STYLE_NESTED,
                      int precision = 10,
                      const char* indent = "  ",
                      const char* linefeed = "\n",
                      bool source_comments = false)
  : Sass_Inspect_Options(style, precision),
    indent(indent), linefeed(linefeed),
    source_comments(source_comments)
  { }

};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

namespace Sass {

  template <typename T>
  T clamp(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
  }

  template <typename T>
  T absmod(const T& n, const T& r) {
    T m = std::fmod(n, r);
    if (m < 0.0) m += r;
    return m;
  }

  double round32(double val, int precision = SassDefaultPrecision);
  
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
// #define SASS_ASSERT(cond, msg) ((void)0)
#define SASS_ASSERT(cond, msg) assert(cond && msg)
#else
#ifdef DEBUG
#define SASS_ASSERT(cond, msg) assert(cond && msg)
#else
#define SASS_ASSERT(cond, msg) ((void)0)
#endif
#endif

#endif
