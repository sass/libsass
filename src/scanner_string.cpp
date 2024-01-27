/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "scanner_string.hpp"

#include "charcode.hpp"
#include "character.hpp"
#include "exceptions.hpp"
#include "utf8/checked.h"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  StringScanner::StringScanner(
    Logger& logger,
    SourceDataObj source) :
    source(source),
    startpos(source->contentStart()),
    endpos(source->contentEnd()),
    position(source->contentStart()),
    sourceUrl(source->getAbsPath()),
    srcid(source->getSrcIdx()),
    offset(),
    relevant(),
    logger(logger)
  {
    // consume BOM?

    // This can use up to 3% runtime (mostly under 1%)
    auto invalid = utf8::find_invalid(startpos, endpos);
    if (invalid != endpos) {
      SourceSpan pstate(source);
      Offset start(startpos, invalid);
      pstate.position.line = start.line;
      pstate.position.column = start.column;
      throw Exception::InvalidUnicode(pstate, {});
    }
  }

  // Whether the scanner has completely consumed [string].
  bool StringScanner::isDone() const
  {
    return position >= endpos;
  }

  // Called whenever a character is consumed.
  // Used to update scanner line/column position.
  void StringScanner::consumedChar(uint8_t character)
  {
    // std::cerr << "COnsumed [" << character << "]\n";
    switch (character) {
    case $space:
    case $tab:
    case $vt:
    case $ff:
    case $cr:
      offset.column += 1;
      break;
    case $lf:
      offset.line += 1;
      offset.column = 0;
      break;
    default:
      // skip over 10xxxxxx and 01xxxxxx
      // count ASCII and first utf8 bytes
      if (Character::isCharacter(character)) {
        // 64 => first utf8 byte
        // 128 => regular ASCII char
        offset.column += 1;
        // sync relevant position
        relevant = offset;
      }
      break;
    }
  }


  // Consumes a single character and returns its character
  // code. This throws a [FormatException] if the string has
  // been fully consumed. It doesn't affect [lastMatch].
  uint8_t StringScanner::readChar()
  {
    if (isDone()) _fail("more input");
    uint8_t ascii = *position;
    consumedChar(ascii);
    position += 1;
    return ascii;
  }

  // Returns the character code of the character [offset] away
  // from [position]. [offset] defaults to zero, and may be negative
  // to inspect already-consumed characters. This returns `null` if
  // [offset] points outside the string. It doesn't affect [lastMatch].
  uint8_t StringScanner::peekChar(size_t offset) const
  {
    const char* cur = position + offset;
    if (cur < startpos || cur >= endpos) {
      return 0;
    }
    return *cur;
  }

  // Same as above, but stores next char into passed variable.
  bool StringScanner::peekChar(uint8_t& chr, size_t offset) const
  {
    const char* cur = position + offset;
    if (cur < startpos || cur >= endpos) {
      return false;
    }
    chr = *cur;
    return true;
  }

  // If the next character in the string is [character], consumes it.
  // Returns whether or not [character] was consumed.
  bool StringScanner::scanChar(uint8_t character)
  {
    if (isDone()) return false;
    uint8_t ascii = *position;
    if (character != ascii) return false;
    consumedChar(character);
    position += 1;
    return true;
  }

  // If the next character in the string is [character], consumes it.
  // If [character] could not be consumed, throws a [FormatException]
  // describing the position of the failure. [name] is used in this
  // error as the expected name of the character being matched; if
  // it's empty, the character itself is used instead.
  void StringScanner::expectChar(uint8_t character, const sass::string& name, bool advance)
  {
    if (!scanChar(character)) {
      if (advance && !isDone()) {
        relevant = offset;
      }
      if (name.empty()) {
        if (character == $quote) {
          _fail("\"\\\"\"");
        }
        else {
          sass::string msg("\"");
          msg += character;
          _fail(msg + "\"");
        }
      }
      _fail(name);
    }
  }

  // If [pattern] matches at the current position of the string, scans forward
  // until the end of the match. Returns whether or not [pattern] matched.
  bool StringScanner::scan(const sass::string& pattern)
  {
    const char* cur = position;
    for (uint8_t code : pattern) {
      if (isDone()) return false;
      uint8_t ascii = *cur;
      if (ascii != code) return false;
      consumedChar(ascii);
      cur += 1;
    }
    position = cur;
    return true;
  }

  // If [pattern] matches at the current position of the string, scans
  // forward until the end of the match. If [pattern] did not match,
  // throws a [FormatException] describing the position of the failure.
  // [name] is used in this error as the expected name of the pattern
  // being matched; if it's `null`, the pattern itself is used instead.
  void StringScanner::expect(const sass::string& pattern, const sass::string& name)
  {
    if (!scan(pattern)) {
      if (name.empty()) {
        _fail(pattern);
      }
      _fail(name);
    }
  }

  // If the string has not been fully consumed,
  // this throws a [FormatException].
  void StringScanner::expectDone()
  {
    if (isDone()) return;
    SourceSpan span(rawSpan());
    callStackFrame frame(logger, span);
    throw Exception::ParserException(
      logger, "expected no more input.");
  }

  // Returns whether or not [pattern] matches at the current position
  // of the string. This doesn't move the scan pointer forward.
  bool StringScanner::matches(const sass::string& pattern)
  {
    const char* cur = position;
    for (char chr : pattern) {
      if (chr != *cur) {
        return false;
      }
      cur += 1;
    }
    return true;
  }

  // Returns the substring of [string] between [start] and [end].
  // Unlike [String.substring], [end] defaults to [position]
  // rather than the end of the string.
  sass::string StringScanner::substring(const char* start, const char* end)
  {
    if (end == nullptr) end = position;
    return sass::string(start, end);
  }

  // Throws a [FormatException] describing that [name] is
  // expected at the current position in the string.
  void StringScanner::_fail(
    const sass::string& name) const
  {
    SourceSpan span(relevantSpan());
    callStackFrame frame(logger, span);
    sass::string msg("expected " + name + ".");
    throw Exception::ParserException(logger, msg);
  }

  // Throws a [FormatException] with [traces] and [pstate].
  void StringScanner::error(
    const sass::string& message,
    const BackTraces& traces,
    const SourceSpan& pstate) const
  {
    callStackFrame frame(logger, pstate);
    throw Exception::ParserException(traces, message);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool StringScanner::hasLineBreak(
    const char* before) const
  {
    const char* cur = before;
    while (cur < endpos) {
      if (*cur == '\r') return true;
      if (*cur == '\n') return true;
      cur += 1;
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
