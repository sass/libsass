/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ERROR_EXCEPTIONS_HPP
#define SASS_ERROR_EXCEPTIONS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <string>
#include <sstream>
#include <stdexcept>
#include "logger.hpp"
#include "units.hpp"
#include "file.hpp"
#include "backtrace.hpp"
#include "ast_fwd_decl.hpp"
#include "environment_cnt.hpp"

namespace Sass {

  class Extender;
  class BackTrace;

  sass::string toSentence(
    const StringVector& names,
    const sass::string& conjunction,
    const sass::string& prefix = {},
    const sass::string& postfix = {},
    const uint8_t quote = 0);

  StringVector getKeyVector(const ValueFlatMap& names);

  sass::string pluralize(const sass::string& singular, size_t size, const sass::string& plural = "");

  namespace Exception {

    const sass::string def_msg("Invalid sass detected");
    const sass::string def_op_null_msg("Invalid null operation");
    const sass::string def_nesting_limit("Code too deeply nested");

    const sass::string msg_recursion_limit =
      "Too deep recursion detected. This can be caused by too deep level nesting.\n"
      "LibSass will abort here in order to avoid a possible stack overflow.\n";

    class Base : public std::runtime_error {
      public:
        sass::string msg;
      public:
        StackTraces traces;
      public:
        Base(sass::string msg, BackTraces traces);
        Base(sass::string msg, BackTraces traces, SourceSpan pstate);
        virtual const char* what() const throw() { return msg.c_str(); }
        // virtual ~Base() noexcept {};
    };

    class ParserException : public Base {
    public:
      ParserException(BackTraces traces, sass::string msg);
    };

    class RuntimeException : public Base {
    public:
      RuntimeException(BackTraces traces, sass::string msg);
      // RuntimeException(sass::string msg, BackTraces traces);
      RuntimeException(sass::string msg, BackTraces traces, SourceSpan pstate);
      virtual const char* what() const throw() { return msg.c_str(); }
    };

    class ModuleUnknown : public RuntimeException
    {
    public:
      ModuleUnknown(
        BackTraces traces,
        sass::string name);
    };

    class VariableUnknown : public RuntimeException
    {
    public:
      VariableUnknown(
        BackTraces traces,
        const EnvKey& name);
    };

    class ModuleAlreadyKnown : public RuntimeException
    {
    public:
      ModuleAlreadyKnown(
        BackTraces traces,
        sass::string name);
    };

    class TardyAtRule : public RuntimeException
    {
    public:
      TardyAtRule(
        BackTraces traces,
        sass::string name);
    };

    class InvalidSassIdentifier : public RuntimeException
    {
    public:
      InvalidSassIdentifier(
        BackTraces traces,
        sass::string name);
    };

    class InvalidDefaultNamespace : public RuntimeException
    {
    public:
      InvalidDefaultNamespace(
        BackTraces traces,
        sass::string name);
    };

    class UnknownImport : public RuntimeException
    {
    public:
      UnknownImport(
        BackTraces traces);
    };

    class AmbiguousImports : public RuntimeException
    {
    public:
      AmbiguousImports(BackTraces traces,
        sass::vector<ResolvedImport> imports);
    };


    class UnitMismatch : public RuntimeException
    {
    public:
      UnitMismatch(
        BackTraces traces,
        const Number& lhs,
        const Number& rhs);
    };

    class IncompatibleCalcValue : public RuntimeException
    {
    public:
      IncompatibleCalcValue(
        BackTraces traces,
        const AstNode& lhs,
        SourceSpan pstate);
    };


    class DeprecatedColorAdjustFn : public RuntimeException
    {
    public:
      DeprecatedColorAdjustFn(
        Logger& logger,
        const ValueVector& arguments,
        sass::string name,
        sass::string prefix,
        sass::string secondarg = Strings::amount);
    };

    class InvalidParent : public Base {
      protected:
        Selector* parent;
        Selector* selector;
      public:
        InvalidParent(Selector* parent, BackTraces traces, Selector* selector);
    };

    class InvalidUnicode : public Base {
    public:
      InvalidUnicode(SourceSpan pstate, BackTraces traces);
    };

    class CustomImportError : public Base {
    public:
      CustomImportError(BackTraces traces, sass::string msg);
    };

    class SassScriptException : public Base {
    public:
      SassScriptException(
        BackTraces traces,
        SourceSpan pstate,
        sass::string msg,
        sass::string name = "");

      SassScriptException(sass::string msg,
        BackTraces traces, SourceSpan pstate,
        sass::string name = "");
    };

    class CustomImportNotFound : public RuntimeException {
    public:
      CustomImportNotFound(BackTraces traces, sass::string file);
    };

    class CustomImportAmbigous : public RuntimeException {
    public:
      CustomImportAmbigous(BackTraces traces, sass::string file);
    };

    class CustomImportLoadError : public RuntimeException {
    public:
      CustomImportLoadError(BackTraces traces, sass::string file);
    };

    class RecursionLimitError : public Base {
      public:
        RecursionLimitError();
    };

    class EndlessExtendError : public Base {
    public:
      EndlessExtendError(BackTraces traces);
    };

    class OpNotCalcSafe : public RuntimeException {
    public:
      OpNotCalcSafe(BackTraces traces, const BinaryOpExpression* op);
      OpNotCalcSafe(BackTraces traces, const Expression* op);
    };

    class MissingMathOp : public RuntimeException {
    public:
      MissingMathOp(BackTraces traces, const Expression* op);
      MissingMathOp(BackTraces traces, const Expression* lhs, const Expression* rhs);
    };

    class DuplicateKeyError : public Base {
      protected:
        const Map& dup;
        const Value& org;
      public:
        DuplicateKeyError(BackTraces traces,
          const Map& dup, const Value& org);
    };

    class TooFewArguments : public RuntimeException {
    public:
      TooFewArguments(BackTraces traces, size_t given, size_t expected);
      TooFewArguments(BackTraces traces, const ExpressionFlatMap& given, const Sass::EnvKeySet& expected);
      TooFewArguments(BackTraces traces, const ValueFlatMap& superfluous);
    };

    class DuplicateKeyArgument : public RuntimeException {
    public:
      DuplicateKeyArgument(BackTraces traces, const ValueFlatMap& superfluous);
    };

    class TooManyArguments : public RuntimeException {
      public:
        TooManyArguments(BackTraces traces, size_t given, size_t expected);
        TooManyArguments(BackTraces traces, const ExpressionFlatMap& given, const Sass::EnvKeySet& expected);
        TooManyArguments(BackTraces traces, const ValueFlatMap& superfluous);
    };

    class NoAngleArgument : public RuntimeException {
    public:
      NoAngleArgument(BackTraces traces, const Value* value, const sass::string& name);
    };

    class MissingArgument : public RuntimeException {
    public:
      MissingArgument(BackTraces traces, const EnvKey& name);
      MissingArgument(BackTraces traces, const sass::string& name);
    };

    class MustHaveArguments : public RuntimeException {
    public:
      MustHaveArguments(BackTraces traces, const sass::string& name);
    };

    class ArgumentGivenTwice : public RuntimeException {
    public:
      ArgumentGivenTwice(BackTraces traces, const EnvKey& name);
    };

    class UnknownNamedArgument : public RuntimeException {
    public:
      UnknownNamedArgument(BackTraces traces, ValueFlatMap names);
    };

    class MixedParamGroups : public RuntimeException {
    public:
      MixedParamGroups(BackTraces traces, const sass::string& first, const StringVector seconds);
    };
    

    class InvalidCssValue : public Base {
      public:
        InvalidCssValue(BackTraces traces, const Value& val);
    };


    /* common virtual base class (has no pstate or trace) */
    class OperationError : public std::runtime_error {
      protected:
        sass::string msg;
      public:
        OperationError(sass::string msg = sass::string("Undefined operation"))
        : std::runtime_error(msg.c_str()), msg(msg)
        {};
      public:
        virtual const char* what() const throw() { return msg.c_str(); }
    };

    class TopLevelParent : public Base {
    public:
      TopLevelParent(BackTraces traces, SourceSpan pstate);
    };

    class UnsatisfiedExtend : public Base {
    public:
      UnsatisfiedExtend(BackTraces traces, Extension* extension);
    };

    class ExtendAcrossMedia : public Base {
    public:
      ExtendAcrossMedia(BackTraces traces, const Extension* extension);
      ExtendAcrossMedia(BackTraces traces, const Extender* extender);
    };

    class IoError : public Base {
    public:
      IoError(BackTraces traces, const sass::string& msg, const sass::string& path);
    };

  }

}

#endif
