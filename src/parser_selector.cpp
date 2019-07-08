/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "parser_selector.hpp"

#include "charcode.hpp"
#include "character.hpp"
#include "ast_selectors.hpp"
#include "scanner_string.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  // Parse content into selector list
  // Throws if not everything is consumed
  SelectorList* SelectorParser::parseSelectorList()
  {
    SelectorListObj selector(readSelectorList());
    if (!scanner.isDone()) {
      error("expected selector.",
        scanner.rawSpan());
    }
    return selector.detach();
  }

  // Parse content into compound selector
  // Throws if not everything is consumed
  CompoundSelector* SelectorParser::parseCompoundSelector()
  {
    CompoundSelectorObj compound(readCompoundSelector());
    if (!scanner.isDone()) {
      error("expected selector.",
        scanner.rawSpan());
    }
    return compound.detach();
  }

  // Parse content into simple selector
  // Throws if not everything is consumed
  SimpleSelector* SelectorParser::parseSimpleSelector()
  {
    SimpleSelectorObj simple(readSimpleSelector(allowParent));
    if (!scanner.isDone()) {
      error("unexpected token.",
        scanner.relevantSpan());
    }
    return simple.detach();
  }

  // Consumes a selector list.
  SelectorList* SelectorParser::readSelectorList()
  {
    Offset start(scanner.offset);
    const char* previousLine = scanner.position;
    sass::vector<ComplexSelectorObj> items;
    items.emplace_back(readComplexSelector());

    scanWhitespace();
    while (scanner.scanChar($comma)) {
      scanWhitespace();
      uint8_t next = scanner.peekChar();
      if (next == $comma) continue;
      if (scanner.isDone()) break;

      bool lineBreak = scanner.hasLineBreak(previousLine); // ToDo
      // var lineBreak = scanner.line != previousLine;
      // if (lineBreak) previousLine = scanner.line;
      items.emplace_back(readComplexSelector(lineBreak));
    }

    return SASS_MEMORY_NEW(SelectorList,
      scanner.relevantSpanFrom(start), std::move(items));
  }

  // Consumes a complex selector.
  ComplexSelector* SelectorParser::readComplexSelector(bool lineBreak)
  {

    uint8_t next;
    Offset start(scanner.offset);
    SelectorComponentVector complex;

    while (true) {
      scanWhitespace();

      Offset before(scanner.offset);
      if (!scanner.peekChar(next)) {
        goto endOfLoop;
      }
      switch (next) {
      case $plus:
        scanner.readChar();
        complex.emplace_back(SASS_MEMORY_NEW(SelectorCombinator,
          scanner.rawSpanFrom(before), SelectorCombinator::ADJACENT));
        break;

      case $gt:
        scanner.readChar();
        complex.emplace_back(SASS_MEMORY_NEW(SelectorCombinator,
          scanner.rawSpanFrom(before), SelectorCombinator::CHILD));
        break;

      case $tilde:
        scanner.readChar();
        complex.emplace_back(SASS_MEMORY_NEW(SelectorCombinator,
          scanner.rawSpanFrom(before), SelectorCombinator::GENERAL));
        break;

      case $lbracket:
      case $dot:
      case $hash:
      case $percent:
      case $colon:
      case $ampersand:
      case $asterisk:
      case $pipe:
        complex.emplace_back(readCompoundSelector());
        if (scanner.peekChar() == $ampersand) {
          error(
            "\"&\" may only used at the beginning of a compound selector.",
            scanner.rawSpan());
        }
        break;

      default:
        if (!lookingAtIdentifier()) goto endOfLoop;
        complex.emplace_back(readCompoundSelector());
        if (scanner.peekChar() == $ampersand) {
          error(
            "\"&\" may only used at the beginning of a compound selector.",
            scanner.rawSpan());
        }
        break;
      }
    }

  endOfLoop:

    if (complex.empty()) {
      error("expected selector.",
        scanner.rawSpan());
    }

    ComplexSelector* selector = SASS_MEMORY_NEW(ComplexSelector,
      scanner.rawSpanFrom(start), std::move(complex));
    selector->hasPreLineFeed(lineBreak);
    return selector;

  }
  // EO readComplexSelector

  // Consumes a compound selector.
  CompoundSelector* SelectorParser::readCompoundSelector()
  {
    // Note: libsass uses a flag on the compound selector to
    // signal that it contains a real parent reference.
    // dart-sass uses ParentSelector with a suffix.
    Offset start(scanner.offset);
    CompoundSelectorObj compound = SASS_MEMORY_NEW(CompoundSelector,
      scanner.relevantSpan());

    if (scanner.scanChar($amp)) {
      if (!allowParent) {
        error(
          "Parent selectors aren't allowed here.",
          scanner.rawSpanFrom(start));
      }
      compound->withExplicitParent(true);
      if (lookingAtIdentifierBody()) {
        Offset before(scanner.offset);
        sass::string body(identifierBody());
        SimpleSelectorObj simple = SASS_MEMORY_NEW(TypeSelector,
          scanner.rawSpanFrom(before), std::move(body), "", false);
        if (!simple.isNull()) compound->append(simple);
      }
    }
    else {
      SimpleSelectorObj simple = readSimpleSelector(false);
      if (!simple.isNull()) compound->append(simple);
    }

    while (isSimpleSelectorStart(scanner.peekChar())) {
      SimpleSelectorObj simple = readSimpleSelector(false);
      if (!simple.isNull()) compound->append(simple);
    }

    compound->pstate(scanner.rawSpanFrom(start));
    return compound.detach();

  }
  // EO readCompoundSelector

  // Consumes a simple selector.
  SimpleSelector* SelectorParser::readSimpleSelector(bool allowParent)
  {

    Offset start(scanner.offset);
    uint8_t next = scanner.peekChar();
    if (next == $lbracket) {
      return readAttributeSelector();
    }
    else if (next == $dot) {
      return readClassSelector();
    }
    else if (next == $hash) {
      return readIdSelector();
    }
    else if (next == $percent)
    {
      PlaceholderSelectorObj selector(readPlaceholderSelector());
      if (!allowPlaceholder) {
        error("Placeholder selectors aren't allowed here.",
          scanner.rawSpanFrom(start));
      }
      return selector.detach();
    }
    else if (next == $colon) {
      return readPseudoSelector();
    }
    else if (next == $ampersand)
    {
      if (!allowParent) {
        error(
          "Parent selectors aren't allowed here.",
          scanner.rawSpanFrom(start));
      }
      return {};
    }
    else {
      return readTypeOrUniversalSelector();
    }

  }
  // EO readSimpleSelector

  // Consumes an attribute selector.
  AttributeSelector* SelectorParser::readAttributeSelector()
  {

    scanner.expectChar($lbracket);

    scanWhitespace();
    Offset start(scanner.offset);
    struct QualifiedName name(readAttributeName());
    SourceSpan span(scanner.relevantSpanFrom(start));
    scanWhitespace();

    if (scanner.scanChar($rbracket)) {
      return SASS_MEMORY_NEW(AttributeSelector,
        span, std::move(name));
    }

    sass::string op(readAttributeOperator());

    scanWhitespace();

    bool isIdent = true;
    sass::string value;
    uint8_t next = scanner.peekChar();
    // Check if we are looking at an unquoted text
    if (next != $single_quote && next != $double_quote) {
      value = readIdentifier();
    }
    else {
      value = string();
      isIdent = isIdentifier(value);
    }

    scanWhitespace();
    uint8_t modifier = 0;
    if (isAlphabetic(scanner.peekChar())) {
      modifier = scanner.readChar();
      scanWhitespace();
    }

    span = scanner.relevantSpanFrom(start);

    scanner.expectChar($rbracket);

    return SASS_MEMORY_NEW(AttributeSelector, span,
      std::move(name), std::move(op),
      std::move(value), isIdent,
      modifier);

  }
  // EO readAttributeSelector

  struct QualifiedName SelectorParser::readAttributeName()
  {

    if (scanner.scanChar($asterisk)) {
      scanner.expectChar($pipe);
      return { readIdentifier(), "*", true };
    }

    sass::string nameOrNamespace = readIdentifier();
    if (scanner.peekChar() != $pipe || scanner.peekChar(1) == $equal) {
      return { std::move(nameOrNamespace), "", false };
    }

    scanner.readChar();
    return { readIdentifier(), std::move(nameOrNamespace), true };

  }
  // EO readAttributeName

  // Consumes an attribute operator.
  sass::string SelectorParser::readAttributeOperator()
  {
    Offset start(scanner.offset);
    switch (scanner.readChar()) {
    case $equal:
      return "="; // AttributeOperator.equal;

    case $tilde:
      scanner.expectChar($equal);
      return "~="; // AttributeOperator.include;

    case $pipe:
      scanner.expectChar($equal);
      return "|="; // AttributeOperator.dash;

    case $caret:
      scanner.expectChar($equal);
      return "^="; // AttributeOperator.prefix;

    case $dollar:
      scanner.expectChar($equal);
      return "$="; // AttributeOperator.suffix;

    case $asterisk:
      scanner.expectChar($equal);
      return "*="; // AttributeOperator.substring;

    default:
      error("Expected \"]\".",
        scanner.rawSpanFrom(start));
      throw "Unreachable";
    }
  }
  // EO readAttributeOperator

  // Consumes a class operator.
  ClassSelector* SelectorParser::readClassSelector()
  {
    Offset start(scanner.offset);
    scanner.expectChar($dot);
    sass::string name = readIdentifier();
    return SASS_MEMORY_NEW(ClassSelector,
      scanner.rawSpanFrom(start), "." + name);
  }
  // EO readClassSelector

  // Consumes an in operator.
  IDSelector* SelectorParser::readIdSelector()
  {
    Offset start(scanner.offset);
    scanner.expectChar($hash);
    sass::string name = readIdentifier();
    return SASS_MEMORY_NEW(IDSelector,
      scanner.rawSpanFrom(start), "#" + name);
  }
  // EO readIdSelector

  // Consumes a placeholder operator.
  PlaceholderSelector* SelectorParser::readPlaceholderSelector()
  {
    Offset start(scanner.offset);
    scanner.expectChar($percent);
    sass::string name = readIdentifier();
    return SASS_MEMORY_NEW(PlaceholderSelector,
      scanner.rawSpanFrom(start), "%" + name);
  }
  // EO readPlaceholderSelector

  // Consumes a pseudo operator.
  PseudoSelector* SelectorParser::readPseudoSelector()
  {
    Offset start(scanner.offset);
    scanner.expectChar($colon);
    bool element = scanner.scanChar($colon);
    sass::string name = readIdentifier();

    if (!scanner.scanChar($lparen)) {
      return SASS_MEMORY_NEW(PseudoSelector,
        scanner.rawSpanFrom(start), name, element);
    }
    scanWhitespace();

    sass::string unvendored(name);
    unvendored = Util::unvendor(unvendored);

    sass::string argument;
    // Offset beforeArgument(scanner.offset);
    SelectorListObj selector = SASS_MEMORY_NEW(SelectorList, scanner.relevantSpan());
    if (element) {
      if (isSelectorPseudoElement(unvendored)) {
        selector = readSelectorList();
        for (auto complex : selector->elements()) {
          complex->chroots(true);
        }
      }
      else {
        argument = declarationValue(true);
      }
    }
    else if (isSelectorPseudoClass(unvendored)) {
      LOCAL_FLAG(allowParent, true);
      selector = readSelectorList();
      for (auto complex : selector->elements()) {
        complex->chroots(true);
      }
    }
    else if (unvendored == "nth-child" || unvendored == "nth-last-child") {
      argument = readAnPlusB();
      scanWhitespace();
      if (isWhitespace(scanner.peekChar(-1)) && scanner.peekChar() != $rparen) {
        expectIdentifier("of", "\"of\"");
        argument += " of";
        scanWhitespace();
        selector = readSelectorList();
      }
    }
    else {
      argument = declarationValue(true);
      StringUtils::makeRightTrimmed(argument);
    }
    scanner.expectChar($rparen);

    auto pseudo = SASS_MEMORY_NEW(PseudoSelector,
      scanner.rawSpanFrom(start),
      std::move(name), element != 0);
    if (!selector->empty()) pseudo->selector(selector);
    pseudo->argument(std::move(argument));
    return pseudo;

  }
  // EO readPlaceholderSelector

  // Consumes an `an+b` expression.
  sass::string SelectorParser::readAnPlusB()
  {

    StringBuffer buffer;
    uint8_t first, next, last;
    switch (scanner.peekChar()) {
    case $e:
    case $E:
      expectIdentifier("even", "\"even\"");
      return "even";

    case $o:
    case $O:
      expectIdentifier("odd", "\"odd\"");
      return "odd";

    case $plus:
    case $minus:
      buffer.write(scanner.readChar());
      break;
    }

    if (scanner.peekChar(first) && isDigit(first)) {
      while (isDigit(scanner.peekChar())) {
        buffer.write(scanner.readChar());
      }
      scanWhitespace();
      if (!scanCharIgnoreCase($n)) return buffer.buffer;
    }
    else {
      expectCharIgnoreCase($n);
    }
    buffer.write($n);
    scanWhitespace();

    scanner.peekChar(next);
    if (next != $plus && next != $minus) return buffer.buffer;
    buffer.write(scanner.readChar());
    scanWhitespace();

    if (!scanner.peekChar(last) || !isDigit(last)) {
      error("Expected a number.",
        scanner.rawSpan());
    }
    while (isDigit(scanner.peekChar())) {
      buffer.write(scanner.readChar());
    }
    return buffer.buffer;
  }
  // readAnPlusB

  // Consumes a type of universal (simple) selector.
  SimpleSelector* SelectorParser::readTypeOrUniversalSelector()
  {
    // Note: libsass has no explicit UniversalSelector,
    // we use a regular type selector with name == "*".
    Offset start(scanner.offset);
    uint8_t first = scanner.peekChar();
    if (first == $asterisk) {
      scanner.readChar();
      if (!scanner.scanChar($pipe)) {
        return SASS_MEMORY_NEW(TypeSelector,
          scanner.rawSpanFrom(start),
          "*", "", false);
      }
      if (scanner.scanChar($asterisk)) {
        return SASS_MEMORY_NEW(TypeSelector,
          scanner.rawSpanFrom(start),
          "*", "*", true);
      }
      else {
        return SASS_MEMORY_NEW(TypeSelector,
          scanner.rawSpanFrom(start),
          readIdentifier(), "*", true);
      }
    }
    else if (first == $pipe) {
      scanner.readChar();
      if (scanner.scanChar($asterisk)) {
        return SASS_MEMORY_NEW(TypeSelector,
          scanner.rawSpanFrom(start),
          "*", "", true);
      }
      else {
        return SASS_MEMORY_NEW(TypeSelector,
          scanner.rawSpanFrom(start),
          readIdentifier(), "", true);
      }
    }

    sass::string nameOrNamespace = readIdentifier();
    if (!scanner.scanChar($pipe)) {
      return SASS_MEMORY_NEW(TypeSelector,
        scanner.rawSpanFrom(start),
        std::move(nameOrNamespace),
        "", false);
    }
    else if (scanner.scanChar($asterisk)) {
      return SASS_MEMORY_NEW(TypeSelector,
        scanner.rawSpanFrom(start), "*",
        std::move(nameOrNamespace), true);
    }
    else {
      return SASS_MEMORY_NEW(TypeSelector,
        scanner.rawSpanFrom(start), readIdentifier(),
        std::move(nameOrNamespace), true);
    }

  }
  // EO readTypeOrUniversalSelector

}
