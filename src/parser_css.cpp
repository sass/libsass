/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "parser_css.hpp"

#include "character.hpp"
#include "ast_imports.hpp"
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
    auto modifiers(tryImportModifiers());
    // auto queries(tryImportQueries());
    expectStatementSeparator("@import rule");
    SourceSpan span(scanner.relevantSpanFrom(beforeQuery));
    ImportRuleObj imp(SASS_MEMORY_NEW(ImportRule, span));
    StaticImportObj entry(SASS_MEMORY_NEW(StaticImport, span,
      SASS_MEMORY_NEW(Interpolation, span, url), modifiers,
      true));
    entry->outOfOrder(false);
    imp->append(entry.ptr());
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

  // Consume a silent comment and throws error
  void CssParser::scanSilentComment()
  {
    Offset start(scanner.offset);
    lastSilentComment = ScssParser::readSilentComment();
    error("Silent comments aren't allowed in plain CSS.",
      scanner.relevantSpanFrom(start));
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
    return name == str_red
      || name == str_green
      || name == str_blue
      || name == str_mix
      || name == str_hue
      || name == str_saturation
      || name == str_lightness
      || name == str_adjust_hue
      || name == str_lighten
      || name == str_darken
      || name == str_desaturate
      || name == str_complement
      || name == str_opacify
      || name == str_fade_in
      || name == str_transparentize
      || name == str_fade_out
      || name == str_adjust_color
      || name == str_scale_color
      || name == str_change_color
      || name == str_ie_hex_str
      || name == str_unquote
      || name == str_quote
      || name == str_str_length
      || name == str_str_insert
      || name == str_str_index
      || name == str_str_slice
      || name == str_to_upper_case
      || name == str_to_lower_case
      || name == str_percentage
      || name == str_round
      || name == str_ceil
      || name == str_floor
      || name == str_abs
      || name == str_max
      || name == str_min
      || name == str_random
      || name == str_length
      || name == str_nth
      || name == str_set_nth
      || name == str_join
      || name == str_append
      || name == str_zip
      || name == str_index
      || name == str_list_separator
      || name == str_is_bracketed
      || name == str_map_get
      || name == str_map_merge
      || name == str_map_remove
      || name == str_map_keys
      || name == str_map_values
      || name == str_map_has_key
      || name == str_keywords
      || name == str_selector_nest
      || name == str_selector_append
      || name == str_selector_extend
      || name == str_selector_replace
      || name == str_selector_unify
      || name == str_is_superselector
      || name == str_simple_selectors
      || name == str_selector_parse
      || name == str_feature_exists
      || name == str_inspect
      || name == str_type_of
      || name == str_unit
      || name == str_unitless
      || name == str_comparable
      || name == str_whiteness
      || name == str_blackness
      || name == str_if
      || name == str_unique_id;
  }
  // EO isDisallowedFunction

  // Expression* CssParser::namespacedExpression(sass::string ns, Offset start) {
  //   auto expression = StylesheetParser::readNamespacedExpression(ns, start);
  //   error("Module namespaces aren't allowed in plain CSS.", expression->pstate());
  //   return nullptr;
  // }

  Expression* CssParser::readParenthesizedExpression() {
    // Expressions are only allowed within calculations, but we verify this at
    // evaluation time.
    Offset start(scanner.offset);
    scanner.expectChar($lparen);
    scanWhitespace();
    Expression* expression = readExpressionUntilComma();
    scanner.expectChar($rparen);
    return SASS_MEMORY_NEW(ParenthesizedExpression,
      scanner.relevantSpanFrom(start), expression);
  }


  // Consumes an expression that starts like an identifier.
  Expression* CssParser::readIdentifierLike()
  {
    Offset start(scanner.offset);
    InterpolationObj identifier = readInterpolatedIdentifier();
    sass::string plain(identifier->getPlainString());
    auto lower = StringUtils::toLowerCase(plain);
    StringExpressionObj specialFunction = trySpecialFunction(lower, start);

    if (specialFunction != nullptr) {
      return specialFunction.detach();
    }

    // Offset beforeArguments(scanner.offset);

    if (scanner.scanChar($dot)) {
      return readNamespacedExpression(plain, start);
    }

    if (!scanner.scanChar($lparen)) {
      return SASS_MEMORY_NEW(StringExpression,
        scanner.rawSpanFrom(start), identifier);
    }

    bool allowEmptySecondArg = (lower == "var");

    if (auto specialFunction = trySpecialFunction(lower, start)) {
      return specialFunction;
    }

    ExpressionVector arguments;

    if (!scanner.scanChar($rparen)) {
      do {
        scanWhitespace();
        if (allowEmptySecondArg && arguments.size() == 1 && scanner.peekChar() == $rparen) {
          arguments.push_back(SASS_MEMORY_NEW(StringExpression, scanner.rawSpan(), ""));
          break;
        }
        Expression* argument = readExpressionUntilComma(true);
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

    CallableArguments* args = SASS_MEMORY_NEW(CallableArguments,
      scanner.rawSpanFrom(start), std::move(arguments), {});
    
    // Plain Css as it's interpolated
    // if (name->getPlainString().empty()) {
    //   return SASS_MEMORY_NEW(ItplFnExpression,
    //     scanner.relevantSpanFrom(start), name, args, "");
    // }

    return SASS_MEMORY_NEW(FunctionExpression,
      scanner.rawSpanFrom(start), plain, args);

  }
  // EO readIdentifierLike

  Expression* CssParser::readNamespacedExpression(
    const sass::string& ns, Offset start)
  {
    SourceSpan pstate(scanner.relevantSpanFrom(start));
    ExpressionObj expression = ScssParser::readNamespacedExpression(ns, start);
    error("Module namespaces aren't allowed in plain CSS.", pstate);
    return expression;
  }
}
