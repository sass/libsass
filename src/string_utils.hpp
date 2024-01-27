/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_STRING_UTILS_HPP
#define SASS_STRING_UTILS_HPP

// sass.hpp must go before all system headers
// to get the  __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "character.hpp"

namespace Sass {
  namespace StringUtils {

    // Standard string utility functions (names are self-explanatory)
    bool endsWith(const sass::string& str, const sass::string& suffix);
    bool startsWith(const sass::string& str, const sass::string& prefix);
    bool equalsIgnoreCase(const sass::string& a, const sass::string& b);
    bool endsWithIgnoreCase(const sass::string& str, const sass::string& suffix);
    bool startsWithIgnoreCase(const sass::string& str, const sass::string& prefix);

    // Optimized versions when you pass `const char*` you know is already lowercase.
    bool endsWith(const sass::string& str, const char* suffix, size_t len);
    bool startsWith(const sass::string& str, const char* prefix, size_t len);
    bool equalsIgnoreCase(const sass::string& a, const char* b, size_t len);
    bool endsWithIgnoreCase(const sass::string& str, const char* suffix, size_t len);
    bool startsWithIgnoreCase(const sass::string& str, const char* prefix, size_t len);

    // Make the passed string whitespace trimmed.
    void makeTrimmed(sass::string& str);

    // Trim the left side of passed string.
    void makeLeftTrimmed(sass::string& str);

    // Trim the right side of passed string.
    void makeRightTrimmed(sass::string& str);

    // Make the passed string lowercase.
    void makeLowerCase(sass::string& str);

    // Make the passed string uppercase.
    void makeUpperCase(sass::string& str);

    // Return new string converted to lowercase.
    sass::string toUpperCase(const sass::string& str);

    // Return new string converted to uppercase.
    sass::string toLowerCase(const sass::string& str);

    // Check if string contains white-space only
    // Returns true if string to check is empty
    bool isWhitespaceOnly(const sass::string& str);

    // Replace all occurrences of `search` in string `str` with `replacement`.
    void makeReplace(sass::string& str, const sass::string& search, const sass::string& replacement);

    // Return list of strings split by `delimiter`.
    // Optionally `trim` all results (default behavior).
    sass::vector<sass::string> split(sass::string str, char delimiter, bool trim = true);

    // Return joined string from all passed strings, delimited by separator.
    sass::string join(const sass::vector<sass::string>& strings, const char* separator);

    // Returns [name] without a vendor prefix.
    // If [name] has no vendor prefix, it's returned as-is.
    sass::string unvendor(const sass::string& name);

  }
}

#endif
