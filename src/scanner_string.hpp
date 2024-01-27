/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_SCANNER_STRING_HPP
#define SASS_SCANNER_STRING_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "source.hpp"
#include "backtrace.hpp"

namespace Sass {

  struct StringScannerState {
    const char* position;
    Offset offset;
  };

  // A class that scans through a string using [Pattern]s.
  class StringScanner {

  public:

    // Value constructor
    StringScanner(
      Logger& logger,
      SourceDataObj source);

    // The source associated with this scanner
    SourceDataObj source;

    // The string being scanned through.
    const char* startpos;

    // The final position to scan to.
    const char* endpos;

    // The current position.
    const char* position;

    // The URL of the source of the string being scanned.
    // This is used for error reporting. It may be `null`,
    // indicating that the source URL is unknown or unavailable.
    const char* sourceUrl;

    // The global id for this input file.
    size_t srcid;

    // The current line/col offset
    Offset offset;

    // Last non whitespace position
    // Used to create parser state spans
    Offset relevant;

    // Attached logger
    Logger& logger;

    // Whether the scanner has completely consumed [string].
    bool isDone() const;

    // Called whenever a character is consumed.
    // Used to update scanner line/column position.
    void consumedChar(uint8_t character);

    // Consumes a single character and returns its character
    // code. This throws a [FormatException] if the string has
    // been fully consumed. It doesn't affect [lastMatch].
    uint8_t readChar();

    // Returns the character code of the character [offset] away
    // from [position]. [offset] defaults to zero, and may be negative
    // to inspect already-consumed characters. This returns `null` if
    // [offset] points outside the string. It doesn't affect [lastMatch].
    uint8_t peekChar(size_t offset = 0) const;

    bool peekChar(uint8_t& chr, size_t offset = 0) const;

    // If the next character in the string is [character], consumes it.
    // Returns whether or not [character] was consumed.
    bool scanChar(uint8_t character);

    // If the next character in the string is [character], consumes it.
    // If [character] could not be consumed, throws a [FormatException]
    // describing the position of the failure. [name] is used in this
    // error as the expected name of the character being matched; if
    // it's `null`, the character itself is used instead.
    void expectChar(uint8_t character, const sass::string& name = Strings::empty, bool advance = true);

    // If [pattern] matches at the current position of the string, scans forward
    // until the end of the match. Returns whether or not [pattern] matched.
    bool scan(const sass::string& pattern);

    // If [pattern] matches at the current position of the string, scans
    // forward until the end of the match. If [pattern] did not match,
    // throws a [FormatException] describing the position of the failure.
    // [name] is used in this error as the expected name of the pattern
    // being matched; if it's `null`, the pattern itself is used instead.
    void expect(const sass::string& pattern, const sass::string& name = Strings::empty);

    // If the string has not been fully consumed,
    // this throws a [FormatException].
    void expectDone();

    // Returns whether or not [pattern] matches at the current position
    // of the string. This doesn't move the scan pointer forward.
    bool matches(const sass::string& pattern);

    // Returns the substring of [string] between [start] and [end].
    // Unlike [String.substring], [end] defaults to [position]
    // rather than the end of the string.
    sass::string substring(const char* start, const char* end = 0);

    // Throws a [FormatException] describing that [name] is
    // expected at the current position in the string.
    void _fail(const sass::string& name) const;

    // Throws a [FormatException] with [traces] and [pstate].
    void error(const sass::string& name,
      const BackTraces& traces,
      const SourceSpan& pstate) const;

    bool hasLineBreak(const char* before) const;

    StringScannerState state() {
      return { position, offset };
    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Backtrack the scanner to the given position
    void backtrack(const StringScannerState& state)
    {
      position = state.position;
      offset.line = state.offset.line;
      offset.column = state.offset.column;
      // We assume we always store states
      // only from relevant positions.
      relevant = offset;
    }

    // Get a source span pointing to raw position
    // Raw means whitespace may already be consumed
    inline SourceSpan spanAt(Offset& start) const
    {
      SourceSpan pstate(source, start);
      return source ? source->adjustSourceSpan(pstate) : pstate;
    }

    // Get a source span pointing to raw position
    // Raw means whitespace may already be consumed
    inline SourceSpan rawSpan() const
    {
      SourceSpan pstate(source, offset);
      return source ? source->adjustSourceSpan(pstate) : pstate;
    }

    // Get a source span pointing to last relevant position
    // Last relevant means whitespace is not yet parsed (word ending)
    inline SourceSpan relevantSpan() const // 161
    {
      SourceSpan pstate(source, relevant);
      return source ? source->adjustSourceSpan(pstate) : pstate;
    }

    // Create a source span from start to raw position
    // Raw means whitespace may already be consumed
    inline SourceSpan rawSpanFrom(const Offset& start) // 53
    {
      SourceSpan pstate(source, start, Offset::distance(start, offset));
      return source ? source->adjustSourceSpan(pstate) : pstate;
    }

    // Create a source span from start to last relevant position
    // Last relevant means whitespace is not yet parsed (word ending)
    inline SourceSpan relevantSpanFrom(const Offset& start) // 161
    {
      SourceSpan pstate(source, start, Offset::distance(start, relevant));
      return source ? source->adjustSourceSpan(pstate) : pstate;
    }

    inline SourceSpan relevantSpanFrom(const Offset& start, size_t delta) // 161
    {
      SourceSpan pstate(source, start, Offset::distance(start, relevant));
      return source ? source->adjustSourceSpan(pstate) : pstate;
    }

  };

}

#endif
