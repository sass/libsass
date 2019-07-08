/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_HPP
#define SASS_PARSER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// Parser has a great chance for stack overflow
// To fix this once and for all we need a different approach
// http://lambda-the-ultimate.org/node/1599
// Basically we should not call into recursion, but rather
// return some continuation bit, of course we still need to
// add our ast nodes somewhere (instead of returning as now)

#include "interpolation.hpp"
#include "scanner_string.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class Parser
  {
  public:

    // Compiler context
    Compiler& context;

    // The scanner that scans through the text being parsed.
    StringScanner scanner;

  protected:

    // Value constructor
    Parser(
      Compiler& context,
      SourceDataObj source);

    friend class ExpressionParser;

    // Original source mappings. If defined, SourceSpans will
    // point to where the original source has come from. 
    // sass::vector<Mapping> srcmap;

    // Points to context.varStack
    sass::vector<EnvFrame*>& varStack;

    // The silent comment this parser encountered previously.
    SilentCommentObj lastSilentComment;

  protected:

    // Returns whether [text] is a valid CSS identifier.
    bool isIdentifier(sass::string text);

    // Consumes whitespace, including any comments.
    virtual void scanWhitespace()
    {
      do {
        scanWhitespaceWithoutComments();
      } while (scanComment());
    }

    // Consumes whitespace, but not comments.
    inline void scanWhitespaceWithoutComments()
    {
      while (!scanner.isDone() && Character::isWhitespace(scanner.peekChar())) {
        scanner.readChar();
      }
    }

    // Consumes spaces and tabs.
    inline void scanSpaces()
    {
      while (!scanner.isDone() && Character::isSpaceOrTab(scanner.peekChar())) {
        scanner.readChar();
      }
    }

    // Consumes and ignores a comment if possible.
    // Returns whether the comment was consumed.
    bool scanComment();

    // Consumes and ignores a silent (Sass-style) comment.
    virtual SilentComment* readSilentComment();

    // Consumes and ignores a loud (CSS-style) comment.
    void loudComment();

    // Consumes a plain CSS identifier. If [unit] is `true`, this 
    // doesn't parse a `-` followed by a digit. This ensures that 
    // `1px-2px` parses as subtraction rather than the unit `px-2px`.
    sass::string readIdentifier(bool unit = false);

    // Consumes a chunk of a plain CSS identifier after the name start.
    sass::string identifierBody();

    // Like [consumeidentifierBody], but parses the body into the [text] buffer.
    void consumeidentifierBody(StringBuffer& text, bool unit = false);

    // Consumes a plain CSS string. This returns the parsed contents of the 
    // string—that is, it doesn't include quotes and its escapes are resolved.
    sass::string string();

    // Consumes and returns a natural number.
    // That is, a non - negative integer.
    // Doesn't support scientific notation.
    double naturalNumber();

    // Consumes tokens until it reaches a top-level `";"`, `")"`, `"]"`,
    // or `"}"` and returns their contents as a string. If [allowEmpty]
    // is `false` (the default), this requires at least one token.
    sass::string declarationValue(bool allowEmpty = false);

    // Consumes a `url()` token if possible, and returns `null` otherwise.
    sass::string tryUrl();

    // Consumes a Sass variable name, and returns
    // its name without the dollar sign.
    sass::string variableName();

    // Consumes an escape sequence and returns the text that defines it.
    // If [identifierStart] is true, this normalizes the escape sequence
    // as though it were at the beginning of an identifier.
    void escape(StringBuffer& buffer, bool identifierStart = false);

    // Consumes an escape sequence and returns the character it represents.
    uint32_t escapeCharacter();

    // Consumes the next character if it matches [condition].
    // Returns whether or not the character was consumed.
    bool scanCharIf(bool (*condition)(uint8_t character));

    // Consumes the next character if it's equal
    // to [letter], ignoring ASCII case.
    bool scanCharIgnoreCase(uint8_t letter);

    // Consumes the next character and asserts that
    // it's equal to [letter], ignoring ASCII case.
    void expectCharIgnoreCase(uint8_t letter);

    // Returns whether the scanner is immediately before a number. This follows [the CSS algorithm].
    // [the CSS algorithm]: https://drafts.csswg.org/css-syntax-3/#starts-with-a-number
    bool lookingAtNumber() const;

    // Returns whether the scanner is immediately before a plain CSS identifier.
    // If [forward] is passed, this looks that many characters forward instead.
    // This is based on [the CSS algorithm][], but it assumes all backslashes start escapes.
    // [the CSS algorithm]: https://drafts.csswg.org/css-syntax-3/#would-start-an-identifier
    bool lookingAtIdentifier(size_t forward = 0) const;

    // Returns whether the scanner is immediately before a sequence
    // of characters that could be part of a plain CSS identifier body.
    bool lookingAtIdentifierBody();

    // Consumes an identifier if its name exactly matches [text].
    bool scanIdentifier(const char* text);
    bool scanIdentifier(sass::string text);

    // ToDo: make template to ignore return type
    template <typename T, typename X>
    sass::string rawText(T(X::* consumer)())
    {
      const char* start = scanner.position;
      // We need to clean up after ourself
      // This has a chance to leak memory!
      (static_cast<X*>(this)->*consumer)();
      return scanner.substring(start);
    }

    // Consumes an identifier and asserts that its name exactly matches [text].
    void expectIdentifier(const char* text, sass::string name);

    // Throws an error associated with [span].
    void error(sass::string message, SourceSpan pstate /*, FileSpan span */);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
