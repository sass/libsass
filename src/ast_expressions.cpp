/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_expressions.hpp"
#include "interpolation.hpp"

#include <sstream>

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SelectorExpression::SelectorExpression(
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

  // Convert to string (only for debugging)
  sass::string ValueExpression::toString() const
  {
    return value_->inspect();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  NullExpression::NullExpression(
    SourceSpan pstate,
    Null* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  // Convert to string (only for debugging)
  sass::string NullExpression::toString() const
  {
    return "null";
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ColorExpression::ColorExpression(
    SourceSpan pstate,
    Color* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  // Convert to string (only for debugging)
  sass::string ColorExpression::toString() const
  {
    return value_->inspect();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  NumberExpression::NumberExpression(
    SourceSpan pstate,
    Number* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  // Convert to string (only for debugging)
  sass::string NumberExpression::toString() const
  {
    return value_->inspect();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BooleanExpression::BooleanExpression(
    SourceSpan pstate,
    Boolean* value) :
    Expression(std::move(pstate)),
    value_(value)
  {}

  // Convert to string (only for debugging)
  sass::string BooleanExpression::toString() const
  {
    return value_ ? "true" : "false";
  }

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

  // find best quote_mark by detecting if the string contains any single
  // or double quotes. When a single quote is found, we not we want a double
  // quote as quote_mark. Otherwise we check if the string contains any double
  // quotes, which will trigger the use of single quotes as best quote_mark.
  uint8_t StringExpression::findBestQuote() const
  {
    using namespace Character;
    bool containsDoubleQuote = false;
    for (auto item : text_->elements()) {
      if (auto str = item->isaString()) { // Ex
        const auto& value = str->value();
        for (size_t i = 0; i < value.size(); i++) {
          if (value[i] == $apos) return $quote;
          if (value[i] == $quote) containsDoubleQuote = true;
        }
      }
      else if (auto str = item->isaItplString()) { // Ex
        const auto& value = str->text();
        for (size_t i = 0; i < value.size(); i++) {
          if (value[i] == $apos) return $quote;
          if (value[i] == $quote) containsDoubleQuote = true;
        }
      }
    }
    return containsDoubleQuote ? $apos : $quote;
  }

  sass::string StringExpression::toString() const
  {
    InterpolationObj itpl = getAsInterpolation();
    return itpl->toString();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  SupportsExpression::SupportsExpression(
    SourceSpan pstate,
    SupportsCondition* condition) :
    Expression(std::move(pstate)),
    condition_(condition)
  {}

  sass::string SupportsExpression::toString() const
  {
    // auto qwe = condition_;
    sass::string msg = "(";
    // return condition_->toString();
    msg += "NO INSPECT FOR SUPPORTS EXPRESSION";
    return msg + ")";
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ItplFnExpression::ItplFnExpression(
    SourceSpan pstate,
    Interpolation* itpl,
    CallableArguments* arguments,
    const sass::string& ns) :
    InvocationExpression(
      std::move(pstate), arguments),
    itpl_(itpl),
    ns_(ns)
  {}

  // Interpolation that, when evaluated, produces the syntax of this string.
  // Unlike [text], his doesn't resolve escapes and does include quotes for
  // quoted strings. If [static] is true, this escapes any `#{` sequences in
  // the string. If [quote] is passed, it uses that character as the quote mark;
  // otherwise, it determines the best quote to add by looking at the string.
  // Note: [static] is not yet implemented in LibSass (find where we need it)
  InterpolationObj StringExpression::getAsInterpolation(bool escape, uint8_t quote) const
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

  // Convert to string (only for debugging)
  sass::string MapExpression::toString() const
  {
    sass::sstream strm;
    strm << Character::$lparen;
    for (size_t i = 0; i < kvlist_.size(); i += 2) {
      if (i != 0) strm << ", ";
      strm << kvlist_[i]->toString() << ": ";
      strm << kvlist_[i + 1]->toString();
    }
    strm << Character::$rparen;
    return strm.str();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ListExpression::ListExpression(
    SourceSpan&& pstate,
    SassSeparator separator) :
    Expression(std::move(pstate)),
    separator_(separator),
    hasBrackets_(false)
  {}

  // Convert to string (only for debugging)
  sass::string ListExpression::toString() const
  {
    StringVector parts;
    for (auto& part : items_) {
      parts.emplace_back(part->toString());
    }
    return StringUtils::join(parts,
      sass_list_separator(separator_));
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BinaryOpExpression::BinaryOpExpression(
    SourceSpan&& pstate,
    SassOperator operand,
    SourceSpan&& opstate,
    Expression* lhs,
    Expression* rhs,
    bool allowSlash,
    bool isCalcSafeOp) :
    Expression(std::move(pstate)),
    operand_(operand),
    opstate_(opstate),
    left_(lhs),
    right_(rhs),
    allowsSlash_(allowSlash),
    warned_(false),
    isCalcSafeOp_(isCalcSafeOp)
  {}

  // Convert to string (only for debugging)
  sass::string BinaryOpExpression::toString() const
  {
    if (operand_ == SassOperator::DIV) {
      sass::sstream buffer;
      buffer << "math.div(";
      auto left = this->left(); // Hack to make analysis work.
      auto lhs = left->isaBinaryOpExpression();
      auto leftNeedsParens = lhs && sass_op_to_precedence(lhs->operand_) < sass_op_to_precedence(operand_);
      if (leftNeedsParens) buffer << Character::$lparen;
      buffer << left->toString();
      if (leftNeedsParens) buffer << Character::$rparen;
      buffer << ", ";
      auto right = this->right(); // Hack to make analysis work.
      auto rhs = right->isaBinaryOpExpression();
      auto rightNeedsParens = rhs && sass_op_to_precedence(rhs->operand_) <= sass_op_to_precedence(operand_);
      if (rightNeedsParens) buffer << Character::$lparen;
      buffer << right->toString();
      if (rightNeedsParens) buffer << Character::$rparen;
      buffer << ")";
      return buffer.str();
    }
    else {
      sass::sstream buffer;
      auto left = this->left(); // Hack to make analysis work.
      auto lhs = left->isaBinaryOpExpression();
      auto leftNeedsParens = lhs && sass_op_to_precedence(lhs->operand_) < sass_op_to_precedence(operand_);
      if (leftNeedsParens) buffer << Character::$lparen;
      buffer << left->toString();
      if (leftNeedsParens) buffer << Character::$rparen;
      if (operand_ != SassOperator::IESEQ) buffer << Character::$space;
      buffer << sass_op_separator(operand_);
      if (operand_ != SassOperator::IESEQ) buffer << Character::$space;
      auto right = this->right(); // Hack to make analysis work.
      auto rhs = right->isaBinaryOpExpression();
      auto rightNeedsParens = rhs && sass_op_to_precedence(rhs->operand_) <= sass_op_to_precedence(operand_);
      if (rightNeedsParens) buffer << Character::$lparen;
      buffer << right->toString();
      if (rightNeedsParens) buffer << Character::$rparen;
      return buffer.str();
    }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  VariableExpression::VariableExpression(
    SourceSpan&& pstate,
    const EnvKey& name,
    const sass::string& ns) :
    Expression(std::move(pstate)),
    name_(name),
    ns_(ns)
  {}

  // Convert to string (only for debugging)
  sass::string VariableExpression::toString() const
  {
    return "$" + name_.norm();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ParenthesizedExpression::ParenthesizedExpression(
    SourceSpan&& pstate,
    Expression* expression) :
    Expression(std::move(pstate)),
    expression_(expression)
  {}

  // Convert to string (only for debugging)
  sass::string ParenthesizedExpression::toString() const {
    return "(" + expression_->toString() + ")";
  }

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

  // Convert to string (only for debugging)
  sass::string UnaryOpExpression::toString() const
  {
    sass::sstream strm;
    switch (optype_) {
    case PLUS: strm << Character::$plus; break;
    case MINUS: strm << Character::$minus; break;
    case SLASH: strm << Character::$slash; break;
    default: break;
    }
    if (optype_ == NOT) {
      strm << Character::$space;
    }
    strm << operand_->toString();
    return strm.str();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  FunctionExpression::FunctionExpression(
    SourceSpan pstate,
    const sass::string& name,
    CallableArguments* arguments,
    const sass::string& ns) :
    InvocationExpression(
      std::move(pstate),
      arguments),
    ns_(ns),
    name_(name)
  {}

  // Convert to string (only for debugging)
  sass::string FunctionExpression::toString() const
  {
    sass::sstream strm;
    if (!ns_.empty()) {
      strm << ns_ << "::";
    }
    strm << name_;
    strm << Character::$lparen;
    strm << InvocationExpression::toString();
    strm << Character::$rparen;
    return strm.str();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Convert to string (only for debugging)
  sass::string IfExpression::toString() const
  {
    return "if(" + InvocationExpression::toString() + ")";
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Convert to string (only for debugging)
  sass::string InvocationExpression::toString() const
  {
    StringVector components;
    for (auto positional : arguments_->positional()) {
      components.emplace_back(positional->toString());
    }
    for (auto& name : arguments_->named()) {
      sass::sstream strm;
      strm << name.first.norm() << ": ";
      strm << name.second->toString();
      components.push_back(strm.str());
    }
    if (arguments_->restArg()) {
      sass::sstream strm;
      strm << arguments_->restArg()->toString() << "...";
      components.push_back(strm.str());
    }
    if (arguments_->kwdRest()) {
      sass::sstream strm;
      strm << arguments_->kwdRest()->toString() << "...";
      components.push_back(strm.str());
    }
    return StringUtils::join(components, ", ");
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

namespace Sass {

  bool isMathOperator(SassOperator op)
  {
    if (op == MUL) return true;
    if (op == DIV) return true;
    if (op == ADD) return true;
    if (op == SUB) return true;
    return false;
  }

  bool SelectorExpression::isCalcSafe()
  {
    return false;
  }

  bool BinaryOpExpression::isCalcSafe()
  {
    if (isMathOperator(operand())) {
      return left()->isCalcSafe()
        || right()->isCalcSafe();
    }
    return false;
  }

  bool ListExpression::isCalcSafe()
  {
    if (separator() != SASS_SPACE) return false;
    if (hasBrackets() == true) return false;
    if (size() < 2) return false;
    for (auto asd : items()) {
      if (!asd->isCalcSafe())
        return false;
    }
    return true;
  }

  bool ParenthesizedExpression::isCalcSafe()
  {
    return expression()->isCalcSafe();
  }

  bool StringExpression::isCalcSafe()
  {
    if (hasQuotes_) return false;
    auto& str = text()->getInitialPlain();
    if (str.size() > 0 && str[0] == '!') return false;
    if (str.size() > 0 && str[0] == '#') return false;
    if (str.size() > 1 && str[1] == '+') return false;
    if (str.size() > 3 && str[3] == '(') return false;
    // Requires a bit more testings
    return true;
  }

}
