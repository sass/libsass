/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "fn_numbers.hpp"

#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Math {

      /*******************************************************************/

      BUILT_IN_FN(round)
      {
        Number* number = arguments[0]->assertNumber(compiler, "number");
        return SASS_MEMORY_NEW(Number, pstate,
          fuzzyRound(number->value(), compiler.epsilon),
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

      /*******************************************************************/

      BUILT_IN_FN(max)
      {
        Number* max = nullptr;
        for (Value* value : arguments[0]->iterator()) {
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
        Number* min = nullptr;
        for (Value* value : arguments[0]->iterator()) {
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

      void registerFunctions(Compiler& ctx)
	    {
		    ctx.registerBuiltInFunction("round", "$number", round);
		    ctx.registerBuiltInFunction("ceil", "$number", ceil);
		    ctx.registerBuiltInFunction("floor", "$number", floor);
		    ctx.registerBuiltInFunction("abs", "$number", abs);
		    ctx.registerBuiltInFunction("max", "$numbers...", max);
		    ctx.registerBuiltInFunction("min", "$numbers...", min);
		    ctx.registerBuiltInFunction("random", "$limit: null", random);
		    ctx.registerBuiltInFunction("unit", "$number", unit);
		    ctx.registerBuiltInFunction("percentage", "$number", percentage);
		    ctx.registerBuiltInFunction("unitless", "$number", isUnitless);
		    ctx.registerBuiltInFunction("comparable", "$number1, $number2", compatible);
	    }

      /*******************************************************************/

    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

  }

}
