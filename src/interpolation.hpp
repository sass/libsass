/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_INTERPOLATION_H
#define SASS_INTERPOLATION_H

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"
#include "string_utils.hpp"
#include "scanner_string.hpp"
#include "utf8/checked.h"
#include "ast_values.hpp"

namespace Sass {

  class StringBuffer {

  public:

    sass::string buffer;

    StringBuffer() :
      buffer()
    {}

    void writeCharCode(uint32_t character)
    {
      utf8::append(character, std::back_inserter(buffer));
    }

    void write(const ItplString* string)
    {
      buffer += string->text();
    }

    void write(unsigned char character)
    {
      buffer.push_back(character);
    }

    void write(char character)
    {
      buffer.push_back(character);
    }

    void write(const sass::string& text)
    {
      buffer.append(text);
    }

    void write(const sass::string& text, const SourceSpan& pstate)
    {
      buffer.append(text);
    }

    void write(sass::string&& text)
    {
      buffer.append(std::move(text));
    }

    void write(sass::string&& text, const SourceSpan& pstate)
    {
      buffer.append(std::move(text));
    }

    bool empty() const
    {
      return buffer.empty();
    }

    void clear()
    {
      buffer.clear();
    }

  };
  // EO StringBuffer

  class InterpolationBuffer {

  private:

    SourceSpan pstate;

    sass::vector<InterpolantObj> contents;

  public:

    StringBuffer text;

    InterpolationBuffer(const SourceSpan& pstate) :
      pstate(pstate), text()
    {}

    InterpolationBuffer(const StringScanner& scanner) :
      pstate(scanner.rawSpan()), text()
    {}

    // Create new interpolation object from the interpolation buffer
    Interpolation* getInterpolation(const SourceSpan& pstate, bool rtrim = false);

    bool empty() const
    {
      return contents.empty() && text.empty();
    }

    // Empties this buffer.
    void clear() {
      contents.clear();
    }

  private:

    // Flushes [_text] to [_contents] if necessary.
    void flushText();

  public:

    // Add an interpolation to the buffer. 
    void addInterpolation(const InterpolationObj schema);

    void writeCharCode(uint32_t character)
    {
      text.writeCharCode(character);
    }

    void write(unsigned char character)
    {
      text.write(character);
    }

    void write(char character)
    {
      text.write(character);
    }

    void write(const char str[])
    {
      text.write(sass::string(str));
    }

    void write(const sass::string& str)
    {
      text.write(str);
    }

    void write(const sass::string& str, SourceSpan pstate)
    {
      text.write(str, pstate);
    }

    void write(sass::string&& str)
    {
      text.write(std::move(str));
    }

    void write(sass::string&& str, SourceSpan pstate)
    {
      text.write(std::move(str), pstate);
    }

    void write(const ItplString* str)
    {
      text.write(str->text(), str->pstate());
    }

    void write(const InterpolantObj& expression)
    {
      flushText();
      contents.emplace_back(expression);
    }

    void add(const InterpolantObj& expression)
    {
      flushText();
      contents.emplace_back(expression);
    }

    sass::string trailingString()
    {
      return text.buffer;
    }

    bool trailingStringEndsWith(const sass::string& cmp)
    {
      sass::string tail(text.buffer);
      // ToDo: trim shouldn't affect srcmap?
      StringUtils::makeRightTrimmed(tail);
      return StringUtils::endsWith(tail, cmp);
    }

  };
  // EO InterpolationBuffer

}

#endif
