/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_callables.hpp"

#include "exceptions.hpp"
#include "parser_scss.hpp"
#include "compiler.hpp"
#include "eval.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Callable::Callable(
    const SourceSpan& pstate) :
    AstNode(pstate),
    hash_(0)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // This should be thread-safe
  static std::hash<void*> ptrHasher;
  static std::hash<bool> boolHasher;
  //static std::hash<double> doubleHasher;
  static std::hash<SassFnSig> fnHasher;
  //static std::hash<std::size_t> sizetHasher;
  static std::hash<sass::string> stringHasher;
  static std::hash<SassFunctionLambda> lambdaHasher;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BuiltInCallable::BuiltInCallable(
    const EnvKey& envkey,
    CallableSignature* signature,
    const SassFnSig& callback,
    bool isInternal) :
    Callable(SourceSpan::internal("[BUILTIN]")),
    envkey_(envkey),
    // Create a single entry in overloaded function
    function_(SassFnPair{ signature, callback }),
    isInternalFn_(isInternal),
    acceptsContent_(false)
  {}

  // Return callback with matching signature
  const SassFnPair& BuiltInCallable::callbackFor(
    const ArgumentResults& evaluated)
  {
    return function_;
  }

  // Equality comparator (needed for `get-function` value)
  // ToDo: doesn't seem to be one hundert percent correct?
  bool BuiltInCallable::operator==(const Callable& rhs) const
  {
    if (const BuiltInCallable* builtin = rhs.isaBuiltInCallable()) {
      return envkey_ == builtin->envkey_ &&
        function_.first == builtin->function_.first &&
        function_.second == builtin->function_.second;
    }
    return false;
  }

  size_t BuiltInCallable::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(BuiltInCallable).hash_code());
      hash_combine(hash_, stringHasher(envkey_.norm()));
      hash_combine(hash_, fnHasher(function_.second));
      hash_combine(hash_, function_.first->hash());
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  BuiltInCallables::BuiltInCallables(
    const EnvKey& envkey,
    const SassFnPairs& overloads) :
    Callable(SourceSpan::internal("[BUILTINS]")),
    envkey_(envkey),
    overloads_(overloads)
  {
    size_t size = 0;
    for (auto fn : overloads) {
      size = std::max(size,
        fn.first->maxArgs());
    }
    for (auto fn : overloads) {
      fn.first->maxArgs(size);
    }
  }

  // Return callback with matching signature
  const SassFnPair& BuiltInCallables::callbackFor(
    const ArgumentResults& evaluated)
  {
    for (SassFnPair& pair : overloads_) {
      if (pair.first->matches(evaluated)) {
        return pair;
      }
    }
    return overloads_.back();
  }

  // Equality comparator (needed for `get-function` value)
  bool BuiltInCallables::operator==(const Callable& rhs) const
  {
    if (const BuiltInCallables* builtin = rhs.isaBuiltInCallables()) {
      if (!(envkey_ == builtin->envkey_)) return false;
      return overloads_ == builtin->overloads_;
    }
    return false;
  }

  size_t BuiltInCallables::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(BuiltInCallables).hash_code());
      hash_combine(hash_, stringHasher(envkey_.norm()));
      for (const auto& pair : overloads_) {
        hash_combine(hash_, fnHasher(pair.second));
        hash_combine(hash_, pair.first->hash());
      }
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  UserDefinedCallable::UserDefinedCallable(
    const SourceSpan& pstate,
    const EnvKey& envkey,
    CallableDeclarationObj declaration,
    UserDefinedCallable* content) :
    Callable(pstate),
    envkey_(envkey),
    declaration_(declaration),
    content_(content)
  {}

  // Equality comparator (needed for `get-function` value)
  bool UserDefinedCallable::operator==(const Callable& rhs) const
  {
    if (const UserDefinedCallable* builtin = rhs.isaUserDefinedCallable()) {
      return envkey_ == builtin->envkey_ &&
        // Must use pointer equality here
        declaration_ == builtin->declaration_;
    }
    return false;
  }

  size_t UserDefinedCallable::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(UserDefinedCallable).hash_code());
      hash_combine(hash_, stringHasher(envkey_.norm()));
      hash_combine(hash_, ptrHasher(declaration_.ptr()));
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ExternalCallable::ExternalCallable(
    const EnvKey& fname,
    CallableSignature* parameters,
    SassFunctionLambda lambda) :
    Callable(SourceSpan::internal("[EXTERNAL]")),
    envkey_(fname),
    declaration_(parameters),
    lambda_(lambda),
    cookie_(nullptr)
  {}

  // Equality comparator (needed for `get-function` value)
  bool ExternalCallable::operator==(const Callable& rhs) const
  {
    if (const ExternalCallable* builtin = rhs.isaExternalCallable()) {
      return envkey_ == builtin->envkey_ &&
        lambda_ == builtin->lambda_;
    }
    return false;
  }

  size_t ExternalCallable::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(ExternalCallable).hash_code());
      hash_combine(hash_, stringHasher(envkey_.norm()));
      hash_combine(hash_, ptrHasher(declaration_));
      hash_combine(hash_, lambdaHasher(lambda_));
      hash_combine(hash_, ptrHasher(cookie_));
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Argument::Argument(const SourceSpan& pstate,
    const EnvKey& name,
    ExpressionObj defval,
    bool is_rest_argument,
    bool is_keyword_argument) :
    AstNode(pstate),
    name_(name),
    defval_(defval),
    is_rest_argument_(is_rest_argument),
    is_keyword_argument_(is_keyword_argument)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CallableSignature::CallableSignature(
    SourceSpan&& pstate,
    sass::vector<ArgumentObj>&& arguments,
    EnvKey&& restArg) :
    hash_(0), // calc on demand
    AstNode(std::move(pstate)),
    arguments_(std::move(arguments)),
    restArg_(std::move(restArg)),
    maxArgs_(arguments_.size())
  {
    if (!restArg_.empty()) {
      maxArgs_ += 1;
    }
  }

  CallableSignature::CallableSignature(
    const SourceSpan& pstate,
    sass::vector<ArgumentObj>&& arguments,
    EnvKey&& restArg) :
    AstNode(pstate),
    hash_(0), // calc on demand
    arguments_(std::move(arguments)),
    restArg_(std::move(restArg)),
    maxArgs_(arguments_.size())
  {
    if (!restArg_.empty()) {
      maxArgs_ += 1;
    }
  }

  // Parse `source` into signature
  CallableSignature* CallableSignature::parse(
    Compiler& context, SourceData* source)
  {
    ScssParser parser(context, source);
    return parser.parseArgumentDeclaration();
  }

  // Throws a [SassScriptException] if [positional] and
  // [names] aren't valid for this argument declaration.
  void CallableSignature::verify(
    size_t positional,
    const ValueFlatMap& names,
    const SourceSpan& pstate,
    const BackTraces& traces) const
  {

    size_t i = 0;
    size_t namedUsed = 0;
    size_t iL = arguments_.size();
    while (i < std::min(positional, iL)) {
      if (names.count(arguments_[i]->name()) == 1) {
        throw Exception::RuntimeException(traces,
          "Argument $" + arguments_[i]->name().orig() +
          " name was passed both by position and by name.");
      }
      i++;
    }
    while (i < iL) {
      if (names.count(arguments_[i]->name()) == 1) {
        namedUsed++;
      }
      else if (arguments_[i]->defval() == nullptr) {
        throw Exception::RuntimeException(traces,
          "Missing element $" + arguments_[i]->name().orig() + ".");
      }
      i++;
    }

    if (!restArg_.empty()) return;

    if (positional > arguments_.size()) {
      sass::sstream strm;
      strm << "Only " << arguments_.size() << " "; //  " positional ";
      strm << pluralize("argument", arguments_.size());
      strm << " allowed, but " << positional << " ";
      strm << pluralize("was", positional, "were");
      strm << " passed.";
      throw Exception::RuntimeException(
        traces, strm.str());
    }

    if (namedUsed < names.size()) {
      ValueFlatMap unknownNames(names);
      for (Argument* arg : arguments_) {
        unknownNames.erase(arg->name());
      }
      throw Exception::RuntimeException(
        traces, "No argument named $" +
        toSentence(getKeyVector(unknownNames), "or") + ".");
    }

  }
  // EO verify

  // Returns whether [positional] and [names]
  // are valid for this argument declaration.
  bool CallableSignature::matches(
    const ArgumentResults& evaluated) const
  {
    size_t namedUsed = 0; Argument* argument;
    for (size_t i = 0, iL = arguments_.size(); i < iL; i++) {
      argument = arguments_[i];
      if (i < evaluated.positional().size()) {
        if (evaluated.named().count(argument->name()) == 1) {
          return false;
        }
      }
      else if (evaluated.named().count(argument->name()) == 1) {
        namedUsed++;
      }
      else if (argument->defval().isNull()) {
        return false;
      }
    }
    if (!restArg_.empty()) return true;
    if (evaluated.positional().size() > arguments_.size()) return false;
    if (namedUsed < evaluated.named().size()) return false;
    return true;
  }

  size_t CallableSignature::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(CallableSignature).hash_code());
      hash_combine(hash_, stringHasher(restArg_.norm()));
      for (const Argument* param : arguments_) {
        if (param == nullptr) continue;
        hash_combine(hash_, stringHasher(param->name().norm()));
        hash_combine(hash_, boolHasher(param->is_rest_argument()));
        hash_combine(hash_, boolHasher(param->is_keyword_argument()));
      }
    }
    return hash_;
  }
  // EO matches

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CallableArguments::CallableArguments(
    const SourceSpan& pstate,
    ExpressionVector&& positional,
    ExpressionFlatMap&& named,
    Expression* restArg,
    Expression* kwdRest) :
    AstNode(pstate),
    positional_(std::move(positional)),
    named_(std::move(named)),
    restArg_(restArg),
    kwdRest_(kwdRest)
  {}

  CallableArguments::CallableArguments(
    SourceSpan&& pstate,
    ExpressionVector&& positional,
    ExpressionFlatMap&& named,
    Expression* restArg,
    Expression* kwdRest) :
    AstNode(std::move(pstate)),
    positional_(std::move(positional)),
    named_(std::move(named)),
    restArg_(restArg),
    kwdRest_(kwdRest)
  {}

  // Returns whether this invocation passes no arguments.
  bool CallableArguments::isEmpty() const
  {
    return positional_.empty()
      && named_.empty()
      && restArg_.isNull();
  }
  // EO isEmpty

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ArgumentResults::ArgumentResults(
    ValueVector&& positional,
    ValueFlatMap&& named,
    SassSeparator separator) :
    positional_(std::move(positional)),
    named_(std::move(named)),
    separator_(separator)
  {}

  ArgumentResults::ArgumentResults(
    ArgumentResults&& other) noexcept :
    positional_(std::move(other.positional_)),
    named_(std::move(other.named_)),
    separator_(other.separator_)
  {}

  ArgumentResults& ArgumentResults::operator=(
    ArgumentResults&& other) noexcept
  {
    positional_ = std::move(other.positional_);
    named_ = std::move(other.named_);
    separator_ = other.separator_;
    return *this;
  }

  /////////////////////////////////////////////////////////////////////////
  // Implement the execute dispatch to evaluator
  /////////////////////////////////////////////////////////////////////////

  Value* BuiltInCallable::execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate)
  {
    return eval.execute(this, arguments, pstate);
  }

  Value* BuiltInCallables::execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate)
  {
    return eval.execute(this, arguments, pstate);
  }

  Value* UserDefinedCallable::execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate)
  {
    return eval.execute(this, arguments, pstate);
  }

  Value* ExternalCallable::execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate)
  {
    return eval.execute(this, arguments, pstate);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  PlainCssCallable::PlainCssCallable(const SourceSpan& pstate, const EnvKey& fname) :
    Callable(pstate),
    envkey_(fname)
  {
  }

  Value* PlainCssCallable::execute(Eval& eval, CallableArguments* arguments, const SourceSpan& pstate)
  {

    if (!arguments->named().empty() || arguments->kwdRest() != nullptr) {
      throw Exception::RuntimeException(eval.compiler,
        "Plain CSS functions don't support keyword arguments.");
    }

    sass::string text(envkey_.orig());
    text += "(";
    bool joiner = false;
    for (auto arg : arguments->positional()) {
      if (joiner) text += ", ";
      text += arg->toString();
      joiner = true;
    }
    text += ")";
    return SASS_MEMORY_NEW(String, pstate, std::move(text));
  }

  bool PlainCssCallable::operator==(const Callable& rhs) const
  {
    if (const PlainCssCallable* builtin = rhs.isaPlainCssCallable()) {
      return envkey_ == builtin->envkey_;
    }
    return false;
  }

  size_t PlainCssCallable::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(PlainCssCallable).hash_code());
      hash_combine(hash_, stringHasher(envkey_.norm()));
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
