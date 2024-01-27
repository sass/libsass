/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "parser_scss.hpp"

#include "character.hpp"
#include "utf8/checked.h"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Parses and returns a selector used in a style rule.
  Interpolation* ScssParser::styleRuleSelector()
  {
    return readAlmostAnyValue();
  }
  // EO styleRuleSelector

  // Asserts that the scanner is positioned before a statement separator,
  // or at the end of a list of statements. If the [name] of the parent
  // rule is passed, it's used for error reporting. This consumes
  // whitespace, but nothing else, including comments.
  void ScssParser::expectStatementSeparator(sass::string name)
  {
    scanWhitespaceWithoutComments();
    if (scanner.isDone()) return;
    uint8_t next = scanner.peekChar();
    if (next == $semicolon || next == $rbrace) return;
    scanner.expectChar($semicolon);
  }
  // EO expectStatementSeparator

  // Whether the scanner is positioned at the end of a statement.
  bool ScssParser::atEndOfStatement()
  {
    if (scanner.isDone()) return true;
    uint8_t next = scanner.peekChar();
    return next == $semicolon
      || next == $rbrace
      || next == $lbrace;
  }
  // EO atEndOfStatement

  // Whether the scanner is positioned before a block of
  // children that can be parsed with [children].
  bool ScssParser::lookingAtChildren()
  {
    return scanner.peekChar() == $lbrace;
  }
  // EO lookingAtChildren

  // Tries to scan an `@else` rule after an `@if` block, and returns whether 
  // that succeeded. This should just scan the rule name, not anything
  // afterwards. [ifIndentation] is the result of [currentIndentation]
  // from before the corresponding `@if` was parsed.
  bool ScssParser::scanElse(size_t)
  {
    StringScannerState start = scanner.state();
    scanWhitespace();
    // StringScannerState beforeAt = scanner.state();
    if (scanner.scanChar($at)) {
      if (scanIdentifier("else", true)) return true;
      if (scanIdentifier("elseif", true)) {
        /*
        logger.warn(
          "@elseif is deprecated and will not be supported in future Sass versions.\n"
          "Use "@else if" instead.",
          span: scanner.spanFrom(beforeAt),
          deprecation : true);
        */
        scanner.offset.column -= 2;
        scanner.position -= 2;
        return true;
      }
    }
    scanner.backtrack(start);
    return false;
  }
  // EO scanElse

  // Consumes a block of child statements. Unlike most production consumers,
  // this does *not* consume trailing whitespace. This is necessary to ensure
  // that the source span for the parent rule doesn't cover whitespace after the rule.
  StatementVector ScssParser::readChildren(
    Statement* (StylesheetParser::* child)())
  {
    scanner.expectChar($lbrace);
    scanWhitespaceWithoutComments();
    StatementVector children;
    while (true) {
      switch (scanner.peekChar()) {
      case $dollar:
        children.emplace_back(readVariableDeclarationWithoutNamespace("", scanner.offset));
        break;

      case $slash:
        switch (scanner.peekChar(1)) {
        case $slash:
          lastSilentComment = readSilentComment();
          scanWhitespaceWithoutComments();
          break;
        case $asterisk:
          children.emplace_back(readLoudComment());
          scanWhitespaceWithoutComments();
          break;
        default:
          children.emplace_back((this->*child)());
          break;
        }
        break;

      case $semicolon:
        scanner.readChar();
        scanWhitespaceWithoutComments();
        break;

      case $rbrace:
        scanner.expectChar($rbrace);
        return children;

      default:
        children.emplace_back((this->*child)());
        break;
      }
    }
  }
  // EO children

  // Consumes top-level statements. The [statement] callback may return `null`,
  // indicating that a statement was consumed that shouldn't be added to the AST.
  StatementVector ScssParser::readStatements(
    Statement* (StylesheetParser::* statement)())
  {
    scanWhitespaceWithoutComments();
    StatementVector statements;
    while (!scanner.isDone()) {
      switch (scanner.peekChar()) {
      case $dollar:
        statements.emplace_back(readVariableDeclarationWithoutNamespace("", scanner.offset));
        break;

      case $slash:
        switch (scanner.peekChar(1)) {
        case $slash:
          lastSilentComment = readSilentComment();
          scanWhitespaceWithoutComments();
          break;
        case $asterisk:
          statements.emplace_back(readLoudComment());
          scanWhitespaceWithoutComments();
          break;
        default:
          StatementObj child = (this->*statement)();
          if (child != nullptr) statements.emplace_back(child);
          break;
        }
        break;

      case $semicolon:
        scanner.readChar();
        scanWhitespaceWithoutComments();
        break;

      default:
        StatementObj child = (this->*statement)();
        if (child != nullptr) statements.emplace_back(child);
        break;
      }
    }
    return statements;
  }
  // EO statements

  // Consumes a statement-level silent comment block.
  SilentComment* ScssParser::readSilentComment()
  {
    StringScannerState start = scanner.state();
    scanner.expect("//");

    do {
      while (!scanner.isDone() &&
        !isNewline(scanner.readChar())) {}
      if (scanner.isDone()) break;
      scanWhitespaceWithoutComments();
    }
    while (scanner.scan("//"));

    if (plainCss()) {
      error("Silent comments aren't allowed in plain CSS.",
        scanner.relevantSpanFrom(start.offset));
    }

    return SASS_MEMORY_NEW(SilentComment, scanner.rawSpanFrom(start.offset),
        scanner.substring(start.position, scanner.position));
  }
  // EO readSilentComment

  // Consumes a statement-level loud comment block.
  LoudComment* ScssParser::readLoudComment()
  {
    InterpolationBuffer buffer(scanner);
    Offset start(scanner.offset);
    scanner.expect("/*");
    buffer.write("/*");
    while (true) {
      switch (scanner.peekChar()) {
      case $hash:
        if (scanner.peekChar(1) == $lbrace) {
          buffer.add(readSingleInterpolation());
        }
        else {
          buffer.write(scanner.readChar());
        }
        break;

      case $asterisk:
        buffer.write(scanner.readChar());
        if (scanner.peekChar() != $slash) break;
        buffer.write(scanner.readChar());
        return SASS_MEMORY_NEW(LoudComment,
          scanner.rawSpanFrom(start),
          buffer.getInterpolation(
            scanner.rawSpanFrom(start)));

      case $cr:
        scanner.readChar();
        if (scanner.peekChar() != $lf) {
          buffer.write($lf);
        }
        break;

      case $ff:
        scanner.readChar();
        buffer.write($lf);
        break;

      default:
        buffer.write(scanner.readChar());
        break;
      }
    }

    return nullptr;
  }
  // EO loudComment

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
