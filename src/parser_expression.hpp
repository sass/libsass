/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_EXPRESSION_HPP
#define SASS_PARSER_EXPRESSION_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "scanner_string.hpp"
#include "parser_expression.hpp"
#include "parser_stylesheet.hpp"

namespace Sass {

  // Helper class for stylesheet parser
  class ExpressionParser final {

    friend class StylesheetParser;
    friend class ScssParser;
    friend class CssParser;

  public:

    // Value constructor
    ExpressionParser(
      StylesheetParser& parser);

  protected:

    StringScannerState start;

    ExpressionVector commaExpressions;

    ExpressionObj singleEqualsOperand;

    ExpressionVector spaceExpressions;

    // Operators whose right-hand operands are not fully parsed yet, in order of
    // appearance in the document. Because a low-precedence operator will cause
    // parsing to finish for all preceding higher-precedence operators, this is
    // naturally ordered from lowest to highest precedence.
    sass::vector<enum SassOperator> operators;
    sass::vector<SourceSpan> opstates;
    sass::vector<bool> calcSafe;

    // The left-hand sides of [operators]. `operands[n]`
    // is the left-hand side of `operators[n]`.
    ExpressionVector operands;

    // Whether the single expression parsed so far
    // may be interpreted as slash-separated numbers.
    bool allowSlash = true;

    /// The leftmost expression that's been fully-parsed. Never `null`.
    ExpressionObj singleExpression;

    // The associated parser
    StylesheetParser& parser;

    // Resets the scanner state to the state it was at the
    // beginning of the expression, except for [_inParentheses].
    void resetState();

    void resolveOneOperation();

    void resolveOperations();

    void addSingleExpression(
      ExpressionObj expression,
      bool number = false);

    void addOperator(
      SassOperator op,
      Offset& start);

    void resolveSpaceExpressions();

  };

}

#endif
