/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_EXPRESSIONS_HPP
#define SASS_AST_EXPRESSIONS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_values.hpp"
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

  class SelectorExpression final : public Expression
  {

  public:

    // Value constructor
    SelectorExpression(
      SourceSpan&& pstate);

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitSelectorExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final;

    // Convert to string (only for debugging)
    sass::string toString() const override final
    {
      return "&";
    }

  };

  /////////////////////////////////////////////////////////////////////////
  // An expression that directly embeds a [Value]. This is never
  // constructed by the parser. It's only used when ASTs are
  // constructed dynamically, as for the `call()` function.
  /////////////////////////////////////////////////////////////////////////

  class ValueExpression final : public Expression
  {

  private:

    // Value wrapped inside this expression
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

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(ValueExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // The Null Expression.
  /////////////////////////////////////////////////////////////////////////

  class NullExpression final : public Expression
  {
  private:

    // Object can still hold a SourceSpan
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

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(NullExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // The Color Expression.
  /////////////////////////////////////////////////////////////////////////

  class ColorExpression final : public Expression
  {
  private:

    // Color wrapped inside this expression
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

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(ColorExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // The Number Expression.
  /////////////////////////////////////////////////////////////////////////

  class NumberExpression final : public Expression
  {
  private:

    // Number wrapped inside this expression
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

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return true; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(NumberExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // The Boolean Expression.
  /////////////////////////////////////////////////////////////////////////

  class BooleanExpression final : public Expression
  {
  private:

    // Boolean wrapped inside this expression
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

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(BooleanExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // String expression holding an optionally quoted interpolation
  /////////////////////////////////////////////////////////////////////////
  class StringExpression final : public Expression
  {
  private:

    // The interpolation forming this string
    ADD_CONSTREF(InterpolationObj, text);

    // Flag whether the result must be quoted
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
      uint8_t quote = 0) const;

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitStringExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final;

  private:

    // find best quote_mark by detecting if the string contains any single
    // or double quotes. When a single quote is found, we not we want a double
    // quote as quote_mark. Otherwise we check if the string contains any double
    // quotes, which will trigger the use of single quotes as best quote_mark.
    uint8_t findBestQuote() const;

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(StringExpression);
  };


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class SupportsExpression final : public Expression
  {

    // The condition itself
    ADD_CONSTREF(SupportsConditionObj, condition);

  public:

    // Value constructor
    SupportsExpression(SourceSpan pstate,
      SupportsCondition* condition);

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitSupportsExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(SupportsExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Map expression holds an even list of key and value expressions.
  /////////////////////////////////////////////////////////////////////////

  class MapExpression final : public Expression
  {
  private:

    // We can't create a map with expression, since the keys are
    // not yet resolved and we therefore don't know if any of them
    // are duplicates or not. Therefore we store all key and value
    // expressions in a vector, which must always be of even size.
    ADD_CONSTREF(ExpressionVector, kvlist);

  public:

    // Value constructor
    MapExpression(
      SourceSpan&& pstate);

    // Append key or value. You must ensure to always call
    // this method twice for every key-value pair. Otherwise
    // it will result in undefined behavior!
    void append(Expression* expression) {
      kvlist_.emplace_back(expression);
    }

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitMapExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(MapExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // List expression holds a vector of value expressions.
  /////////////////////////////////////////////////////////////////////////

  class ListExpression final : public Expression
  {
  private:

    // The vector containing all children
    ADD_CONSTREF(ExpressionVector, items);

    // The separator to use when rendering the list
    ADD_CONSTREF(SassSeparator, separator);

    // Flag to wrap list in brackets
    ADD_CONSTREF(bool, hasBrackets);

  public:

    // Value constructor
    ListExpression(
      SourceSpan&& pstate,
      SassSeparator separator = SASS_UNDEF);

    // Append a single expression
    void append(Expression* expression) {
      items_.emplace_back(expression);
    }

    // Move items into our vector (append)
    void concat(ExpressionVector&& expressions) {
      items_.insert(items_.end(),
        std::make_move_iterator(expressions.begin()),
        std::make_move_iterator(expressions.end()));
    }

    // Return number of items
    size_t size() const {
      return items_.size();
    }

    // Return expression at position
    Expression* get(size_t position) {
      return items_[position];
    }

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitListExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final;

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(ListExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Arithmetic negation (logical negation is just an ordinary function call).
  /////////////////////////////////////////////////////////////////////////

  class UnaryOpExpression final : public Expression
  {
  private:

    // Type of unary op (minus, plus etc.)
    ADD_CONSTREF(UnaryOpType, optype);

    // Operand following the unary operation
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

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return false; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(UnaryOpExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Binary expressions. Represents logical, relational, and arithmetic ops.
  // Templatized to avoid large switch statements and repetitive sub-classing.
  /////////////////////////////////////////////////////////////////////////

  class BinaryOpExpression final : public Expression
  {
  private:

    // Operand to apply to left and right hand side
    ADD_CONSTREF(SassOperator, operand);

    // Parser state for the operator
    ADD_CONSTREF(SourceSpan, opstate);

    // Left hand side of the operation
    ADD_CONSTREF(ExpressionObj, left);

    // Right hand side of the operation
    ADD_CONSTREF(ExpressionObj, right);

    // Flag to delay divisions as necessary, since certain
    // valid css settings can look like divisions to sass
    // E.g. font: 12px/14px sans-serif
    ADD_CONSTREF(bool, allowsSlash);

    // Flag if a warning was emitted (only emit once)
    ADD_CONSTREF(bool, warned);

    // Is calculation safe (for add and sub)
    ADD_CONSTREF(bool, isCalcSafeOp);

    // Obsolete: may be needed for output formats
    // ADD_CONSTREF(bool, ws_before);
    // ADD_CONSTREF(bool, ws_after);

  public:

    // Value constructor
    BinaryOpExpression(
      SourceSpan&& pstate,
      SassOperator operand,
      SourceSpan&& opstate,
      Expression* lhs,
      Expression* rhs,
      bool allowSlash = false,
      bool isCalcSafe = true);

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitBinaryOpExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final;

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(BinaryOpExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // A lexical variable referencing an expression that
  // was previously assigned to the named variable. If
  // no variable by this name is found an error it thrown.
  /////////////////////////////////////////////////////////////////////////

  class VariableExpression final : public Expression
  {
  private:

    // The name of the variable (without the dollar sign)
    ADD_CONSTREF(EnvKey, name);

    // Cached env references populated during runtime
    ADD_REF(sass::vector<EnvRef>, vidxs);

    // Optional module namespace
    ADD_CONSTREF(sass::string, ns);

  public:

    // Value constructor
    VariableExpression(
      SourceSpan&& pstate,
      const EnvKey& name,
      const sass::string& ns = "");

    // Return if variable is lexical
    // Module variables are at the root
    inline bool isLexical() const {
      return ns_.empty();
    }

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitVariableExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return true; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(VariableExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // An expression that must be wrapped in parentheses.
  /////////////////////////////////////////////////////////////////////////

  class ParenthesizedExpression final : public Expression
  {
  private:

    // Expression to be wrapped in parentheses
    ADD_CONSTREF(ExpressionObj, expression)

  public:

    // Value constructor
    ParenthesizedExpression(
      SourceSpan&& pstate,
      Expression* expression);

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitParenthesizedExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final;

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(ParenthesizedExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Base class for `IfExpression` and `FunctionExpression`
  /////////////////////////////////////////////////////////////////////////

  class InvocationExpression : public Expression
  {
  private:

    // Arguments passed to this invocation
    ADD_CONSTREF(CallableArgumentsObj, arguments);

  public:

    // Value constructor
    InvocationExpression(SourceSpan&& pstate,
      CallableArguments* arguments) :
      Expression(std::move(pstate)),
      arguments_(arguments)
    {}

    // Convert to string (only for debugging)
    virtual sass::string toString() const override;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(IfExpression);
    DECLARE_ISA_CASTER(FunctionExpression);
    DECLARE_ISA_CASTER(ItplFnExpression);
    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(InvocationExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // A plain css function (not executed, simply rendered back in)
  /////////////////////////////////////////////////////////////////////////

  class ItplFnExpression final : public InvocationExpression
  {
  private:

    // Interpolation forming the function name
    ADD_CONSTREF(InterpolationObj, itpl);

    // Optional module namespace
    ADD_CONSTREF(sass::string, ns);

  public:

    // Value constructor
    ItplFnExpression(
      SourceSpan pstate,
      Interpolation* itpl,
      CallableArguments* args,
      const sass::string& ns);

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitItplFnExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return true; }

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(ItplFnExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Ternary expression to either return left or right and evaluated.
  /////////////////////////////////////////////////////////////////////////

  class IfExpression final : public InvocationExpression
  {
  public:

    // Value constructor
    IfExpression(SourceSpan pstate,
      CallableArguments* arguments) :
      InvocationExpression(std::move(pstate), arguments)
    {}

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitIfExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return true; }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(IfExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Expression to invoke a function or if the function
  // is not defined, renders as a plain css function.
  /////////////////////////////////////////////////////////////////////////

  class FunctionExpression final : public InvocationExpression
  {

    // The namespace of the function being invoked,
    // or `null` if it's invoked without a namespace.
    ADD_CONSTREF(sass::string, ns);

    // The name of the function being invoked. If this is
    // interpolated, the function will be interpreted as plain
    // CSS, even if it has the same name as a Sass function.
    ADD_CONSTREF(sass::string, name);

    // Stack reference to function
    ADD_CONSTREF(EnvRef, fidx);

  public:

    // Value constructor
    FunctionExpression(SourceSpan pstate,
      const sass::string& name,
      CallableArguments* arguments,
      const sass::string& ns = "");

    // Expression visitor to sass values entry function
    Value* accept(ExpressionVisitor<Value*>* visitor) override final {
      return visitor->visitFunctionExpression(this);
    }

    // Return if expression can be used in calculations
    bool isCalcSafe() override final { return true; }

    // Imports are transparent for variables, functions and mixins
    // We always need to create entities inside the parent scope
    bool isImport() const { return fidx_.isImport (); }

    // Flag if this scope is considered internal
    bool isInternal() const { return fidx_.isInternal(); }

    // Rules like `@if`, `@for` etc. are semi-global (permeable).
    // Assignments directly in those can bleed to the root scope.
    bool isSemiGlobal() const { return fidx_.isSemiGlobal(); }

    // Set to true once we are compiled via use or forward
    // An import does load the sheet, but does not compile it
    // Compiling it means hard-baking the config vars into it
    bool isCompiled() const { return fidx_.isCompiled(); }

    // Convert to string (only for debugging)
    sass::string toString() const override final;

    // Implement specialized up-casting method
    IMPLEMENT_ISA_CASTER(FunctionExpression);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
