/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "parser_css.hpp"

#include "character.hpp"
#include "ast_expressions.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  // Consumes a plain-CSS `@import` rule that disallows
  // interpolation. [start] should point before the `@`.
  ImportRule* CssParser::readImportRule(Offset start)
  {
    uint8_t next = scanner.peekChar();
    ExpressionObj url;
    if (next == $u || next == $U) {
      url = readFunctionOrStringExpression();
    }
    else {
      StringExpressionObj ex(readInterpolatedString());
      InterpolationObj itpl(ex->getAsInterpolation());
      url = SASS_MEMORY_NEW(StringExpression, ex->pstate(), itpl);
    }
    scanWhitespace();
    Offset beforeQuery(scanner.offset);
    auto queries(tryImportQueries());
    expectStatementSeparator("@import rule");
    SourceSpan span(scanner.relevantSpanFrom(beforeQuery));
    ImportRuleObj imp(SASS_MEMORY_NEW(ImportRule, span));
    StaticImportObj entry(SASS_MEMORY_NEW(StaticImport, span,
      SASS_MEMORY_NEW(Interpolation, span, url),
      queries.first, queries.second));
    entry->outOfOrder(false);
    imp->append(entry);
    return imp.detach();
  }
  // EO readImportRule

  // Consume a silent comment and throws error
  SilentComment* CssParser::readSilentComment()
  {
    Offset start(scanner.offset);
    lastSilentComment = ScssParser::readSilentComment();
    error("Silent comments aren't allowed in plain CSS.",
      scanner.relevantSpanFrom(start));
    return lastSilentComment.detach();
  }
  // EO readSilentComment

  // Helper to declare all forbidden at-rules
  bool isForbiddenCssAtRule(const sass::string& name)
  {
    return name == "at-root"
      || name == "content"
      || name == "debug"
      || name == "each"
      || name == "error"
      || name == "extend"
      || name == "for"
      || name == "function"
      || name == "if"
      || name == "include"
      || name == "mixin"
      || name == "return"
      || name == "warn"
      || name == "while";
  }
  // EO isForbiddenCssAtRule

  // Parse allowed at-rule statement and parse children via [child_parser] parser function
  Statement* CssParser::readAtRule(Statement* (StylesheetParser::* child)(), bool root)
  {
    // NOTE: logic is largely duplicated in CssParser.atRule.
    // Most changes here should be mirrored there.

    Offset start(scanner.offset);
    scanner.expectChar($at);
    InterpolationObj name = readInterpolatedIdentifier();
    scanWhitespace();

    sass::string plain(name->getPlainString());

    if (isForbiddenCssAtRule(plain)) {
      InterpolationObj value = readAlmostAnyValue();
      error("This at-rule isn't allowed in plain CSS.",
        scanner.relevantSpanFrom(start));
    }
    else if (plain == "charset") {
      sass::string charset(string());
      if (!root) {
        error("This at-rule is not allowed here.",
          scanner.relevantSpanFrom(start));
      }
    }
    else if (plain == "import") {
      return readImportRule(start);
    }
    else if (plain == "media") {
      return readMediaRule(start);
    }
    else if (plain == "-moz-document") {
      return readMozDocumentRule(start, name);
    }
    else if (plain == "supports") {
      return readSupportsRule(start);
    }
    else {
      return readAnyAtRule(start, name);
    }

    return nullptr;
  }
  // EO readAtRule

  // Helper to declare all forbidden functions
  bool isDisallowedFunction(sass::string name)
  {
    return name == "red"
      || name == "green"
      || name == "blue"
      || name == "mix"
      || name == "hue"
      || name == "saturation"
      || name == "lightness"
      || name == "adjust-hue"
      || name == "lighten"
      || name == "darken"
      || name == "desaturate"
      || name == "complement"
      || name == "opacify"
      || name == "fade-in"
      || name == "transparentize"
      || name == "fade-out"
      || name == "adjust-color"
      || name == "scale-color"
      || name == "change-color"
      || name == "ie-hex-str"
      || name == "unquote"
      || name == "quote"
      || name == "str-length"
      || name == "str-insert"
      || name == "str-index"
      || name == "str-slice"
      || name == "to-upper-case"
      || name == "to-lower-case"
      || name == "percentage"
      || name == "round"
      || name == "ceil"
      || name == "floor"
      || name == "abs"
      || name == "max"
      || name == "min"
      || name == "random"
      || name == "length"
      || name == "nth"
      || name == "set-nth"
      || name == "join"
      || name == "append"
      || name == "zip"
      || name == "index"
      || name == "list-separator"
      || name == "is-bracketed"
      || name == "map-get"
      || name == "map-merge"
      || name == "map-remove"
      || name == "map-keys"
      || name == "map-values"
      || name == "map-has-key"
      || name == "keywords"
      || name == "selector-nest"
      || name == "selector-append"
      || name == "selector-extend"
      || name == "selector-replace"
      || name == "selector-unify"
      || name == "is-superselector"
      || name == "simple-selectors"
      || name == "selector-parse"
      || name == "feature-exists"
      || name == "inspect"
      || name == "type-of"
      || name == "unit"
      || name == "unitless"
      || name == "comparable"
      || name == "whiteness"
      || name == "blackness"
      || name == "if"
      || name == "unique-id";
  }
  // EO isDisallowedFunction

  // Consumes an expression that starts like an identifier.
  Expression* CssParser::readIdentifierLike()
  {
    Offset start(scanner.offset);
    InterpolationObj identifier = readInterpolatedIdentifier();
    sass::string plain(identifier->getPlainString());
    StringExpressionObj specialFunction = trySpecialFunction(plain, start);

    if (specialFunction != nullptr) {
      return specialFunction;
    }

    // Offset beforeArguments(scanner.offset);

    if (!scanner.scanChar($lparen)) {
      return SASS_MEMORY_NEW(StringExpression,
        scanner.rawSpanFrom(start), identifier);
    }

    ExpressionVector arguments;

    if (!scanner.scanChar($rparen)) {
      do {
        scanWhitespace();
        Expression* argument = readExpression(false, true);
        if (argument) arguments.emplace_back(argument);
        scanWhitespace();
      } while (scanner.scanChar($comma));
      scanner.expectChar($rparen);
    }

    // arguments->pstate(scanner.relevantSpan(start));

    if (isDisallowedFunction(plain)) {
      error(
        "This function isn't allowed in plain CSS.",
        scanner.relevantSpanFrom(start));
    }

    Interpolation* name = SASS_MEMORY_NEW(Interpolation, identifier->pstate());
    name->append(SASS_MEMORY_NEW(StringExpression, identifier->pstate(), identifier));

    ArgumentInvocation* args = SASS_MEMORY_NEW(ArgumentInvocation,
      scanner.rawSpanFrom(start), std::move(arguments), {});
    
    return SASS_MEMORY_NEW(FunctionExpression,
      scanner.rawSpanFrom(start), name, args);

  }
  // EO readIdentifierLike

}
