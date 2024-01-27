/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "fn_math.hpp"

#include <cmath>
#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"
#include "calculation.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Math {

      /*******************************************************************/

      double coerceToRad(Number* number, Logger& compiler, const sass::string& vname) {
        Units radiants("rad");
        // if (std::isinf(number->value())) return number->value();
        if (double factor = number->getUnitConversionFactor(radiants)) {
          return number->value() * factor;
        }
        callStackFrame csf(compiler, number->pstate());
        throw Exception::RuntimeException(compiler, "$" + vname +
          ": Expected " + number->inspect() + " to be an angle.");
      }

      /*******************************************************************/

      BUILT_IN_FN(round)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        return SASS_MEMORY_NEW(Number, pstate,
          std::round(number->value()),
          number->unit());
      }

      /*******************************************************************/

      BUILT_IN_FN(ceil)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        return SASS_MEMORY_NEW(Number, pstate,
          std::ceil(number->value()),
          number->unit());
      }
      BUILT_IN_FN(fnClamp)
      {

        Number* min = arguments[0]->assertNumber(compiler, "min");
        Number* number = arguments[1]->assertNumber(compiler, "number");
        Number* max = arguments[2]->assertNumber(compiler, "max");
        if (min->hasUnits() == number->hasUnits() && number->hasUnits() == max->hasUnits()) {
          if (min->greaterThanOrEquals(max, compiler, pstate)) return min;
          if (min->greaterThanOrEquals(number, compiler, pstate)) return min;
          if (number->greaterThanOrEquals(max, compiler, pstate)) return max;
          return number;
        }

        auto arg2 = min->hasUnits() != number->hasUnits() ? number : max;
        auto arg2Name = min->hasUnits() != number->hasUnits() ? "$number" : "$max";
        throw Exception::RuntimeException(compiler,
          sass::string(arg2Name) + ": " + arg2->inspect() + " and " +
          "$min: " + min->inspect() + " have incompatible units " +
          "(one has units and the other doesn't).");
      }


      /*******************************************************************/

      BUILT_IN_FN(floor)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        return SASS_MEMORY_NEW(Number, pstate,
          std::floor(number->value()),
          number->unit());
      }

      /*******************************************************************/

      BUILT_IN_FN(abs)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        return SASS_MEMORY_NEW(Number, pstate,
          std::abs(number->value()),
          number->unit());
      }

      BUILT_IN_FN(fnHypot)
      {
        sass::vector<Number*> numbers;
        for (Value* value : arguments[0]->start()) {
          numbers.push_back(value->assertNumber(compiler, ""));
        }

        if (numbers.empty()) {
          throw Exception::RuntimeException(compiler,
            "At least one argument must be passed.");
        }

        auto numeratorUnits = numbers[0]->numerators;
        auto denominatorUnits = numbers[0]->denominators;
        auto subtotal = 0.0;
        for (size_t i = 0; i < numbers.size(); i++) {
          auto& number = numbers[i];
          if (number->hasUnits() == numbers[0]->hasUnits()) {
            if (double factor = number->getUnitConversionFactor(numbers[0])) {
              subtotal += std::pow(number->value() * factor, 2.0);
            }
            else {
              throw Exception::UnitMismatch(compiler, numbers[0], number);
            }
          }
          else {
            sass::sstream strm; strm << (i + 1);
            throw Exception::RuntimeException(compiler,
              "$numbers[" + strm.str() + "]: " + number->inspect() + " and " +
              "$numbers[1]: " + numbers[0]->inspect() + " have incompatible units " +
              "(one has units and the other doesn't).");
          }
        }

        return SASS_MEMORY_NEW(Number, pstate,
          std::sqrt(subtotal), numbers[0]);

      }

      BUILT_IN_FN(fnLog)
      {
        auto number = arguments[0]->assertNumber(compiler, Strings::number);
        if (number->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$number: "
            "Expected " + number->inspect() + " to have no units.");
        }

        if (arguments[1]->isNull()) {
          return SASS_MEMORY_NEW(Number,
            pstate, std::log(number->value()));
        }

        auto base = arguments[1]->assertNumber(compiler, "base");
        if (base->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$base: "
            "Expected " + base->inspect() + " to have no units.");
        }

        return SASS_MEMORY_NEW(Number, pstate,
          std::log(number->value()) / std::log(base->value()));
      }

      BUILT_IN_FN(fnDiv)
      {
        Number* number1 = arguments[0]->isaNumber();
        Number* number2 = arguments[1]->isaNumber();

        if (number1 == nullptr || number2 == nullptr) {
          // callStackFrame csf(compiler, arguments[0]->pstate());
          compiler.printWarning("math.div() will only support number arguments in a future release.\n"
            "Use list.slash() instead for a slash separator.", pstate, Logger::WARN_MATH_DIV);
          // compiler.addWarning();
         }

        return arguments[0]->dividedBy(arguments[1], compiler, pstate);

      }

      BUILT_IN_FN(fnPow)
      {

        auto base = arguments[0]->assertNumber(compiler, "base");
        auto exponent = arguments[1]->assertNumber(compiler, "exponent");
        if (base->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$base: "
            "Expected " + base->inspect() + " to have no units.");
        }
        if (exponent->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$exponent: "
            "Expected " + exponent->inspect() + " to have no units.");
        }

        // Exponentiating certain real numbers leads to special behaviors. Ensure that
        // these behaviors are consistent for numbers within the precision limit.
        auto baseValue = base->value();
        auto expValue = exponent->value();
        return SASS_MEMORY_NEW(Number, pstate,
          std::pow(baseValue, expValue));


      }

      BUILT_IN_FN(fnSqrt)
      {
        auto number = arguments[0]->assertNumber(compiler, "number");
        if (number->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$number: "
            "Expected " + number->inspect() + " to have no units.");
        }
        return SASS_MEMORY_NEW(Number, pstate, std::sqrt(
          number->value()));
      }
      

      /*******************************************************************/

      BUILT_IN_FN(max)
      {
        ValueVector foobar;
        for (Value* value : arguments[0]->start()) {
          foobar.push_back(value);
        }
        return Calculation32::calc_max(compiler, pstate, foobar);
        Number* max = nullptr;
        for (Value* value : arguments[0]->start()) {
          Number* number = value->assertNumber(compiler, "");
          if (max == nullptr || max->lessThan(number, compiler, pstate)) {
            max = number;
          }
        }
        if (max != nullptr) return max;
        // Report invalid arguments error
        throw Exception::SassScriptException(
          "At least one argument must be passed.",
          compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(min)
      {
        ValueVector foobar;
        for (Value* value : arguments[0]->start()) {
          foobar.push_back(value);
        }
        return Calculation32::calc_min(compiler, pstate, foobar);

        Number* min = nullptr;
        for (Value* value : arguments[0]->start()) {
          Number* number = value->assertNumber(compiler, "");
          if (min == nullptr || min->greaterThan(number, compiler, pstate)) {
            min = number;
          }
        }
        if (min != nullptr) return min;
        // Report invalid arguments error
        throw Exception::SassScriptException(
          "At least one argument must be passed.",
          compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(random)
      {
        if (arguments[0]->isNull()) {
          return SASS_MEMORY_NEW(Number, pstate,
            getRandomDouble(0, 1));
        }
        Number* nr = arguments[0]->assertNumber(compiler, "limit");
        long limit = nr->assertInt(compiler, "limit");
        if (limit >= 1) {
          return SASS_MEMORY_NEW(Number, pstate,
            (long) getRandomDouble(1, double(limit) + 1));
        }
        // Report invalid arguments error
        sass::sstream strm;
        strm << "$limit: Must be greater than 0, was " << limit << ".";
        throw Exception::SassScriptException(strm.str(), compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(unit)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        sass::string copy(number->unit());
        return SASS_MEMORY_NEW(String, pstate, std::move(copy), true);
      }

      /*******************************************************************/

      BUILT_IN_FN(isUnitless)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        return SASS_MEMORY_NEW(Boolean, pstate, !number->hasUnits());
      }

      /*******************************************************************/

      BUILT_IN_FN(percentage)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        number->assertUnitless(compiler, "number");
        return SASS_MEMORY_NEW(Number, pstate,
          number->value() * 100, "%");
      }

      /*******************************************************************/

      BUILT_IN_FN(compatible)
      {
        Number* n1 = arguments[0]->assertNumber(compiler, "number1");
        Number* n2 = arguments[1]->assertNumber(compiler, "number2");
        if (n1->isUnitless() || n2->isUnitless()) {
          return SASS_MEMORY_NEW(Boolean, pstate, true);
        }
        // normalize into main units
        n1->normalize(); n2->normalize();
        Units& lhs_unit = *n1, & rhs_unit = *n2;
        bool is_comparable = (lhs_unit == rhs_unit);
        return SASS_MEMORY_NEW(Boolean, pstate, is_comparable);
      }

      /*******************************************************************/

      BUILT_IN_FN(fnCos)
      {
        // if (arguments.size() > 1) throw Exception::TooManyArguments(compiler, arguments.size(), 1);
        // else if (arguments.size() < 1) throw Exception::MissingArgument(compiler, str_angle);
        // AstNode* simplified = arguments[0]->simplify(compiler);
        // auto* number = dynamic_cast<Number*>(simplified);
        // if (number == nullptr) return SASS_MEMORY_NEW(
        //   Calculation, pstate, str_cos, { simplified });
        // double factor = number->factorToUnits(unit_rad);
        // if (factor == 0.0) throw Exception::NoAngleArgument(compiler, number, str_angle);
        // auto result = std::cos(number->value() * factor);
        // return SASS_MEMORY_NEW(Number, number->pstate(), result);


        Number* number = arguments[0]->assertNumber(compiler, Strings::number);
        return SASS_MEMORY_NEW(Number, pstate,
          std::cos(coerceToRad(number, compiler, Strings::number)));
      }

      /*******************************************************************/

      BUILT_IN_FN(fnSin)
      {
        Number* number = arguments[0]->assertNumber(compiler, Strings::number);
        return SASS_MEMORY_NEW(Number, pstate,
          std::sin(coerceToRad(number, compiler, Strings::number)));
      }

      /*******************************************************************/

      BUILT_IN_FN(fnTan)
      {
        Number* number = arguments[0]->assertNumber(compiler, Strings::number);
        // double asymptoteInterval = 0.5 * PI; double tanPeriod = 2.0 * PI;
        return SASS_MEMORY_NEW(Number, pstate,
          std::tan(coerceToRad(number, compiler, Strings::number)));
      }

      /*******************************************************************/

      BUILT_IN_FN(fnACos)
      {
        auto number = arguments[0]->assertNumber(compiler, Strings::number);
        if (number->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$number: "
            "Expected " + number->inspect() + " to have no units.");
        }
        return SASS_MEMORY_NEW(Number, pstate,
          std::acos(number->value()) * 180 / PI, "deg");
      }

      /*******************************************************************/

      BUILT_IN_FN(fnASin)
      {
        auto number = arguments[0]->assertNumber(compiler, Strings::number);
        if (number->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$number: "
            "Expected " + number->inspect() + " to have no units.");
        }
        return SASS_MEMORY_NEW(Number, pstate,
          std::asin(number->value()) * 180 / PI, "deg");
      }

      /*******************************************************************/

      BUILT_IN_FN(fnATan)
      {
        auto number = arguments[0]->assertNumber(compiler, Strings::number);
        if (number->hasUnits()) {
          throw Exception::RuntimeException(compiler, "$number: "
            "Expected " + number->inspect() + " to have no units.");
        }
        return SASS_MEMORY_NEW(Number, pstate,
          std::atan(number->value()) * 180 / PI, "deg");
      }

      /*******************************************************************/

      BUILT_IN_FN(fnATan2)
      {
        auto y = arguments[0]->assertNumber(compiler, "y");
        auto x = arguments[1]->assertNumber(compiler, "x");
        if (y->hasUnits() != x->hasUnits()) {
          throw Exception::RuntimeException(compiler,
            "$x: " + x->inspect() + " and $y: " +
            y->inspect() + " have incompatible units " +
            "(one has units and the other doesn't).");
        }

        if (double factor = x->getUnitConversionFactor(y)) {
          double result = std::atan2(y->value(), x->value() * factor) * 180 / PI;
          return SASS_MEMORY_NEW(Number, pstate, result, "deg");
        }

        throw Exception::UnitMismatch(compiler, y, x);
      }

      /*******************************************************************/

      void registerFunctions(Compiler& ctx)
	    {

        BuiltInMod& module(ctx.createModule("math"));

        module.addVariable(key_e, ctx.createBuiltInVariable(key_e,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"),
            2.71828182845904523536028747135266249775724709369995)));
        module.addVariable(key_pi, ctx.createBuiltInVariable(key_pi,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"),
            3.14159265358979323846264338327950288419716939937510)));
        module.addVariable(key_tau, ctx.createBuiltInVariable(key_tau,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"),
            3.14159265358979323846264338327950288419716939937510 * 2.0)));

        module.addVariable(key_epsilon, ctx.createBuiltInVariable(key_epsilon,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"),
            std::numeric_limits<double>().epsilon())));
        module.addVariable(key_min_number, ctx.createBuiltInVariable(key_min_number,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"),
            std::numeric_limits<double>().min())));
        module.addVariable(key_max_number, ctx.createBuiltInVariable(key_max_number,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"),
            std::numeric_limits<double>().max())));

        module.addVariable(key_min_safe_integer, ctx.createBuiltInVariable(key_min_safe_integer,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"), -9007199254740991)));
        module.addVariable(key_max_safe_integer, ctx.createBuiltInVariable(key_max_safe_integer,
          SASS_MEMORY_NEW(Number, SourceSpan::internal("[sass:math]"), 9007199254740991)));

        module.addFunction(key_ceil, ctx.registerBuiltInFunction(key_ceil, "$number", ceil));
        module.addFunction(key_clamp, ctx.createBuiltInFunction(key_clamp, "$min, $number, $max", fnClamp));
        module.addFunction(key_floor, ctx.registerBuiltInFunction(key_floor, "$number", floor));

        // Some functions are marked internal (for what exactly?)
        module.addFunction(key_max, ctx.registerInternalFunction(key_max, "$numbers...", max));
        module.addFunction(key_min, ctx.registerInternalFunction(key_min, "$numbers...", min));
        module.addFunction(key_round, ctx.registerInternalFunction(key_round, "$number", round));
        module.addFunction(key_abs, ctx.registerInternalFunction(key_abs, "$number", abs));

        module.addFunction(key_hypot, ctx.createBuiltInFunction(key_hypot, "$number...", fnHypot));

        module.addFunction(key_log, ctx.createBuiltInFunction(key_log, "$number, $base: null", fnLog));
        module.addFunction(key_pow, ctx.createBuiltInFunction(key_pow, "$base, $exponent", fnPow));
        module.addFunction(key_div, ctx.createBuiltInFunction(key_div, "$number1, $number2", fnDiv));
        module.addFunction(key_sqrt, ctx.createBuiltInFunction(key_sqrt, "$number", fnSqrt));
        module.addFunction(key_cos, ctx.createBuiltInFunction(key_cos, "$number", fnCos));
        module.addFunction(key_sin, ctx.createBuiltInFunction(key_sin, "$number", fnSin));
        module.addFunction(key_tan, ctx.createBuiltInFunction(key_tan, "$number", fnTan));
        module.addFunction(key_acos, ctx.createBuiltInFunction(key_acos, "$number", fnACos));
        module.addFunction(key_asin, ctx.createBuiltInFunction(key_asin, "$number", fnASin));
        module.addFunction(key_atan, ctx.createBuiltInFunction(key_atan, "$number", fnATan));
        module.addFunction(key_atan2, ctx.createBuiltInFunction(key_atan2, "$y, $x", fnATan2));
        module.addFunction(key_random, ctx.registerBuiltInFunction(key_random, "$limit: null", random));
        module.addFunction(key_unit, ctx.registerBuiltInFunction(key_unit, "$number", unit));
        module.addFunction(key_percentage, ctx.registerBuiltInFunction(key_percentage, "$number", percentage));
        module.addFunction(key_is_unitless, ctx.registerBuiltInFunction(key_unitless, "$number", isUnitless));
        module.addFunction(key_compatible, ctx.registerBuiltInFunction(key_comparable, "$number1, $number2", compatible));

      }

      /*******************************************************************/

    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}
