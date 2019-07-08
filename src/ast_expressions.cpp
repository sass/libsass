/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_expressions.hpp"
#include "interpolation.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ParentExpression::ParentExpression(
    SourceSpan&& pstate) :
    Expression(std::move(pstate))
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ValueExpression::ValueExpression(
    SourceSpan pstate,
    ValueObj value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  NullExpression::NullExpression(
    SourceSpan pstate,
    Null* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ColorExpression::ColorExpression(
    SourceSpan pstate,
    Color* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  NumberExpression::NumberExpression(
    SourceSpan pstate,
    Number* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BooleanExpression::BooleanExpression(
    SourceSpan pstate,
    Boolean* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  StringExpression::StringExpression(
    SourceSpan pstate,
    InterpolationObj text,
    bool hasQuotes) :
    Expression(std::move(pstate)),
    text_(text),
    hasQuotes_(hasQuotes)
  {}

  // Crutch instead of LiteralExpression
  // Note: really not used very often
  // ToDo: check for performance impact
  StringExpression::StringExpression(
    SourceSpan&& pstate,
    sass::string&& text,
    bool hasQuotes) :
    Expression(std::move(pstate)),
    text_(SASS_MEMORY_NEW(Interpolation, pstate,
      SASS_MEMORY_NEW(String, pstate, std::move(text)))),
    hasQuotes_(hasQuotes)
  {}

  StringExpression* Interpolation::wrapInStringExpression() {
    return SASS_MEMORY_NEW(StringExpression, pstate(), this);
  }

  // find best quote_mark by detecting if the string contains any single
  // or double quotes. When a single quote is found, we not we want a double
  // quote as quote_mark. Otherwise we check if the string contains any double
  // quotes, which will trigger the use of single quotes as best quote_mark.
  uint8_t StringExpression::findBestQuote()
  {
    using namespace Character;
    bool containsDoubleQuote = false;
    for (auto item : text_->elements()) {
      if (auto str = item->isaString()) { // Ex
        auto& value = str->value();
        for (size_t i = 0; i < value.size(); i++) {
          if (value[i] == $single_quote) return $double_quote;
          if (value[i] == $double_quote) containsDoubleQuote = true;
        }
      }
    }
    return containsDoubleQuote ? $single_quote : $double_quote;
  }

  // Interpolation that, when evaluated, produces the syntax of this string.
  // Unlike [text], his doesn't resolve escapes and does include quotes for
  // quoted strings. If [static] is true, this escapes any `#{` sequences in
  // the string. If [quote] is passed, it uses that character as the quote mark;
  // otherwise, it determines the best quote to add by looking at the string.
  // Note: [static] is not yet implemented in LibSass (find where we need it)
  InterpolationObj StringExpression::getAsInterpolation(bool escape, uint8_t quote)
  {

    using namespace Character;

    if (!hasQuotes()) return text_;

    if (!quote && hasQuotes()) quote = findBestQuote();

    InterpolationBuffer buffer(pstate());

    if (quote != 0) {
      buffer.write(quote);
    }

    for (auto value : text_->elements()) {
      if (ItplString* str = value->isaItplString()) {
        sass::string value(str->text());
        for (size_t i = 0; i < value.size(); i++) {
          uint8_t codeUnit = value[i];
          if (isNewline(codeUnit)) {
            buffer.write($backslash);
            buffer.write($a);
            if (i != value.size() - 1) {
              uint8_t next = value[i + 1];
              if (isWhitespace(next) || isHex(next)) {
                buffer.write($space);
              }
            }
          }
          else {
            if (codeUnit == quote ||
              codeUnit == $backslash ||
              (escape &&
                codeUnit == $hash &&
                i < value.size() - 1 &&
                value[i + 1] == $lbrace)) {
              buffer.write($backslash);
            }
            buffer.write(codeUnit);
          }

        }
      }
      else {
        buffer.add(value);
      }
    }

    if (quote != 0) {
      buffer.write(quote);
    }

    return buffer.getInterpolation(pstate());

  }
  // EO getAsInterpolation

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  MapExpression::MapExpression(
    SourceSpan&& pstate) :
    Expression(std::move(pstate)),
    kvlist_()
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ListExpression::ListExpression(
    SourceSpan&& pstate,
    SassSeparator separator) :
    Expression(std::move(pstate)),
    separator_(separator),
    hasBrackets_(false)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BinaryOpExpression::BinaryOpExpression(
    SourceSpan&& pstate,
    SassOperator operand,
    Expression* lhs,
    Expression* rhs,
    bool allowSlash) :
    Expression(std::move(pstate)),
    operand_(operand),
    left_(lhs),
    right_(rhs),
    allowsSlash_(allowSlash)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  VariableExpression::VariableExpression(
    SourceSpan&& pstate,
    const EnvKey& name) :
    Expression(std::move(pstate)),
    name_(name)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ParenthesizedExpression::ParenthesizedExpression(
    SourceSpan&& pstate,
    Expression* expression) :
    Expression(std::move(pstate)),
    expression_(expression)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  UnaryOpExpression::UnaryOpExpression(
    SourceSpan&& pstate,
    UnaryOpType optype,
    ExpressionObj operand) :
    Expression(std::move(pstate)),
    optype_(optype),
    operand_(operand)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  FunctionExpression::FunctionExpression(
    SourceSpan pstate,
    Interpolation* name,
    ArgumentInvocation* arguments,
    const sass::string& ns) :
    InvocationExpression(
      std::move(pstate),
      arguments),
    ns_(ns),
    name_(name),
    selfAssign_(false)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
