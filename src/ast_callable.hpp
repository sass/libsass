/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_CALLABLE_HPP
#define SASS_AST_CALLABLE_HPP

// sass.hpp must go before all system headers
// to get the  __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "fn_utils.hpp"
#include "ast_nodes.hpp"
#include "environment_key.hpp"
#include "environment_stack.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  typedef Value* (*SassFnSig)(FN_PROTOTYPE);
  typedef std::pair<CallableSignatureObj, SassFnSig> SassFnPair;
  typedef sass::vector<SassFnPair> SassFnPairs;

  /////////////////////////////////////////////////////////////////////////
  // Base class for everything that can be called on demand.
  /////////////////////////////////////////////////////////////////////////

  class Callable : public AstNode
  {

  protected:

    // Hash is only calculated once and afterwards the value
    // must not be mutated, which is the case with how sass
    // works, although we must be a bit careful not to alter
    // any value that has already been added to a set or map.
    // Must create a copy if you need to alter such an object.
    mutable size_t hash_;

  public:

    // Value constructor
    Callable(const SourceSpan& pstate);

    // The main entry point to execute the function
    // Must be implemented in each specialization
    virtual Value* execute(Eval& eval,
      CallableArguments* arguments,
      const SourceSpan& pstate) = 0;

    // Return name of this callable/function
    virtual const sass::string& name() const = 0;

    // Equality comparator (needed for `get-function` value)
    virtual bool operator==(const Callable& rhs) const = 0;

    // Check if call is considered internal
    // True only for certain built-ins
    virtual bool isInternal() const {
      return false;
    }

    // Implement interface for base Value class
    virtual size_t hash() const = 0;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(BuiltInCallable);
    DECLARE_ISA_CASTER(BuiltInCallables);
    DECLARE_ISA_CASTER(UserDefinedCallable);
    DECLARE_ISA_CASTER(ExternalCallable);
    DECLARE_ISA_CASTER(PlainCssCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  // Individual argument object for function signatures.
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
  // Object for the function signature holding which parameters a
  // callable can have or expects, with optional rest arguments.
  //////////////////////////////////////////////////////////////////////

  class CallableSignature final : public AstNode
  {

  protected:

    // Hash is only calculated once and afterwards the value
    // must not be mutated, which is the case with how sass
    // works, although we must be a bit careful not to alter
    // any value that has already been added to a set or map.
    // Must create a copy if you need to alter such an object.
    mutable size_t hash_;

  private:

    // The arguments that are taken.
    ADD_CONSTREF(sass::vector<ArgumentObj>, arguments);

    // The name of the rest argument (as in `$args...`),
    ADD_CONSTREF(EnvKey, restArg);

    // This is only used for debugging
    ADD_CONSTREF(size_t, maxArgs);

  public:

    // Value constructor
    CallableSignature(SourceSpan&& pstate,
      sass::vector<ArgumentObj>&& arguments = {},
      EnvKey&& restArg = {});

    // Value constructor
    CallableSignature(const SourceSpan& pstate,
      sass::vector<ArgumentObj>&& arguments = {},
      EnvKey&& restArg = {});

    // Checks if signature is void
    bool isEmpty() const {
      return arguments_.empty()
        && restArg_.empty();
    }

    // Parse `source` into signature
    static CallableSignature* parse(
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

    size_t hash() const;

  };

  /////////////////////////////////////////////////////////////////////////
  // Object for the actual function arguments to pass to the function
  // invocation. It must be valid in regard to the callable signature
  // of the invoked function (will throw an error otherwise).
  /////////////////////////////////////////////////////////////////////////

  class CallableArguments final : public AstNode
  {
  private:

    // The arguments passed by position.
    ADD_CONSTREF(ExpressionVector, positional);

    // The arguments passed by name.
    ADD_CONSTREF(ExpressionFlatMap, named);
    
    // Optional rest argument (as in `$args...`).
    // Supports only one rest arg and it must be last.
    // ToDo: explain difference between restArg and kwdRest.
    ADD_CONSTREF(ExpressionObj, restArg);

    // The second rest argument, which is expected to only contain a keyword map.
    // This can be an already evaluated Map (via call) or a MapExpression.
    // So we must guarantee that this evaluates to a real Map value.
    ADD_CONSTREF(ExpressionObj, kwdRest);

  public:

    // Value move constructor
    CallableArguments(SourceSpan&& pstate,
      ExpressionVector&& positional,
      ExpressionFlatMap&& named,
      Expression* restArgs = nullptr,
      Expression* kwdRest = nullptr);

    // Partial value move constructor
    CallableArguments(const SourceSpan& pstate,
      ExpressionVector&& positional,
      ExpressionFlatMap&& named,
      Expression* restArgs = nullptr,
      Expression* kwdRest = nullptr);

    // Returns whether this invocation passes no arguments.
    bool isEmpty() const;

  };

  /////////////////////////////////////////////////////////////////////////
  // The result of evaluating arguments to a function or mixin.
  // It's basically the same as `CallableArguments` but with all
  // based values already evaluated in order to check compliance
  // with the expected callable signature.
  /////////////////////////////////////////////////////////////////////////

  class ArgumentResults final {

    // Arguments passed by position.
    ADD_REF(ValueVector, positional);

    // Arguments passed by name.
    // A list implementation is often more efficient
    // We don't expect any function to have many arguments
    // Normally trade-off starts around 8 items in the list
    ADD_REF(ValueFlatMap, named);

    // Separator used for rest argument list, if any.
    ADD_CONSTREF(SassSeparator, separator);

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

}

#endif
