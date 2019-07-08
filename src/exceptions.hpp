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
#include "backtrace.hpp"
#include "ast_fwd_decl.hpp"
#include "environment_cnt.hpp"
#include "sass/functions.h"

namespace Sass {

  class BackTrace;

  StringVector getKeyVector(const ValueFlatMap& names);

  sass::string pluralize(const sass::string& singular, size_t size, const sass::string& plural = "");
  sass::string toSentence(const StringVector& names, const sass::string& conjunction = "and", const uint8_t prefix = 0);

  namespace Exception {

    const sass::string def_msg("Invalid sass detected");
    const sass::string def_op_null_msg("Invalid null operation");
    const sass::string def_nesting_limit("Code too deeply nested");

    const sass::string msg_recursion_limit =
      "Too deep recursion detected. This can be caused by too deep level nesting.\n"
      "LibSass will abort here in order to avoid a possible stack overflow.\n";

    class Base : public std::runtime_error {
      protected:
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

    class DuplicateKeyError : public Base {
      protected:
        const Map& dup;
        const Value& org;
      public:
        DuplicateKeyError(BackTraces traces,
          const Map& dup, const Value& org);
    };

    

      class TooManyArguments : public RuntimeException {
      public:
        TooManyArguments(BackTraces traces, size_t given, size_t expected);
        TooManyArguments(BackTraces traces, const ExpressionFlatMap& given, const Sass::EnvKeySet& expected);
        TooManyArguments(BackTraces traces, const ValueFlatMap& superfluous);
    };

    class MissingArgument : public RuntimeException {
    public:
      MissingArgument(BackTraces traces, const EnvKey& name);
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

    class ZeroDivisionError : public OperationError {
      public:
        ZeroDivisionError(const Value& lhs, const Value& rhs);
    };

    class IncompatibleUnits : public OperationError {
      protected:
        // const Sass::UnitType lhs;
        // const Sass::UnitType rhs;
      public:
        IncompatibleUnits(const Units& lhs, const Units& rhs);
        // virtual ~IncompatibleUnits() noexcept {};
    };



    class TopLevelParent : public Base {
    public:
      TopLevelParent(BackTraces traces, SourceSpan pstate);
      // virtual ~TopLevelParent() noexcept {};
    };

    class UnsatisfiedExtend : public Base {
    public:
      UnsatisfiedExtend(BackTraces traces, Extension extension);
      // virtual ~UnsatisfiedExtend() noexcept {};
    };

    class ExtendAcrossMedia : public Base {
    public:
      ExtendAcrossMedia(BackTraces traces, Extension extension);
      // virtual ~ExtendAcrossMedia() noexcept {};
    };

  }

}

#endif
