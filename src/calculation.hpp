/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CALCULATION_HPP
#define SASS_CALCULATION_HPP

#include "ast.hpp"
#include "strings.hpp"
#include "exceptions.hpp"

#include "debugger.hpp"

#include <cmath>

namespace Sass {

  class Calculation32 {

    sass::string name;

    sass::vector<AstNodeObj> arguments;

    bool isSpecialNumber = true;

  public:

    static Calculation32 unsimplified(const sass::string& name, sass::vector<AstNodeObj> arguments) {
      return Calculation32(name, arguments);
    }

    static Number* roundWithStep(const sass::string&, Number* number, Number* step);

    static Value* singleArgument(Logger& logger, const FunctionExpression* pstate, const sass::string& name, AstNode* argument, Number*(*mathFunc)(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number*), bool forbitUnits = false)
    {
      AstNode* simplified = argument->simplify(logger);
      auto* number = dynamic_cast<Number*>(simplified);
      if (number == nullptr) return SASS_MEMORY_NEW(
        Calculation, argument->pstate(), name, { simplified });
      // if (forbitUnits) number->assertNoUnits();
      return mathFunc(logger, pstate, argument, number);
    }

    static Value* singleArgument2(Logger& logger, const SourceSpan& pstate, const sass::string& name, const ValueVector& args, Number* (*mathFunc)(Logger& logger, const SourceSpan& pstate, AstNode* argument, Number*), bool forbitUnits = false)
    {
      AstNode* simplified = args[0]->simplify(logger);
      auto* number = dynamic_cast<Number*>(simplified);
      if (number == nullptr) return SASS_MEMORY_NEW(
        Calculation, pstate, name, {simplified});
      // if (forbitUnits) number->assertNoUnits();
      return mathFunc(logger, pstate, args[0], number);
    }

    // No support for numbers with units
    static Number* fnSqrt(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg) {
      double rv = std::sqrt(arg->value());
      return SASS_MEMORY_NEW(Number, arg->pstate(), rv);
    }

    static Number* fnSin(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg) {
      double factor = arg->factorToUnits(unit_rad);
      if (factor == 0.0) {
        throw Exception::SassScriptException(
          "$number: Expected " + argument->toString() + " to have an angle unit (deg, grad, rad, turn).",
          logger, pstate->pstate());
      }
      return SASS_MEMORY_NEW(Number, arg->pstate(),
        std::sin(arg->value() * factor));
    }

    static Number* fnCos(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg) {
      double factor = arg->factorToUnits(unit_rad);
      if (factor == 0.0) {
        throw Exception::SassScriptException(
          "$number: Expected " + argument->toString() + " to have an angle unit (deg, grad, rad, turn).",
          logger, pstate->pstate());
      }
      return SASS_MEMORY_NEW(Number, arg->pstate(),
        std::cos(arg->value() * factor));
    }
    
    static Number* fnTan(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg) {
      double factor = arg->factorToUnits(unit_rad);
      if (factor == 0.0) {
        throw Exception::SassScriptException(
          "$number: Expected " + argument->toString() + " to have an angle unit (deg, grad, rad, turn).",
          logger, pstate->pstate());
      }
      return SASS_MEMORY_NEW(Number, arg->pstate(),
        std::tan(arg->value() * factor));
    }



    static Value* calc_sign2(Logger& logger, const SourceSpan& pstate, const ValueVector& args);

    static Value* calc_exp2(Logger& logger, const SourceSpan& pstate, const ValueVector& args);

    static Value* calc_sqrt(Logger& logger, const SourceSpan& pstate, const ValueVector& args);

    static Value* calc_abs(Logger& logger, const SourceSpan& pstate, const ValueVector& args);

    static Value* calc_sin(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_cos(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_tan(Logger& logger, const SourceSpan& pstate, const ValueVector& argument);

    static Value* calc_asin(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_acos(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_atan(Logger& logger, const SourceSpan& pstate, const ValueVector& argument);

    static Value* calc_min(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_max(Logger& logger, const SourceSpan& pstate, const ValueVector& args);

    static Value* calc_clamp(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_hypot(Logger& logger, const SourceSpan& pstate, const ValueVector& args);

    static Value* calc_pow2(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_mod2(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_rem2(Logger& logger, const SourceSpan& pstate, const ValueVector& args);
    static Value* calc_atan3(Logger& logger, const SourceSpan& pstate, const ValueVector& args);


    static Number* fnAbs(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg);
    static Number* fnExp(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg);
    static Number* fnSign(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg);
    static Value* calc_abs(Logger& logger, const FunctionExpression* pstate, AstNode* argument);
    static Value* calc_exp(Logger& logger, const FunctionExpression* pstate, AstNode* argument);
    static Value* calc_sign(Logger& logger, const FunctionExpression* pstate, AstNode* argument);

    static Number* fnMin(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg);
    static Number* fnMax(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg);

    static Value* calc_pow(Logger& logger, const FunctionExpression* pstate, AstNode* arg1, AstNode* arg2);
    static Value* calc_atan2(Logger& logger, const SourceSpan& pstate, AstNode* arg1, AstNode* arg2);
    static Value* calc_log(Logger& logger, const FunctionExpression* pstate, AstNode* arg1, AstNode* arg2);

    static Value* calc_sqrt(Logger& logger, const FunctionExpression* pstate, AstNode* argument) {
      return singleArgument(logger, pstate, str_sqrt, argument, fnSqrt, true);
    }


    // static Value* calc_sin(Logger& logger, const FunctionExpression* pstate, AstNode* argument) {
    //   return singleArgument(logger, pstate, str_sin, argument, fnSin, true);
    // }
    // 
    // static Value* calc_cos(Logger& logger, const FunctionExpression* pstate, AstNode* argument) {
    //   return singleArgument(logger, pstate, str_cos, argument, fnCos, true);
    // }
    // static Value* calc_tan(Logger& logger, const FunctionExpression* pstate, AstNode* argument) {
    //   return singleArgument(logger, pstate, str_tan, argument, fnTan, true);
    // }

    static Value* calc_round(Logger& logger, Expression* node, const ValueVector& arguments);


    static Value* calc_rem(Logger& logger, const FunctionExpression* pstate, AstNode* lhs, AstNode* rhs);

    static Value* calc_mod(Logger& logger, const FunctionExpression* pstate, AstNode* lhs, AstNode* rhs);

    static Value* calc_fn(Logger& logger, AstNode* argument) {
      AstNodeObj simplified = argument->simplify(logger);
      // debug_ast(simplified, "simplified: ");
      if (auto number = dynamic_cast<Number*>(simplified.ptr())) {
        simplified.detach(); return number;
      }
      else if (auto calc = dynamic_cast<Calculation*>(simplified.ptr())) {
        simplified.detach(); return calc;
      }
      else {
        return SASS_MEMORY_NEW(Calculation,
          argument->pstate(), "calc", { simplified });
      }
    }



  private:

    // Internal constructor that doesn't perform any validation or simplification.
    Calculation32(const sass::string& name, sass::vector<AstNodeObj> arguments) :
      name(name), arguments(arguments) { }

    /*
    /// Simplifies a calculation argument.
    static AstNode* _simplify(AstNode* arg) {
      return nullptr;
      // if (dynamic_cast<Number*>(arg)) {}
      // else if (dynamic_cast<Number*>(arg)) {}
      // //else if (dynamic_cast<CalculationOperation*>(arg)) {}
      // //else if (dynamic_cast<CalculationInterpolation*>(arg)) {}
      // else if (dynamic_cast<String*>(arg)) {}
      // else if (dynamic_cast<Calculation32*>(arg)) {}
      // else if (dynamic_cast<Value*>(arg)) {}

    }
    */
    /*
    = > switch (arg) {
      SassNumber() || CalculationOperation() = > arg,
        CalculationInterpolation() = >
        SassString('(${arg.value})', quotes: false),
        SassString(hasQuotes: false) = > arg,
        SassString() = > throw SassScriptException(
          "Quoted string $arg can't be used in a calculation."),
        SassCalculation(
          name: 'calc',
          arguments : [SassString(hasQuotes:false, : var text)]
        )
        when _needsParentheses(text) = >
        SassString('($text)', quotes: false),
        SassCalculation(name: 'calc', arguments : [var value] ) = > value,
        SassCalculation() = > arg,
        Value() = > throw SassScriptException(
          "Value $arg can't be used in a calculation."),
        _ = > throw ArgumentError("Unexpected calculation argument $arg.")
    };
    */

  };

}

#endif
