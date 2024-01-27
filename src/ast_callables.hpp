/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
// This file was deliberately split from `ast_callable.hpp`. It contains
// all the specialized implementations for callables. These often need
// access to higher level classes, which poses include dependency issues
// if everything would be defined in a single header file.
/*****************************************************************************/
#ifndef SASS_AST_CALLABLES_HPP
#define SASS_AST_CALLABLES_HPP

// sass.hpp must go before all system headers
// to get the  __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_nodes.hpp"
#include "ast_callable.hpp"
#include "ast_statements.hpp"
#include "capi_function.hpp"
#include "environment_key.hpp"
#include "environment_stack.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Internal callables provided by LibSass itself.
  /////////////////////////////////////////////////////////////////////////

  class BuiltInCallable final : public Callable
  {
  private:

    // Name of this callable/function
    ADD_CONSTREF(EnvKey, envkey);

    // Pair of signature and callback
    ADD_CONSTREF(SassFnPair, function);

    // Some functions are internal only
    ADD_CONSTREF(bool, isInternalFn);

    // Some mixins accept content blocks
    ADD_CONSTREF(bool, acceptsContent);

  public:

    // Creates a callable with a single [arguments] declaration
    // and a single [callback]. The argument declaration is parsed
    // from [arguments], which should not include parentheses.
    // Throws a [SassFormatException] if parsing fails.
    BuiltInCallable(
      const EnvKey& fname,
      CallableSignature* signature,
      const SassFnSig& callback,
      bool isInternal = false);

    // Return callback with matching signature
    const SassFnPair& callbackFor(
      const ArgumentResults& evaluated);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    size_t hash() const override final;

    // Check if call is considered internal
    bool isInternal() const override final {
      return isInternalFn_;
    }

    // Define isaBuiltInCallable up-cast function
    IMPLEMENT_ISA_CASTER(BuiltInCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  // Internal callable with multiple signatures to choose from.
  /////////////////////////////////////////////////////////////////////////

  class BuiltInCallables final : public Callable
  {
  private:

    // Name of this callable/function
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
    Value* execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    size_t hash() const override final;

    // Define isaBuiltInCallables up-cast function
    IMPLEMENT_ISA_CASTER(BuiltInCallables);
  };

  /////////////////////////////////////////////////////////////////////////
  // User defined callable from sass code (functions and mixins)
  /////////////////////////////////////////////////////////////////////////
  class UserDefinedCallable final : public Callable
  {
  private:

    // Name of this callable (used for reporting)
    ADD_CONSTREF(EnvKey, envkey);

    // The declaration (parameters this callable takes).
    ADD_CONSTREF(CallableDeclarationObj, declaration);

    // Content blocks passed to includes need to preserve
    // the previous content block. Could have been implemented
    // with a stack vector, but we remember it here instead.
    ADD_PROPERTY(UserDefinedCallable*, content);

  public:

    // Value constructor
    UserDefinedCallable(
      const SourceSpan& pstate,
      const EnvKey& fname,
      CallableDeclarationObj declaration,
      UserDefinedCallable* content = nullptr);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    size_t hash() const override final;

    // Define isaUserDefinedCallable up-cast function
    IMPLEMENT_ISA_CASTER(UserDefinedCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  // External callable defined on the C-API side.
  /////////////////////////////////////////////////////////////////////////

  class ExternalCallable final : public Callable
  {
  private:

    // Name of this callable (used for reporting)
    ADD_CONSTREF(EnvKey, envkey);

    // The declaration (parameters this function takes).
    ADD_CONSTREF(CallableSignatureObj, declaration);

    // The attached external callback reference
    ADD_CONSTREF(SassFunctionLambda, lambda);

    // The attached external data cookie
    ADD_PROPERTY(void*, cookie);

  public:

    // Value constructor
    ExternalCallable(
      const EnvKey& fname,
      CallableSignature* parameters,
      SassFunctionLambda function);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    size_t hash() const override final;

    // Define isaExternalCallable up-cast function
    IMPLEMENT_ISA_CASTER(ExternalCallable);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class PlainCssCallable final : public Callable
  {

    // Name of this callable (used for reporting)
    ADD_CONSTREF(EnvKey, envkey);

  public:

    // Value constructor
    PlainCssCallable(
      const SourceSpan& pstate,
      const EnvKey& fname);

    // The main entry point to execute the function (implemented in each specialization)
    Value* execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate) override final;

    // Return the function name
    const sass::string& name() const override final { return envkey_.norm(); }

    // Equality comparator (needed for `get-function` value)
    bool operator==(const Callable& rhs) const override final;

    size_t hash() const override final;

    // Define isaExternalCallable up-cast function
    IMPLEMENT_ISA_CASTER(PlainCssCallable);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
