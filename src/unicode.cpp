/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "unicode.hpp"

namespace Sass {
  namespace Unicode {

    // naming conventions:
    // bytes: raw byte offset (0 based)
    // position: code point offset (0 based)

    // Return number of code points in utf8 string
    size_t codePointCount(const sass::string& utf8) {
      return utf8::distance(utf8.begin(), utf8.end());
    }
    // EO codePointCount

    // Return number of code points in utf8 string up to bytes offset.
    size_t codePointCount(const sass::string& utf8, size_t bytes) {
      return utf8::distance(utf8.begin(), utf8.begin() + bytes);
    }
    // EO codePointCount

    // Return the byte offset at a code point position
    size_t byteOffsetAtPosition(const sass::string& utf8, size_t position) {
      sass::string::const_iterator it = utf8.begin();
      utf8::advance(it, position, utf8.end());
      return std::distance(utf8.begin(), it);
    }
    // EO byteOffsetAtPosition

    // Returns utf8 aware substring.
    // Parameters are in code points.
    sass::string substr(
      sass::string& utf8,
      size_t start,
      size_t len)
    {
      auto first = utf8.begin();
      utf8::advance(first,
        start, utf8.end());
      auto last = first;
      if (len != sass::string::npos) {
        utf8::advance(last,
          len, utf8.end());
      }
      else {
        last = utf8.end();
      }
      return sass::string(
        first, last);
    }
    // EO substr

    // Utf8 aware string replacement.
    // Parameters are in code points.
    // Inserted text must be valid utf8.
    sass::string replace(
      sass::string& text,
      size_t start, size_t len,
      const sass::string& insert)
    {
      auto first = text.begin();
      utf8::advance(first,
        start, text.end());
      auto last = first;
      if (len != sass::string::npos) {
        utf8::advance(last,
          len, text.end());
      }
      else {
        last = text.end();
      }
      return text.replace(
        first, last,
        insert);
    }
    // EO replace

    #ifdef _WIN32

    // utf16 functions
    using std::wstring;

    // convert from utf16/wide string to utf8 string
    sass::string utf16to8(const sass::wstring& utf16)
    {
      sass::string utf8;
      // preallocate expected memory
      utf8.reserve(sizeof(utf16)/2);
      utf8::utf16to8(utf16.begin(), utf16.end(),
                     back_inserter(utf8));
      return utf8;
    }

    // convert from utf8 string to utf16/wide string
    sass::wstring utf8to16(const sass::string& utf8)
    {
      sass::wstring utf16;
      // preallocate expected memory
      utf16.reserve(codePointCount(utf8)*2);
      utf8::utf8to16(utf8.begin(), utf8.end(),
                     back_inserter(utf16));
      return utf16;
    }

    #endif

  }
}
