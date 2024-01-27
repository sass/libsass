/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "parser_sass.hpp"

#include "character.hpp"
#include "utf8/checked.h"
#include "interpolation.hpp"
#include "ast_imports.hpp"
#include "compiler.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  Interpolation* SassParser::styleRuleSelector()
  {
    Offset start(scanner.offset);
    InterpolationBuffer buffer(scanner);
    do {
      buffer.addInterpolation(readAlmostAnyValue(true));
      buffer.writeCharCode($lf);
    } while (buffer.trailingStringEndsWith(",") &&
      scanCharIf(isNewline));

    return buffer.getInterpolation(scanner.rawSpanFrom(start));
  }

  // Consumes and ignores a loud (CSS-style) comment.
  // This overrides loud comment consumption so that
  // it doesn't consume multi-line comments.
  void SassParser::scanLoudComment()
  {
    scanner.expect("/*");
    while (true) {
      auto next = scanner.readChar();
      if (isNewline(next)) {
        scanner._fail("*/");
      }
      if (next != $asterisk) continue;

      do {
        next = scanner.readChar();
      } while (next == $asterisk);
      if (next == $slash) break;
    }
  }

  void SassParser::expectStatementSeparator(sass::string name) {
    if (!atEndOfStatement()) expectNewline();
    if (peekIndentation() <= currentIndentation) return;
    sass::sstream strm;
    strm << "Nothing may be indented ";
    if (name.empty()) { strm << "here."; }
    else { strm << "beneath a " + name + "."; }
    Offset start(scanner.relevant);
    while (uint32_t chr = scanner.peekChar()) {
      if (!isWhitespace(chr)) break;
      scanner.readChar();
    }
    error(strm.str(), scanner.rawSpanFrom(start));

      /*,
      position: nextIndentationEnd.position*/
  }

  bool SassParser::atEndOfStatement()
  {
    uint8_t next;
    if (scanner.peekChar(next)) {
      return isNewline(next);
    }
    return true;
  }

  bool SassParser::lookingAtChildren()
  {
    return atEndOfStatement() &&
      peekIndentation() > currentIndentation;
  }

  void SassParser::scanImportArgument(ImportRule* rule)
  {
    uint8_t next = scanner.peekChar();
    StringScannerState state(scanner.state());
    switch (next) {
    case $u:
    case $U:
      if (scanIdentifier("url")) {
        if (scanner.scanChar($lparen)) {
          scanner.backtrack(state);
          StylesheetParser::scanImportArgument(rule);
          return;
        }
        else {
          scanner.backtrack(state);
        }
      }
      break;

    case $quote:
    case $apos:
      StylesheetParser::scanImportArgument(rule);
      return;
    }

    Offset start(scanner.offset);
    StringScannerState state2(scanner.state());
    while (scanner.peekChar(next) &&
      next != $comma &&
      next != $semicolon &&
      !isNewline(next)) {
      scanner.readChar();
      next = scanner.peekChar();
    }

    sass::string url = scanner.substring(state2.position);

    if (isPlainImportUrl(url)) {
      InterpolationObj itpl = SASS_MEMORY_NEW(
        Interpolation, scanner.relevantSpanFrom(start));
      auto str = SASS_MEMORY_NEW(String,
        scanner.relevantSpanFrom(start),
        std::move(url), true);
      // Must be an easier way to get quotes?
      str->value(str->inspect());
      itpl->append(str);
      rule->append(SASS_MEMORY_NEW(StaticImport,
        scanner.relevantSpanFrom(start), itpl, nullptr));
    }
    else {

      SourceSpan pstate(scanner.relevantSpanFrom(start));
      if (!compiler.callCustomImporters(url, pstate, rule)) {
        rule->append(SASS_MEMORY_NEW(IncludeImport,
          pstate, scanner.sourceUrl, url, nullptr));
      }

    }

  }

  bool SassParser::scanElse(size_t ifIndentation)
  {
    if (peekIndentation() != ifIndentation) return false;
    StringScannerState state(scanner.state());
    size_t startIndentation = currentIndentation;
    size_t startNextIndentation = nextIndentation;
    StringScannerState startNextIndentationEnd = nextIndentationEnd;

    readIndentation();
    if (scanner.scanChar($at) && scanIdentifier("else")) return true;

    scanner.backtrack(state);
    currentIndentation = startIndentation;
    nextIndentation = startNextIndentation;
    nextIndentationEnd = startNextIndentationEnd;
    return false;
  }

  StatementVector SassParser::readChildren(Statement* (StylesheetParser::* parser)())
  {
    StatementVector children;
    whileIndentedLower(parser, children);
    return children;
  }

  StatementVector SassParser::readStatements(Statement* (StylesheetParser::* parser)())
  {
    uint8_t first = scanner.peekChar();
    if (first == $tab || first == $space) {
      error("Indenting at the beginning of the document is illegal.",
        scanner.rawSpan());
        /*position: 0, length : scanner.position*/
    }

    StatementVector statements;
    while (!scanner.isDone()) {
      Statement* child = parseChild(parser);
      if (child != nullptr) statements.emplace_back(child);
      size_t indentation = readIndentation();
      if (indentation != 0) {
        error(
          "Inconsistent indentation, expected 0 spaces.",
          scanner.rawSpan());
      }
    }
    return statements;
  }

  Statement* SassParser::parseChild(Statement* (StylesheetParser::* child)())
  {
    switch (scanner.peekChar()) {
      // Ignore empty lines.
    case $cr:
    case $lf:
    case $ff:
      return nullptr;

    case $dollar:
      return readVariableDeclarationWithoutNamespace("", scanner.offset);
      break;

    case $slash:
      switch (scanner.peekChar(1)) {
      case $slash:
        lastSilentComment = readSilentComment();
        return lastSilentComment.ptr();
        break;
      case $asterisk:
        return readLoudComment();
        break;
      default:
        return (this->*child)();
        break;
      }
      break;

    default:
      return (this->*child)();
      break;
    }
  }

  SilentComment* SassParser::readSilentComment()
  {
    Offset start(scanner.offset);
    scanner.expect("//");
    StringBuffer buffer;
    size_t parentIndentation = currentIndentation;

    do {
      sass::string commentPrefix = scanner.scanChar($slash) ? "///" : "//";

      while (true) {
        buffer.write(commentPrefix);

        // Skip the initial characters because we're already writing the
        // slashes.
        for (size_t i = commentPrefix.length();
          i < currentIndentation - parentIndentation;
          i++) {
          buffer.writeCharCode($space);
        }

        while (!scanner.isDone() && !isNewline(scanner.peekChar())) {
          buffer.writeCharCode(scanner.readChar());
        }
        buffer.write("\n");

        if (peekIndentation() < parentIndentation) goto endOfLoop;

        if (peekIndentation() == parentIndentation) {
          // Look ahead to the next line to see if it starts another comment.
          if (scanner.peekChar(1 + parentIndentation) == $slash &&
            scanner.peekChar(2 + parentIndentation) == $slash) {
            readIndentation();
          }
          break;
        }
        readIndentation();
      }
    } while (scanner.scan("//"));

  endOfLoop:

    lastSilentComment = SASS_MEMORY_NEW(SilentComment,
      scanner.rawSpanFrom(start), std::move(buffer.buffer));
    return lastSilentComment;
  }

  LoudComment* SassParser::readLoudComment()
  {
    Offset start(scanner.offset);
    scanner.expect("/*");

    bool first = true;
    InterpolationBuffer buffer(scanner);
    buffer.write("/*");
    size_t parentIndentation = currentIndentation;
    while (true) {
      if (first) {
        // If the first line is empty, ignore it.
        const char* beginningOfComment = scanner.position;
        scanSpaces();
        if (isNewline(scanner.peekChar())) {
          readIndentation();
          buffer.writeCharCode($space);
        }
        else {
          buffer.write(scanner.substring(beginningOfComment));
        }
      }
      else {
        buffer.write("\n");
        buffer.write(" * ");
      }
      first = false;

      for (size_t i = 3; i < currentIndentation - parentIndentation; i++) {
        buffer.writeCharCode($space);
      }

      while (!scanner.isDone()) {
        uint8_t next = scanner.peekChar();
        switch (next) {
        case $lf:
        case $cr:
        case $ff:
          goto endOfLoop;

        case $hash:
          if (scanner.peekChar(1) == $lbrace) {
            buffer.add(readSingleInterpolation());
          }
          else {
            buffer.writeCharCode(scanner.readChar());
          }
          break;

        default:
          buffer.writeCharCode(scanner.readChar());
          break;
        }
      }

    endOfLoop:

      if (peekIndentation() <= parentIndentation) break;

      // Preserve empty lines.
      while (lookingAtDoubleNewline()) {
        expectNewline();
        buffer.write("\n");
        buffer.write(" *");
      }

      readIndentation();
    }

    if (!buffer.trailingStringEndsWith("*/")) {
      buffer.write(" */");
    }

    SourceSpan pstate(scanner.rawSpanFrom(start));
    InterpolationObj itpl = buffer.getInterpolation(pstate);
    return SASS_MEMORY_NEW(LoudComment, std::move(pstate), itpl);
  }

  void SassParser::scanWhitespaceWithoutComments()
  {
    // This overrides whitespace consumption so that
    // it doesn't consume newlines or loud comments.
    while (!scanner.isDone()) {
      uint8_t next = scanner.peekChar();
      if (next != $tab && next != $space) break;
      scanner.readChar();
    }

    if (scanner.peekChar() == $slash && scanner.peekChar(1) == $slash) {
      lastSilentComment = readSilentComment();
    }
  }

  void SassParser::expectNewline()
  {
    switch (scanner.peekChar()) {
    case $semicolon:
      error("semicolons aren't allowed in the indented syntax.",
        scanner.rawSpan());
      return;
    case $cr:
      scanner.readChar();
      if (scanner.peekChar() == $lf) scanner.readChar();
      return;
    case $lf:
    case $ff:
      scanner.readChar();
      return;
    default:
      error("expected newline.",
        scanner.rawSpan());
    }
  }

  bool SassParser::lookingAtDoubleNewline()
  {
    uint8_t next = scanner.peekChar();
    uint8_t nextChar = scanner.peekChar(1);
    switch (next) {
    case $cr:
      if (nextChar == $lf) return isNewline(scanner.peekChar(2));
      return nextChar == $cr || nextChar == $ff;
    case $lf:
    case $ff:
      return isNewline(scanner.peekChar(1));
    default:
      return false;
    }
  }

  void SassParser::whileIndentedLower(Statement* (StylesheetParser::* child)(), StatementVector& children)
  {
    size_t parentIndentation = currentIndentation;
    size_t childIndentation = sass::string::npos;
    while (peekIndentation() > parentIndentation) {
      size_t indentation = readIndentation();
      if (childIndentation == sass::string::npos) {
        childIndentation = indentation;
      }
      if (childIndentation != indentation) {
        sass::sstream msg;
        msg << "Inconsistent indentation, expected "
          << childIndentation << " spaces.";
        error(msg.str(),
          scanner.rawSpan());

          /*,
          position: scanner.position - scanner.column,
          length : scanner.column*/
      }
      children.emplace_back(parseChild(child));
    }

  }

  size_t SassParser::readIndentation()
  {
    if (nextIndentation == sass::string::npos) {
      peekIndentation();
    }
    currentIndentation = nextIndentation;
    scanner.backtrack(nextIndentationEnd);
    nextIndentation = sass::string::npos;
    // What does this mean, where is it used?
    // nextIndentationEnd = null; ToDo
    return currentIndentation;

  }

  // Returns the indentation level of the next line.
  size_t SassParser::peekIndentation()
  {
    if (nextIndentation != sass::string::npos) {
      return nextIndentation;
    }

    if (scanner.isDone()) {
      nextIndentation = 0;
      nextIndentationEnd = scanner.state();
      return 0;
    }

    StringScannerState start = scanner.state();
    if (!scanCharIf(isNewline)) {
      error("Expected newline.",
        scanner.rawSpan());
        /* position: scanner.position*/
    }

    bool containsTab;
    bool containsSpace;
    do {
      containsTab = false;
      containsSpace = false;
      nextIndentation = 0;

      while (true) {
        uint8_t next = scanner.peekChar();
        if (next == $space) {
          containsSpace = true;
        }
        else if (next == $tab) {
          containsTab = true;
        }
        else {
          break;
        }
        nextIndentation++;
        scanner.readChar();
      }

      if (scanner.isDone()) {
        nextIndentation = 0;
        nextIndentationEnd = scanner.state();
        scanner.backtrack(start);
        return 0;
      }
    } while (scanCharIf(isNewline));

    checkIndentationConsistency(containsTab, containsSpace);

    if (nextIndentation > 0) {
      if (indentType == SassIndentType::AUTO) {
        indentType = containsSpace ?
          SassIndentType::SPACES :
          SassIndentType::TABS;
      }
    }
    nextIndentationEnd = scanner.state();
    scanner.backtrack(start);
    return nextIndentation;
  }

  // Ensures that the document uses consistent characters for indentation.
  // The [containsTab] and [containsSpace] parameters refer to
  // a single line of indentation that has just been parsed.
  void SassParser::checkIndentationConsistency(bool containsTab, bool containsSpace)
  {
    if (containsTab) {
      if (containsSpace) {
        error("Tabs and spaces may not be mixed.",
          scanner.rawSpan());
          /* position: scanner.position - scanner.column,
          length : scanner.column*/
      }
      else if (useSpaceIndentation()) {
        error("Expected spaces, was tabs.",
          scanner.rawSpan());
          /* position: scanner.position - scanner.column,
          length : scanner.column*/
      }
    }
    else if (containsSpace && useTabIndentation()) {
      error("Expected tabs, was spaces.",
        scanner.rawSpan());
        /* position: scanner.position - scanner.column, length : scanner.column*/
    }
  }

}
