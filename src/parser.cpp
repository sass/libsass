/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "parser.hpp"

#include "compiler.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "exceptions.hpp"
#include "ast_statements.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;
  
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Constructor
  Parser::Parser(
    Compiler& context,
    SourceDataObj source) :
    context(context),
    scanner(context, source),
    varStack(context.varStack),
    lastSilentComment()
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // ToDo: implement without the try clause
  // ToDo: measure if this brings any speed?
  bool Parser::isIdentifier(sass::string text)
  {
    try {
      auto src = SASS_MEMORY_NEW(SourceString,
        "sass:://identifier", std::move(text));
      Parser parser(context, src);
      sass::string id(parser.readIdentifier());
      return parser.scanner.isDone();
    }
    catch (Exception::ParserException&) {
      return false;
    }
  }

  // Consumes and ignores a comment if possible.
  // Returns whether the comment was consumed.
  bool Parser::scanComment()
  {
    if (scanner.peekChar() != $slash) return false;
    auto next = scanner.peekChar(1);
    if (next == $slash) {
      lastSilentComment = readSilentComment();
      return true;
    }
    else if (next == $asterisk) {
      loudComment();
      return true;
    }
    else {
      return false;
    }
  }

  // Consumes and ignores a silent (Sass-style) comment.
  SilentComment* Parser::readSilentComment()
  {
    scanner.expect("//");
    while (!scanner.isDone() && !isNewline(scanner.peekChar())) {
      scanner.readChar();
    }
    return nullptr;
  }

  // Consumes and ignores a loud (CSS-style) comment.
  void Parser::loudComment()
  {
    scanner.expect("/*");
    while (true) {
      auto next = scanner.readChar();
      if (next != $asterisk) continue;

      do {
        next = scanner.readChar();
      } while (next == $asterisk);
      if (next == $slash) break;
    }
  }

  // Consumes a plain CSS identifier. If [unit] is `true`, this 
  // doesn't parse a `-` followed by a digit. This ensures that 
  // `1px-2px` parses as subtraction rather than the unit `px-2px`.
  sass::string Parser::readIdentifier(bool unit)
  {

    // NOTE: this logic is largely duplicated in StylesheetParser._interpolatedIdentifier
    // and isIdentifier in utils.dart. Most changes here should be mirrored there.

    Offset start(scanner.offset);
    StringBuffer text;
    if (scanner.scanChar($dash)) {
      text.write($dash);
      if (scanner.scanChar($dash)) {
        text.writeCharCode($dash);
        consumeidentifierBody(text, unit);
        return text.buffer;
      }
    }

    auto first = scanner.peekChar();
    if (first == $nul) {
      error("Expected identifier.",
        scanner.rawSpanFrom(start));
    }
    else if (isNameStart(first)) {
      text.write(scanner.readChar());
    }
    else if (first == $backslash) {
      escape(text, /*identifierStart*/true);
    }
    else {
      error("Expected identifier.",
        scanner.rawSpanFrom(start));
    }

    consumeidentifierBody(text, unit);

    return std::move(text.buffer);

  }

  // Consumes a chunk of a plain CSS identifier after the name start.
  sass::string Parser::identifierBody()
  {
    StringBuffer text;
    consumeidentifierBody(text);
    if (text.empty()) {
      error(
        "Expected identifier body.",
        scanner.rawSpan());
    }
    return text.buffer;
  }

  // Like [consumeidentifierBody], but parses the body into the [text] buffer.
  void Parser::consumeidentifierBody(StringBuffer& text, bool unit)
  {
    while (true) {
      uint8_t next = scanner.peekChar();
      if (next == $nul) {
        break;
      }
      else if (unit && next == $dash) {
        // Disallow `-` followed by a dot or a digit in units.
        uint8_t second = scanner.peekChar(1);
        if (second != $nul && (second == $dot || isDigit(second))) break;
        text.write(scanner.readChar());
      }
      else if (isName(next)) {
        text.write(scanner.readChar());
      }
      else if (next == $backslash) {
        escape(text);
      }
      else {
        break;
      }
    }
  }

  // Consumes a plain CSS string. This returns the parsed contents of the 
  // string—that is, it doesn't include quotes and its escapes are resolved.
  sass::string Parser::string()
  {
    // NOTE: this logic is largely duplicated in ScssParser._interpolatedString.
    // Most changes here should be mirrored there.

    uint8_t quote = scanner.readChar();
    if (quote != $single_quote && quote != $double_quote) {
      error("Expected string.",
        scanner.rawSpan());
        /*,
        position: quote == null ? scanner.position : scanner.position - 1*/
    }

    StringBuffer buffer;
    while (true) {
      uint8_t next = scanner.peekChar();
      if (next == quote) {
        scanner.readChar();
        break;
      }
      else if (next == $nul || isNewline(next)) {
        sass::sstream strm;
        strm << "Expected " << quote << ".";
        error(strm.str(),
          scanner.rawSpan());
      }
      else if (next == $backslash) {
        if (isNewline(scanner.peekChar(1))) {
          scanner.readChar();
          scanner.readChar();
        }
        else {
          buffer.writeCharCode(escapeCharacter());
        }
      }
      else {
        buffer.write(scanner.readChar());
      }
    }

    return buffer.buffer;
  }

  // Consumes and returns a natural number.
  // That is, a non - negative integer.
  // Doesn't support scientific notation.
  double Parser::naturalNumber()
  {
    if (!isDigit(scanner.peekChar())) {
      error("Expected digit.",
        scanner.rawSpan());
    }
    uint8_t first = scanner.readChar();
    double number = asDecimal(first);
    while (isDigit(scanner.peekChar())) {
      number = asDecimal(scanner.readChar())
        + number * 10;
    }
    return number;
  }
  // EO naturalNumber

  // Consumes tokens until it reaches a top-level `";"`, `")"`, `"]"`,
  // or `"}"` and returns their contents as a string. If [allowEmpty]
  // is `false` (the default), this requires at least one token.
  sass::string Parser::declarationValue(bool allowEmpty)
  {
    // NOTE: this logic is largely duplicated in
    // StylesheetParser.parseInterpolatedDeclarationValue.
    // Most changes here should be mirrored there.

    sass::string url;
    StringBuffer buffer;
    bool wroteNewline = false;
    sass::vector<uint8_t> brackets;

    while (true) {
      uint8_t next = scanner.peekChar();
      switch (next) {
      case $backslash:
        escape(buffer, true);
        wroteNewline = false;
        break;

      case $double_quote:
      case $single_quote:
        buffer.write(rawText(&Parser::string));
        wroteNewline = false;
        break;

      case $slash:
        if (scanner.peekChar(1) == $asterisk) {
          buffer.write(rawText(&Parser::loudComment));
        }
        else {
          buffer.write(scanner.readChar());
        }
        wroteNewline = false;
        break;

      case $space:
      case $tab:
        if (wroteNewline || !isWhitespace(scanner.peekChar(1))) {
          buffer.write($space);
        }
        scanner.readChar();
        break;

      case $lf:
      case $cr:
      case $ff:
        if (!isNewline(scanner.peekChar(-1))) {
          buffer.write("\n");
        }
        scanner.readChar();
        wroteNewline = true;
        break;

      case $lparen:
      case $lbrace:
      case $lbracket:
        buffer.write(next);
        brackets.emplace_back(opposite(scanner.readChar()));
        wroteNewline = false;
        break;

      case $rparen:
      case $rbrace:
      case $rbracket:
        if (brackets.empty()) goto outOfLoop;
        buffer.write(next);
        scanner.expectChar(brackets.back());
        wroteNewline = false;
        brackets.pop_back();
        break;

      case $semicolon:
        if (brackets.empty()) goto outOfLoop;
        buffer.write(scanner.readChar());
        break;

      case $u:
      case $U:
        url = tryUrl() ;
        if (!url.empty()) {
          buffer.write(url);
        }
        else {
          buffer.write(scanner.readChar());
        }
        wroteNewline = false;
        break;

      default:
        if (next == $nul) goto outOfLoop;

        if (lookingAtIdentifier()) {
          buffer.write(readIdentifier());
        }
        else {
          buffer.write(scanner.readChar());
        }
        wroteNewline = false;
        break;
      }
    }

  outOfLoop:

    if (!brackets.empty()) scanner.expectChar(brackets.back());
    if (!allowEmpty && buffer.empty()) error(
      "Expected token.", scanner.rawSpan());
    return buffer.buffer;

  }
  // EO declarationValue

  // Consumes a `url()` token if possible, and returns `null` otherwise.
  sass::string Parser::tryUrl()
  {

    // NOTE: this logic is largely duplicated in ScssParser.tryUrlContents.
    // Most changes here should be mirrored there.

    StringScannerState state = scanner.state();
    if (!scanIdentifier("url")) return "";

    if (!scanner.scanChar($lparen)) {
      scanner.backtrack(state);
      return "";
    }

    scanWhitespace();

    // Match Ruby Sass's behavior: parse a raw URL() if possible,
    // and if not backtrack and re-parse as a function expression.
    uint8_t next;
    StringBuffer buffer;
    buffer.write("url(");
    while (true) {
      if (!scanner.peekChar(next)) {
        break;
      }
      if (next == $percent ||
          next == $ampersand ||
          next == $hash ||
          (next >= $asterisk && next <= $tilde) ||
          next >= 0x0080) {
        buffer.write(scanner.readChar());
      }
      else if (next == $backslash) {
        escape(buffer);
      }
      else if (isWhitespace(next)) {
        scanWhitespace();
        if (scanner.peekChar() != $rparen) break;
      }
      else if (next == $rparen) {
        buffer.write(scanner.readChar());
        return buffer.buffer;
      }
      else {
        break;
      }
    }

    scanner.backtrack(state);
    return "";
  }
  // EO tryUrl

  // Consumes a Sass variable name, and returns
  // its name without the dollar sign.
  sass::string Parser::variableName()
  {
    scanner.expectChar($dollar);
    // dart sass removes the dollar
    return readIdentifier();
  }

  // Consumes an escape sequence and returns the text that defines it.
  // If [identifierStart] is true, this normalizes the escape sequence
  // as though it were at the beginning of an identifier.
  void Parser::escape(StringBuffer& buffer, bool identifierStart)
  {
    Offset start(scanner.offset);
    scanner.expectChar($backslash);
    uint32_t value = 0;
    uint8_t first, next;
    if (!scanner.peekChar(first)) {
      return;
    }
    else if (isNewline(first)) {
      error("Expected escape sequence.",
        scanner.rawSpan());
      return;
    }
    else if (isHex(first)) {
      for (uint8_t i = 0; i < 6; i++) {
        scanner.peekChar(next);
        if (next == $nul || !isHex(next)) break;
        value *= 16;
        value += asHex(scanner.readChar());
      }

      scanCharIf(isWhitespace);
    }
    else {
      value = scanner.readChar();
    }

    if (identifierStart ? isNameStart(value) : isName(value)) {
      if (!utf8::internal::is_code_point_valid(value)) {
        error("Invalid Unicode code point.",
          scanner.relevantSpanFrom(start));
      }
      buffer.writeCharCode(value);
      return;
    }
    else if (value <= 0x1F ||
      value == 0x7F ||
      (identifierStart && isDigit(value))) {
      buffer.write($backslash);
      if (value > 0xF) buffer.write(hexCharFor(value >> 4));
      buffer.write(hexCharFor(value & 0xF));
      buffer.write($space);
      return;
    }
    else {
      buffer.write($backslash);
      buffer.writeCharCode(value);
      return;
    }
  }
  // EO tryUrl

  // Consumes an escape sequence and returns the character it represents.
  uint32_t Parser::escapeCharacter()
  {
    // See https://drafts.csswg.org/css-syntax-3/#consume-escaped-code-point.

    uint8_t first, next;
    scanner.expectChar($backslash);
    if (!scanner.peekChar(first)) {
      return 0xFFFD;
    }
    else if (isNewline(first)) {
      error("Expected escape sequence.",
        scanner.rawSpan());
      return 0;
    }
    else if (isHex(first)) {
      uint32_t value = 0;
      for (uint8_t i = 0; i < 6; i++) {
        if (!scanner.peekChar(next) || !isHex(next)) break;
        value = (value << 4) + asHex(scanner.readChar());
      }
      if (isWhitespace(scanner.peekChar())) {
        scanner.readChar();
      }
      if (value == 0 ||
        (value >= 0xD800 && value <= 0xDFFF) ||
        value >= 0x10FFFF) {
        return 0xFFFD;
      }
      else {
        return value;
      }
    }
    else {
      return scanner.readChar();
    }
  }

  // Consumes the next character if it matches [condition].
  // Returns whether or not the character was consumed.
  bool Parser::scanCharIf(bool(*condition)(uint8_t character))
  {
    uint8_t next = scanner.peekChar();
    if (!condition(next)) return false;
    scanner.readChar();
    return true;
  }

  // Consumes the next character if it's equal
  // to [letter], ignoring ASCII case.
  bool Parser::scanCharIgnoreCase(uint8_t letter)
  {
    if (!equalsLetterIgnoreCase(letter, scanner.peekChar())) return false;
    scanner.readChar();
    return true;
  }

  // Consumes the next character and asserts that
  // it's equal to [letter], ignoring ASCII case.
  void Parser::expectCharIgnoreCase(uint8_t letter)
  {
    Offset start(scanner.offset);
    uint8_t actual(scanner.readChar());
    if (!equalsLetterIgnoreCase(letter, actual)) {
      sass::string msg = "Expected \"";
      msg += letter; msg += "\".";
      scanner.offset = start;
      error(msg, scanner.rawSpan());
    }
  }

  // Returns whether the scanner is immediately before a number. This follows [the CSS algorithm].
  // [the CSS algorithm]: https://drafts.csswg.org/css-syntax-3/#starts-with-a-number
  bool Parser::lookingAtNumber() const
  {
    uint8_t first, second, third;
    if (!scanner.peekChar(first)) return false;
    if (isDigit(first)) return true;

    if (first == $dot) {
      return scanner.peekChar(second, 1)
        && isDigit(second);
    }
    else if (first == $plus || first == $minus) {
      if (!scanner.peekChar(second, 1)) return false;
      if (isDigit(second)) return true;
      if (second != $dot) return false;

      return scanner.peekChar(third, 2)
        && isDigit(third);
    }
    else {
      return false;
    }
  }
  // EO lookingAtNumber

  // Returns whether the scanner is immediately before a plain CSS identifier.
  // If [forward] is passed, this looks that many characters forward instead.
  // This is based on [the CSS algorithm][], but it assumes all backslashes start escapes.
  // [the CSS algorithm]: https://drafts.csswg.org/css-syntax-3/#would-start-an-identifier
  bool Parser::lookingAtIdentifier(size_t forward) const
  {
    // See also [ScssParser.lookingAtInterpolatedIdentifier].
    uint8_t first, second;
    if (!scanner.peekChar(first, forward)) return false;
    if (isNameStart(first) || first == $backslash) return true;
    if (first != $dash) return false;

    if (!scanner.peekChar(second, forward + 1)) return false;

    return isNameStart(second)
      || second == $backslash
      || second == $dash;
  }
  // EO lookingAtIdentifier

  // Returns whether the scanner is immediately before a sequence
  // of characters that could be part of a plain CSS identifier body.
  bool Parser::lookingAtIdentifierBody()
  {
    uint8_t next = scanner.peekChar();
    return next && (isName(next) || next == $backslash);
  }
  // EO lookingAtIdentifierBody

  // Consumes an identifier if its name exactly matches [text].
  bool Parser::scanIdentifier(const char* text)
  {
    if (!lookingAtIdentifier()) return false;

    StringScannerState state = scanner.state();
    for (size_t i = 0; text[i]; i++) {
      if (scanCharIgnoreCase(text[i])) continue;
      scanner.backtrack(state);
      return false;
    }

    if (!lookingAtIdentifierBody()) return true;
    scanner.backtrack(state);
    return false;
  }
  // EO scanIdentifier

  // Consumes an identifier if its name exactly matches [text].
  bool Parser::scanIdentifier(sass::string text)
  {
    if (!lookingAtIdentifier()) return false;

    StringScannerState state = scanner.state();
    for (size_t i = 0; text[i]; i++) {
      // uint8_t next = text[i]; // cast needed
      if (scanCharIgnoreCase(text[i])) continue;
      scanner.backtrack(state);
      return false;
    }

    if (!lookingAtIdentifierBody()) return true;
    scanner.backtrack(state);
    return false;
  }
  // EO scanIdentifier

  // Consumes an identifier and asserts that its name exactly matches [text].
  void Parser::expectIdentifier(const char* text, sass::string name)
  {
    Offset start(scanner.offset);
    for (uint8_t i = 0; text[i]; i++) {
      if (scanCharIgnoreCase(text[i])) continue;
      error("Expected " + name + ".",
        scanner.rawSpanFrom(start));
    }
    if (!lookingAtIdentifierBody()) return;
    error("Expected " + name + ".",
      scanner.rawSpanFrom(start));
  }
  
  // Throws an error associated with [pstate].
  void Parser::error(sass::string message, SourceSpan pstate) {
    callStackFrame frame(context, BackTrace(pstate));
    throw Exception::ParserException(context, message);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
