/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_EXPRESSIONS_HPP
#define SASS_AST_EXPRESSIONS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_callables.hpp"
#include "environment_stack.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Enum for UnaryOpExpression (value prefix)
  enum UnaryOpType { PLUS, MINUS, NOT, SLASH };

  /////////////////////////////////////////////////////////////////////////
  // The Parent Reference Expression.
  /////////////////////////////////////////////////////////////////////////
  class ParentExpression final : public Expression
  {
  public:
    // Value constructor
    ParentExpression(
      SourceSpan&& pstate);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitParentExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // An expression that directly embeds a [Value]. This is never
  // constructed by the parser. It's only used when ASTs are
  // constructed dynamically, as for the `call()` function.
  /////////////////////////////////////////////////////////////////////////
  class ValueExpression final : public Expression
  {
    ADD_CONSTREF(ValueObj, value);
  public:
    // Value constructor
    ValueExpression(
      SourceSpan pstate,
      ValueObj value);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitValueExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // The Null Expression.
  /////////////////////////////////////////////////////////////////////////
  class NullExpression final : public Expression
  {
    ADD_CONSTREF(NullObj, value);
  public:
    // Value constructor
    NullExpression(
      SourceSpan pstate,
      Null* value);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitNullExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // The Color Expression.
  /////////////////////////////////////////////////////////////////////////
  class ColorExpression final : public Expression
  {
    ADD_CONSTREF(ColorObj, value);
  public:
    // Value constructor
    ColorExpression(
      SourceSpan pstate,
      Color* color);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitColorExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // The Number Expression.
  /////////////////////////////////////////////////////////////////////////
  class NumberExpression final : public Expression
  {
    ADD_CONSTREF(NumberObj, value);
  public:
    // Value constructor
    NumberExpression(
      SourceSpan pstate,
      Number* value);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitNumberExpression(this);
    }
    IMPLEMENT_ISA_CASTER(NumberExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // The Boolean Expression.
  /////////////////////////////////////////////////////////////////////////
  class BooleanExpression final : public Expression
  {
    ADD_CONSTREF(BooleanObj, value);
  public:
    // Value constructor
    BooleanExpression(
      SourceSpan pstate,
      Boolean* value);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitBooleanExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // String expression holding an optionally quoted interpolation
  /////////////////////////////////////////////////////////////////////////
  class StringExpression final : public Expression
  {
    ADD_CONSTREF(InterpolationObj, text);
    ADD_CONSTREF(bool, hasQuotes);
  public:
    // Value constructor
    StringExpression(
      SourceSpan pstate,
      InterpolationObj text,
      bool hasQuotes = false);
    // Crutch instead of LiteralExpression
    // Note: really not used very often
    // ToDo: check for performance impact
    StringExpression(
      SourceSpan&& pstate,
      sass::string&& text,
      bool hasQuotes = false);
    // Interpolation that, when evaluated, produces the syntax of this string.
    // Unlike [text], his doesn't resolve escapes and does include quotes for
    // quoted strings. If [static] is true, this escapes any `#{` sequences in
    // the string. If [quote] is passed, it uses that character as the quote mark;
    // otherwise, it determines the best quote to add by looking at the string.
    InterpolationObj getAsInterpolation(
      bool escape = false,
      uint8_t quote = 0);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitStringExpression(this);
    }
  private:

    // find best quote_mark by detecting if the string contains any single
    // or double quotes. When a single quote is found, we not we want a double
    // quote as quote_mark. Otherwise we check if the string contains any double
    // quotes, which will trigger the use of single quotes as best quote_mark.
    uint8_t findBestQuote();

  };

  /////////////////////////////////////////////////////////////////////////
  // Map expression hold an even list of key and value expressions.
  /////////////////////////////////////////////////////////////////////////
  class MapExpression final : public Expression
  {
    ADD_CONSTREF(ExpressionVector, kvlist);
  public:
    // Value constructor
    MapExpression(
      SourceSpan&& pstate);
    // Append key or value
    void append(Expression* expression) {
      kvlist_.emplace_back(expression);
    }
    // Return number of items
    size_t size() const {
      return kvlist_.size();
    }
    // Return expression at position
    Expression* get(size_t position) {
      return kvlist_[position];
    }
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitMapExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  class ListExpression final : public Expression
  {
    ADD_CONSTREF(ExpressionVector, contents);
    ADD_CONSTREF(SassSeparator, separator);
    ADD_CONSTREF(bool, hasBrackets);
  public:
    // Value constructor
    ListExpression(
      SourceSpan&& pstate,
      SassSeparator separator = SASS_UNDEF);
    // Append a single expression
    void append(Expression* expression) {
      contents_.emplace_back(expression);
    }
    // Move items into our vector (append)
    void concat(ExpressionVector&& expressions) {
      contents_.insert(contents_.end(),
        std::make_move_iterator(expressions.begin()),
        std::make_move_iterator(expressions.end()));
    }
    // Return number of items
    size_t size() const {
      return contents_.size();
    }
    // Return expression at position
    Expression* get(size_t position) {
      return contents_[position];
    }
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitListExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // Arithmetic negation (logical negation is just an ordinary function call).
  /////////////////////////////////////////////////////////////////////////

  class UnaryOpExpression final : public Expression
  {
    ADD_CONSTREF(UnaryOpType, optype);
    ADD_CONSTREF(ExpressionObj, operand);
  public:
    // Value constructor
    UnaryOpExpression(
      SourceSpan&& pstate,
      UnaryOpType optype,
      ExpressionObj operand);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitUnaryOpExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // Binary expressions. Represents logical, relational, and arithmetic ops.
  // Templatized to avoid large switch statements and repetitive sub-classing.
  /////////////////////////////////////////////////////////////////////////
  class BinaryOpExpression final : public Expression
  {
    ADD_CONSTREF(SassOperator, operand);
    ADD_CONSTREF(ExpressionObj, left);
    ADD_CONSTREF(ExpressionObj, right);
    // ADD_CONSTREF(bool, ws_before);
    // ADD_CONSTREF(bool, ws_after);
    ADD_CONSTREF(bool, allowsSlash);
  public:
    // Value constructor
    BinaryOpExpression(
      SourceSpan&& pstate,
      SassOperator operand,
      Expression* lhs,
      Expression* rhs,
      bool allowSlash = false);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitBinaryOpExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // VariableExpression references.
  /////////////////////////////////////////////////////////////////////////
  class VariableExpression final : public Expression
  {
    ADD_CONSTREF(EnvKey, name);
    ADD_REF(sass::vector<VarRef>, vidxs);
  public:
    // Value constructor
    VariableExpression(
      SourceSpan&& pstate,
      const EnvKey& name);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitVariableExpression(this);
    }
    IMPLEMENT_ISA_CASTER(VariableExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  class ParenthesizedExpression final : public Expression
  {
    ADD_CONSTREF(ExpressionObj, expression)
  public:
    ParenthesizedExpression(
      SourceSpan&& pstate,
      Expression* expression);
    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitParenthesizedExpression(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  // Base class for `IfExpression` and `FunctionExpression`
  /////////////////////////////////////////////////////////////////////////

  class InvocationExpression :
    public CallableInvocation,
    public Expression
  {
  public:
    InvocationExpression(SourceSpan&& pstate,
      ArgumentInvocation* arguments) :
      CallableInvocation(arguments),
      Expression(std::move(pstate))
    {}
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class IfExpression final : public InvocationExpression
  {
  public:
    static ArgumentDeclarationObj prototype;
    IfExpression(SourceSpan pstate,
      ArgumentInvocation* arguments) :
      InvocationExpression(std::move(pstate), arguments)
    {
    }

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitIfExpression(this);
    }

  };

  // This may be a plain CSS function or a Sass function.
  class FunctionExpression final : public InvocationExpression
  {

    // The namespace of the function being invoked,
    // or `null` if it's invoked without a namespace.
    ADD_CONSTREF(sass::string, ns);

    // The name of the function being invoked. If this is
    // interpolated, the function will be interpreted as plain
    // CSS, even if it has the same name as a Sass function.
    ADD_CONSTREF(InterpolationObj, name);

    // The frame offset for the function
    ADD_CONSTREF(VarRef, fidx);

    // Internal optimization flag
    ADD_CONSTREF(bool, selfAssign);

  public:
    FunctionExpression(SourceSpan pstate,
      Interpolation* name,
      ArgumentInvocation* arguments,
      const sass::string& ns = "");

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitFunctionExpression(this);
    }

    IMPLEMENT_ISA_CASTER(FunctionExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
