/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_CALLABLES_HPP
#define SASS_AST_CALLABLES_HPP

// sass.hpp must go before all system headers
// to get the  __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_nodes.hpp"
#include "environment_key.hpp"
#include "environment_stack.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  typedef Value* (*SassFnSig)(FN_PROTOTYPE2);
  typedef std::pair<ArgumentDeclarationObj, SassFnSig> SassFnPair;
  typedef sass::vector<SassFnPair> SassFnPairs;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class Callable : public AstNode
  {
  public:

    // Value constructor
    Callable(const SourceSpan& pstate);

    // The main entry point to execute the function (implemented in each specialization)
    virtual Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate, bool selfAssign) = 0;

    // Return the function name
    virtual const sass::string& name() const = 0;

    // Equality comparator (needed for `get-function` value)
    virtual bool operator==(const Callable& rhs) const = 0;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(BuiltInCallable);
    DECLARE_ISA_CASTER(BuiltInCallables);
    DECLARE_ISA_CASTER(PlainCssCallable);
    DECLARE_ISA_CASTER(UserDefinedCallable);
    DECLARE_ISA_CASTER(ExternalCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class BuiltInCallable final : public Callable {

    // The function name
    ADD_CONSTREF(EnvKey, envkey);

    ADD_CONSTREF(ArgumentDeclarationObj, parameters);

    ADD_REF(SassFnPair, function);

  public:

    // Creates a callable with a single [arguments] declaration
    // and a single [callback]. The argument declaration is parsed
    // from [arguments], which should not include parentheses.
    // Throws a [SassFormatException] if parsing fails.
    BuiltInCallable(
      const EnvKey& fname,
      ArgumentDeclaration* parameters,
      const SassFnSig& callback);

    // Return callback with matching signature
    const SassFnPair& callbackFor(
      const ArgumentResults& evaluated);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate, bool selfAssign) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    IMPLEMENT_ISA_CASTER(BuiltInCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class BuiltInCallables final : public Callable {

    // The function name
    ADD_CONSTREF(EnvKey, envkey);

    // The overloads declared for this callable.
    ADD_CONSTREF(SassFnPairs, overloads);

  public:

    // Creates a callable with multiple implementations. Each
    // key/value pair in [overloads] defines the argument declaration
    // for the overload (which should not include parentheses), and
    // the callback to execute if that argument declaration matches.
    // Throws a [SassFormatException] if parsing fails.
    BuiltInCallables(
      const EnvKey& envkey,
      const SassFnPairs& overloads);

    // Return callback with matching signature
    const SassFnPair& callbackFor(
      const ArgumentResults& evaluated);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate, bool selfAssign) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    IMPLEMENT_ISA_CASTER(BuiltInCallables);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class PlainCssCallable final : public Callable
  {
  private:

    ADD_CONSTREF(sass::string, fname);

  public:

    // Value constructor
    PlainCssCallable(
      const SourceSpan& pstate,
      const sass::string& fname);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate, bool selfAssign) override final;

    // Return the function name
    const sass::string& name() const override final { return fname(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    IMPLEMENT_ISA_CASTER(PlainCssCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class UserDefinedCallable final : public Callable
  {
  private:

    // Name of this callable (used for reporting)
    ADD_CONSTREF(EnvKey, envkey);
    // The declaration (parameters this function takes).
    ADD_CONSTREF(CallableDeclarationObj, declaration);
    // The environment in which this callable was declared.
    ADD_PROPERTY(UserDefinedCallable*, content);

  public:

    // Value constructor
    UserDefinedCallable(
      const SourceSpan& pstate,
      const EnvKey& fname,
      CallableDeclarationObj declaration,
      UserDefinedCallable* content);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate, bool selfAssign) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    IMPLEMENT_ISA_CASTER(UserDefinedCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class ExternalCallable final : public Callable
  {
  private:

    // Name of this callable (used for reporting)
    ADD_CONSTREF(EnvKey, envkey);
    // The declaration (parameters this function takes).
    ADD_CONSTREF(ArgumentDeclarationObj, declaration);
    // The attached external callback reference
    ADD_PROPERTY(struct SassFunction*, function);

  public:

    // Value constructor
    ExternalCallable(
      const EnvKey& fname,
      ArgumentDeclaration* parameters,
      struct SassFunction* function);

    // Destructor
    ~ExternalCallable() override final {
      sass_delete_function(function_);
    }

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate, bool selfAssign) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    IMPLEMENT_ISA_CASTER(ExternalCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  // Individual argument objects for mixin and function calls.
  /////////////////////////////////////////////////////////////////////////
  class Argument final : public AstNode
  {
  private:

    ADD_CONSTREF(EnvKey, name);
    ADD_CONSTREF(ExpressionObj, defval);
    ADD_CONSTREF(bool, is_rest_argument);
    ADD_CONSTREF(bool, is_keyword_argument);

  public:

    // Value constructor
    Argument(const SourceSpan& pstate,
      const EnvKey& name,
      ExpressionObj defval,
      bool is_rest_argument = false,
      bool is_keyword_argument = false);

  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ArgumentDeclaration final : public AstNode
  {
  private:

    // The arguments that are taken.
    ADD_REF(sass::vector<ArgumentObj>, arguments);

    // The name of the rest argument (as in `$args...`),
    // or `null` if none was declared.
    ADD_CONSTREF(EnvKey, restArg);

    // This is only used for debugging
    ADD_CONSTREF(size_t, maxArgs);

  public:

    // Value constructor
    ArgumentDeclaration(SourceSpan&& pstate,
      sass::vector<ArgumentObj>&& arguments = {},
      EnvKey&& restArg = {});

    // Check if signature is void
    bool isEmpty() const {
      return arguments_.empty()
        && restArg_.empty();
    }

    // Parse source into arguments
    static ArgumentDeclaration* parse(
      Compiler& context, SourceData* source);

    // Throws a [SassScriptException] if [positional] and
    // [names] aren't valid for this argument declaration.
    void verify(
      size_t positional,
      const ValueFlatMap& names,
      const SourceSpan& pstate,
      const BackTraces& traces) const;

    // Returns whether [positional] and [names]
    // are valid for this argument declaration.
    bool matches(const ArgumentResults& evaluated) const;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class ArgumentInvocation final : public AstNode
  {
  private:

    // The arguments passed by position.
    ADD_REF(ExpressionVector, positional);

    // The argument expressions passed by name.
    ADD_REF(ExpressionFlatMap, named);

    // The first rest argument (as in `$args...`).
    ADD_CONSTREF(ExpressionObj, restArg);

    // The second rest argument, which is expected to only contain a keyword map.
    // This can be an already evaluated Map (via call) or a MapExpression.
    // So we must guarantee that this evaluates to a real Map value.
    ADD_CONSTREF(ExpressionObj, kwdRest);

  public:

    // Value constructor
    ArgumentInvocation(const SourceSpan& pstate,
      ExpressionVector&& positional,
      ExpressionFlatMap&& named,
      Expression* restArgs = nullptr,
      Expression* kwdRest = nullptr);

    // Value constructor
    ArgumentInvocation(SourceSpan&& pstate,
      ExpressionVector&& positional,
      ExpressionFlatMap&& named,
      Expression* restArgs = nullptr,
      Expression* kwdRest = nullptr);

    // Returns whether this invocation passes no arguments.
    bool isEmpty() const;

  };

  /////////////////////////////////////////////////////////////////////////
  // The result of evaluating arguments to a function or mixin.
  /////////////////////////////////////////////////////////////////////////

  class ArgumentResults final {

    // Arguments passed by position.
    ADD_REF(ValueVector, positional);

    // Arguments passed by name.
    // A list implementation is often more efficient
    // I don't expect any function to have many arguments
    // Normally the trade-off is around 8 items in the list
    ADD_REF(ValueFlatMap, named);

    // The separator used for the rest argument list, if any.
    ADD_REF(SassSeparator, separator);

  public:

    // Value constructor
    ArgumentResults() :
      separator_(SASS_UNDEF)
    {};

    // Value move constructor
    ArgumentResults(
      ValueVector&& positional,
      ValueFlatMap&& named,
      SassSeparator separator);

    // Move constructor
    ArgumentResults(
      ArgumentResults&& other) noexcept;

    // Move assignment operator
    ArgumentResults& operator=(
      ArgumentResults&& other) noexcept;

    // Clear results
    void clear() {
      named_.clear();
      positional_.clear();
    }

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CallableInvocation
  {

  private:

    // The arguments passed to the callable.
    ADD_CONSTREF(ArgumentInvocationObj, arguments);

  public:

    // Value constructor
    CallableInvocation(
      ArgumentInvocation* arguments) :
      arguments_(arguments)
    {}

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
