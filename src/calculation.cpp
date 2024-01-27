/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "calculation.hpp"

#include "eval.hpp"

namespace Sass {


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /// Verifies that all the numbers in [args] aren't known to be incompatible
  /// with one another, and that they don't have units that are too complex for
  /// calculations.
  static void _verifyCompatibleNumbers3(Logger& logger, const SourceSpan& pstate, const ValueVector& args, bool strict = true) {
    // Note: this logic is largely duplicated in
    // _EvaluateVisitor._verifyCompatibleNumbers and most changes here should
    // also be reflected there.
    for (auto arg : args) {
      if (Number* nr = dynamic_cast<Number*>(arg.ptr())) {
        if (nr->isValidCssUnit() == false) {
          throw Exception::IncompatibleCalcValue(
            logger, *arg, nr->pstate());
        }
      }
    }

    for (unsigned int i = 0; i < args.size() - 1; i++) {
      if (Number* nr1 = dynamic_cast<Number*>(args[i].ptr())) {
        for (unsigned int j = i + 1; j < args.size(); j++) {
          if (Number* nr2 = dynamic_cast<Number*>(args[j].ptr())) {
            // if (number1.hasPossiblyCompatibleUnits(number2)) continue;
            if (nr1->hasPossiblyCompatibleUnits(nr2, strict)) continue;
            throw Exception::UnitMismatch(logger, nr1, nr2);
          }
        }
      }
    }
  }

  static void _verifyCompatibleNumbers2(Logger& logger, const SourceSpan& pstate, sass::vector<AstNode*> args, bool strict = true) {
    // Note: this logic is largely duplicated in
    // _EvaluateVisitor._verifyCompatibleNumbers and most changes here should
    // also be reflected there.
    for (auto arg : args) {
      if (Number* nr = dynamic_cast<Number*>(arg)) {
        if (nr->isValidCssUnit() == false) {
          throw Exception::IncompatibleCalcValue(
            logger, *arg, nr->pstate());
        }
      }
    }

    for (unsigned int i = 0; i < args.size() - 1; i++) {
      if (Number* nr1 = dynamic_cast<Number*>(args[i])) {
        for (unsigned int j = i + 1; j < args.size(); j++) {
          if (Number* nr2 = dynamic_cast<Number*>(args[j])) {
            // if (number1.hasPossiblyCompatibleUnits(number2)) continue;
            if (nr1->hasPossiblyCompatibleUnits(nr2, strict)) continue;
            throw Exception::UnitMismatch(logger, nr1, nr2);
          }
        }
      }
    }
  }

  static void _verifyLength(Logger& logger, const ValueVector& args, size_t len)
  {
    if (args.size() == len) return;
    for (const auto& arg : args)
      if (arg->isaString()) return;
    if (args.size() > len) throw Exception::TooManyArguments(logger, args.size(), len);
    else if (args.size() < len) throw Exception::TooFewArguments(logger, args.size(), len);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  const double NaN = std::numeric_limits<double>::quiet_NaN();

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Value* Calculation32::calc_sqrt(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_sqrt, { simplified });
    number->assertUnitless(logger, str_number);
    auto result = std::sqrt(number->value());
    return SASS_MEMORY_NEW(Number, number->pstate(), result);
  }

  Value* Calculation32::calc_sign2(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_sign, { simplified });
    double result = 0; 
    if (number->value() == 0.0) result = number->value();
    else if (std::isnan(number->value())) result = NaN;
    else result = std::signbit(number->value()) ? -1 : 1;
    return SASS_MEMORY_NEW(Number, number->pstate(), result, number);
  }

  Value* Calculation32::calc_exp2(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_exp, { simplified });
    number->assertUnitless(logger, str_number);
    double result = 0;
    if (std::isnan(number->value())) result = NaN;
    else result = std::exp(number->value());
    return SASS_MEMORY_NEW(Number, number->pstate(), result);
  }

  Value* Calculation32::calc_abs(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    if (const auto str = dynamic_cast<String*>(simplified.ptr())) {
      if (str->isVar()) return SASS_MEMORY_NEW(
        Calculation, pstate, str_abs, { simplified });
    }
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_abs, { simplified });
    auto result = std::abs(number->value());
    return SASS_MEMORY_NEW(Number, number->pstate(), result, number);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Value* Calculation32::calc_sin(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_angle);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_sin, { simplified });
    double factor = number->factorToUnits(unit_rad);
    if (factor == 0.0) throw Exception::NoAngleArgument(logger, number, str_angle);
    auto result = std::sin(number->value() * factor);
    return SASS_MEMORY_NEW(Number, number->pstate(), result);
  }

  Value* Calculation32::calc_cos(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_angle);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_cos, { simplified });
    double factor = number->factorToUnits(unit_rad);
    if (factor == 0.0) throw Exception::NoAngleArgument(logger, number, str_angle);
    auto result = std::cos(number->value() * factor);
    return SASS_MEMORY_NEW(Number, number->pstate(), result);
  }

  Value* Calculation32::calc_tan(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_angle);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_tan, { simplified });
    double factor = number->factorToUnits(unit_rad);
    if (factor == 0.0) throw Exception::NoAngleArgument(logger, number, str_angle);
    auto result = std::tan(number->value() * factor);
    return SASS_MEMORY_NEW(Number, number->pstate(), result);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Value* Calculation32::calc_asin(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_asin, { simplified });
    number->assertNoUnits(logger, str_number);
    auto degs = std::asin(number->value()) * Constants::Math::RAD_TO_DEG;
    return SASS_MEMORY_NEW(Number, number->pstate(), degs, unit_deg);
  }

  Value* Calculation32::calc_acos(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_acos, { simplified });
    number->assertNoUnits(logger, str_number);
    auto degs = std::acos(number->value()) * Constants::Math::RAD_TO_DEG;
    return SASS_MEMORY_NEW(Number, number->pstate(), degs, unit_deg);
  }

  Value* Calculation32::calc_atan(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 1) throw Exception::TooManyArguments(logger, args.size(), 1);
    else if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj simplified = args[0]->simplify(logger);
    auto* number = dynamic_cast<Number*>(simplified.ptr());
    if (number == nullptr) return SASS_MEMORY_NEW(
      Calculation, pstate, str_atan, { simplified });
    number->assertNoUnits(logger, str_number);  
    auto degs = std::atan(number->value()) * Constants::Math::RAD_TO_DEG;
    return SASS_MEMORY_NEW(Number, number->pstate(), degs, unit_deg);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Value* Calculation32::calc_pow2(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    _verifyLength(logger, args, 2); // Allows string to pass always
    // else if (args.size() < 2) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj arg_dividend = args[0]->simplify(logger);
    AstNodeObj arg_modulus = args.size() > 1 ? args[1]->simplify(logger) : nullptr;
    // _verifyLength(args, 2); _verifyCompatibleNumbers(args);
    // _verifyCompatibleNumbers2(logger, pstate, { arg_dividend, arg_modulus });
    if (NumberObj nr_base = dynamic_cast<Number*>(arg_dividend.ptr())) {
      if (NumberObj nr_exp = dynamic_cast<Number*>(arg_modulus.ptr())) {
        nr_base->assertNoUnits(logger, str_base);
        nr_exp->assertNoUnits(logger, str_exp);
        return SASS_MEMORY_NEW(Number, pstate,
          std::pow(nr_base->value(), nr_exp->value()));
      }
    }
    return SASS_MEMORY_NEW(Calculation, pstate,
      str_pow, { arg_dividend, arg_modulus });
  }

  Value* Calculation32::calc_atan3(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 2) throw Exception::TooManyArguments(logger, args.size(), 2);
    else if (args.size() < 2) throw Exception::TooFewArguments(logger, args.size(), 2);
    // else if (args.size() < 2) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj arg_dividend = args[0]->simplify(logger);
    AstNodeObj arg_modulus = args.size() > 1 ? args[1]->simplify(logger) : nullptr;
    // _verifyLength(args, 2); _verifyCompatibleNumbers(args);
    // _verifyCompatibleNumbers2(logger, pstate, { arg_dividend, arg_modulus });
    _verifyCompatibleNumbers2(logger, pstate, { arg_dividend, arg_modulus }, true);
    if (NumberObj nr_base = dynamic_cast<Number*>(arg_dividend.ptr())) {
      if (NumberObj nr_exp = dynamic_cast<Number*>(arg_modulus.ptr())) {
        if (!((Units)*nr_base == unit_percent) || !((Units)*nr_exp == unit_percent)) {
          double factor = nr_exp->getUnitConversionFactor(nr_base, true);
          if (factor != 0 && nr_base->hasCompatibleUnits(nr_exp)) {
            auto rads = std::atan2(nr_base->value(), nr_exp->value() * factor);
            return SASS_MEMORY_NEW(Number, pstate, rads * Constants::Math::RAD_TO_DEG, unit_deg);
          }
        }
      }
    }
    return SASS_MEMORY_NEW(Calculation, pstate,
      str_atan2, { arg_dividend, arg_modulus });
  }


  Value* Calculation32::calc_mod2(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 2) throw Exception::TooManyArguments(logger, args.size(), 2);
    else if (args.size() < 2) throw Exception::TooFewArguments(logger, args.size(), 2);
    // else if (args.size() < 2) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj arg_dividend = args[0]->simplify(logger);
    AstNodeObj arg_modulus = args.size() > 1 ? args[1]->simplify(logger) : nullptr;
    // _verifyLength(args, 2); _verifyCompatibleNumbers(args);
    _verifyCompatibleNumbers2(logger, pstate, { arg_dividend, arg_modulus });
    if (NumberObj dividend_nr = dynamic_cast<Number*>(arg_dividend.ptr())) {
      if (NumberObj modulus_nr = dynamic_cast<Number*>(arg_modulus.ptr())) {
        double factor = dividend_nr->getUnitConversionFactor(modulus_nr, true);
        if (factor == 0.0) {
          if (dividend_nr->isCustomUnit() || modulus_nr->isCustomUnit()) {
            return SASS_MEMORY_NEW(Calculation, pstate,
              str_mod, { arg_dividend, arg_modulus });
          }
          throw Exception::UnitMismatch(logger, dividend_nr, modulus_nr);
        }
        return dividend_nr->modulo(modulus_nr, logger, pstate);
      }
    }
    return SASS_MEMORY_NEW(Calculation, pstate,
      str_mod, { arg_dividend, arg_modulus });
  }


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /// Creates a `rem()` calculation with the given [dividend] and [modulus].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation].
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  ///
  /// This may be passed fewer than two arguments, but only if one of the
  /// arguments is an unquoted `var()` string.
  Value* Calculation32::calc_rem2(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() > 2) throw Exception::TooManyArguments(logger, args.size(), 2);
    else if (args.size() < 2) throw Exception::TooFewArguments(logger, args.size(), 2);
    AstNodeObj arg_dividend = args[0]->simplify(logger);
    AstNodeObj arg_modulus = args.size() > 1 ? args[1]->simplify(logger) : nullptr;
    // _verifyLength(args, 2); _verifyCompatibleNumbers(args);
    _verifyCompatibleNumbers2(logger, pstate, { arg_dividend, arg_modulus });
    if (NumberObj dividend_nr = dynamic_cast<Number*>(arg_dividend.ptr())) {
      if (NumberObj modulus_nr = dynamic_cast<Number*>(arg_modulus.ptr())) {
        double factor = dividend_nr->getUnitConversionFactor(modulus_nr, true);
        if (factor == 0.0) {
          if (dividend_nr->isCustomUnit() || modulus_nr->isCustomUnit()) {
            return SASS_MEMORY_NEW(Calculation, pstate,
              str_rem, { arg_dividend, arg_modulus });
          }
          throw Exception::UnitMismatch(logger, dividend_nr, modulus_nr);
        }
        NumberObj result = Cast<Number>(dividend_nr->modulo(modulus_nr, logger, pstate));
        double div = dividend_nr->value(), mod = modulus_nr->value();
        if (std::signbit(div) == std::signbit(mod)) return result.detach();
        if (std::isinf(mod)) return dividend_nr.detach();
        if (result->value() == 0.0) return result->unaryMinus(logger, pstate);
        return result->minus(modulus_nr, logger, pstate);
      }
    }
    return SASS_MEMORY_NEW(Calculation, pstate,
      str_rem, { arg_dividend, arg_modulus });
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /// Creates a `clamp()` calculation with the given [min], [value], and [max].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation].
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  ///
  /// This may be passed fewer than three arguments, but only if one of the
  /// arguments is an unquoted `var()` string.
  Value* Calculation32::calc_clamp(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.size() < 1) throw Exception::MissingArgument(logger, str_number);
    AstNodeObj arg_min = args[0]->simplify(logger);
    AstNodeObj arg_val = args.size() > 1 ? args[1]->simplify(logger) : nullptr;
    AstNodeObj arg_max = args.size() > 2 ? args[2]->simplify(logger) : nullptr;
    NumberObj nr_min = Cast<Number>(arg_min);
    NumberObj nr_val = Cast<Number>(arg_val);
    NumberObj nr_max = Cast<Number>(arg_max);

    if (nr_min && nr_val && nr_max) {
      if (nr_min->hasCompatibleUnits(nr_val) && nr_max->hasCompatibleUnits(nr_val)) {
        if (nr_val->lessThanOrEquals(nr_min, logger, pstate)) return nr_min.detach();
        if (nr_val->greaterThanOrEquals(nr_max, logger, pstate)) return nr_max.detach();
        else return nr_val.detach();
      }
    }
    _verifyCompatibleNumbers2(logger, pstate, { nr_min, nr_val });
    _verifyCompatibleNumbers2(logger, pstate, { nr_min, nr_max });
    _verifyLength(logger, args, 3); // Allows string to pass always
    return SASS_MEMORY_NEW(Calculation, pstate,
      str_clamp, { arg_min, arg_val, arg_max });
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /// Creates a `min()` calculation with the given [arguments].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation]. It must be passed at
  /// least one argument.
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  Value* Calculation32::calc_min(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.empty()) throw Exception::MustHaveArguments(logger, str_min);
    sass::vector<AstNodeObj> simplified(args.size());
    std::transform(args.begin(), args.end(),
      simplified.begin(), [&](ValueObj value) {
        return value->simplify(logger);
      });
    // find min number now
    NumberObj min = nullptr;
    for (size_t i = 0; i < args.size(); i++) {
      Value* val = Cast<Value>(simplified[i]);
      if (val->isaCalculation() || val->isaCalcOperation()) {
        _verifyCompatibleNumbers3(logger, pstate, args);
        return SASS_MEMORY_NEW(Calculation,
          pstate, str_min, std::move(simplified));
      }
      if (auto str = val->isaString()) {
        //_verifyCompatibleNumbers3(logger, pstate, args);
        if (str->isVar()) {
          _verifyCompatibleNumbers3(logger, pstate, args);
          return SASS_MEMORY_NEW(Calculation,
            pstate, str_min, std::move(simplified));
        }
      }
      Number* nr = val->assertNumber(logger, str_empty);
      if (nr == nullptr) {
        _verifyCompatibleNumbers3(logger, pstate, args);
        return SASS_MEMORY_NEW(Calculation,
          pstate, str_min, std::move(simplified));
      }
      if (min == nullptr) { min = nr; continue; }
      // _verifyCompatibleNumbers2(logger, pstate, { min, nr }, false);
      double factor = nr->getUnitConversionFactor(min, false);
      if (factor == 0.0) {
        // throw Exception::UnitMismatch(logger, min, nr);
        _verifyCompatibleNumbers3(logger, pstate, args);
        return SASS_MEMORY_NEW(Calculation,
          pstate, str_min, std::move(simplified));
      }
      if (min->value() > nr->value() * factor) min = nr;
    }

    // Return min number
    return min.detach();
  }

  /// Creates a `max()` calculation with the given [arguments].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation]. It must be passed at
  /// least one argument.
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  Value* Calculation32::calc_max(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
   // std::cerr << "==== Execute max\n";
    if (args.empty()) throw Exception::MustHaveArguments(logger, str_max);
    sass::vector<AstNodeObj> simplified(args.size());
    std::transform(args.begin(), args.end(),
      simplified.begin(), [&](ValueObj value) {
        return value->simplify(logger);
      });
    // find max number now
    NumberObj max = nullptr;
    for (size_t i = 0; i < simplified.size(); i++) {
      Value* val = Cast<Value>(simplified[i]);
      if (val->isaCalculation() || val->isaCalcOperation()) {
        _verifyCompatibleNumbers3(logger, pstate, args);
        return SASS_MEMORY_NEW(Calculation,
          pstate, str_max, std::move(simplified));
      }
      if (auto str = val->isaString()) {
        if (str->isVar()) {
          _verifyCompatibleNumbers3(logger, pstate, args);
          return SASS_MEMORY_NEW(Calculation,
            pstate, str_max, std::move(simplified));
        }
      }
      Number* nr = val->assertNumber(logger, str_empty);
      if (nr == nullptr) {
        _verifyCompatibleNumbers3(logger, pstate, args);
        return SASS_MEMORY_NEW(Calculation,
          pstate, str_max, std::move(simplified));
      }
      if (max == nullptr) { max = nr; continue; }
      //_verifyCompatibleNumbers2(logger, pstate, { max, nr }, false);
      double factor = nr->getUnitConversionFactor(max, false);
      if (factor == 0.0) {
        _verifyCompatibleNumbers3(logger, pstate, args);
        return SASS_MEMORY_NEW(Calculation,
          pstate, str_max, std::move(simplified));
      }
      if (max->value() < nr->value() * factor) max = nr;
    }
    // Return max number
    return max.detach();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /// Creates a `hypot()` calculation with the given [arguments].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation]. It must be passed at
  /// least one argument.
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  Value* Calculation32::calc_hypot(Logger& logger, const SourceSpan& pstate, const ValueVector& args)
  {
    if (args.empty()) throw Exception::MustHaveArguments(logger, str_max);
    sass::vector<AstNodeObj> simplified(args.size());
    std::transform(args.begin(), args.end(),
      simplified.begin(), [&](ValueObj value) {
        return value->simplify(logger);
      });
    _verifyCompatibleNumbers3(logger, pstate, args);
    Number* first = Cast<Number>(simplified[0]);
    if (first == nullptr) {
      return SASS_MEMORY_NEW(Calculation, pstate,
        str_hypot, std::move(simplified));
    }
    else if (first->hasUnit("%")) {
      return SASS_MEMORY_NEW(Calculation, pstate,
        str_hypot, std::move(simplified));
    }
    else {
      double subtotal = first->value() * first->value();
      for (size_t i = 1; i < simplified.size(); i++) {
        Number* next = Cast<Number>(args[i]);
        if (next == nullptr) return SASS_MEMORY_NEW(Calculation,
          pstate, str_hypot, std::move(simplified));
        _verifyCompatibleNumbers2(logger, pstate, { first, next });
        double factor = next->getUnitConversionFactor(first);
        if (factor == 0.0) {
          if (first->isCustomUnit() || next->isCustomUnit()) {
            return SASS_MEMORY_NEW(Calculation, pstate,
              str_hypot, std::move(simplified));
          }
          throw Exception::UnitMismatch(logger, first, next);
        }
        double value = next->value() * factor; // convert
        subtotal += value * value; // square it
      }
      // Return the result in units of the first number
      return SASS_MEMORY_NEW(Number, pstate,
        std::sqrt(subtotal), first);
    }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////


  // Returns [value] coerced to [number]'s units.
  static Number* _matchUnits(double value, Number* number)
  {
    return SASS_MEMORY_NEW(Number, number->pstate(), value, number);
  }


  Number* Calculation32::roundWithStep(
    const sass::string& strategy,
    Number* number, Number* step)
  {

    // std::cerr << "roundWithStep " << step->value() << "\n";

    if (std::isinf(step->value())) {
      // std::cerr << "Step is infinite\n";
      if (number->value() == 0) {
        return number;
      }
      else if (std::isinf(number->value())) {
        return _matchUnits(NaN, number);
      }
      else if (strategy == str_up) {
        return number->value() > 0
          ? _matchUnits(std::numeric_limits<double>::infinity(), number)
          : _matchUnits(-0.0, number);
      }
      else if (strategy == str_down) {
        return number->value() < 0
          ? _matchUnits(-std::numeric_limits<double>::infinity(), number)
          : _matchUnits(0.0, number);
      }
      else if (strategy == str_nearest || strategy == str_to_zero) {
        // std::cerr << "Return from match units\n";
        if (std::isinf(number->value())) {
          return _matchUnits(-NaN, number);
        }
        return number->value() > 0
          ? _matchUnits(0.0, number)
          : _matchUnits(-0.0, number);
      }
      else {
        std::cerr << "############ NADADAD\n";
        std::cerr << "############ NADADAD\n";
        std::cerr << "############ NADADAD\n";
      }
    }

    auto factor = step->getUnitConversionFactor(number);
    double stepWithNumberUnit = step->value() * factor;

    if (strategy == str_nearest) {
      return _matchUnits(std::round(
        number->value() / stepWithNumberUnit
      ) * stepWithNumberUnit, number);
    }
    else if (strategy == str_up) {
      return _matchUnits((step->value() < 0
        ? std::floor(number->value() / stepWithNumberUnit)
        : std::ceil(number->value() / stepWithNumberUnit)
        ) * stepWithNumberUnit, number);
    }
    else if (strategy == str_down) {
      return _matchUnits((step->value() < 0
        ? std::ceil(number->value() / stepWithNumberUnit)
        : std::floor(number->value() / stepWithNumberUnit)
        ) * stepWithNumberUnit, number);
    }
    else if (strategy == str_to_zero) {
      return _matchUnits((number->value() < 0
        ? std::ceil(number->value() / stepWithNumberUnit)
        : std::floor(number->value() / stepWithNumberUnit)
        ) * stepWithNumberUnit, number);
    }
    // This should not be reached, strategy is checked before
    return SASS_MEMORY_NEW(Number, number->pstate(), 1442.1442);
  }


  /// Verifies that all the numbers in [args] aren't known to be incompatible
  /// with one another, and that they don't have units that are too complex for
  /// calculations.
  void Eval::_verifyCompatibleNumbers(sass::vector<AstNode*> args, const SourceSpan& pstate) {
    _verifyCompatibleNumbers2(logger, pstate, args, true);
  }


  Value* Eval::operateInternal(
    const SourceSpan& pstate, SassOperator op,
    AstNode* left, AstNode* right,
    bool inLegacySassFunction, bool simplify)
  {
    // if (!simplify) return CalculationOperation._(operator, left, right);

    if (!simplify) {
      return SASS_MEMORY_NEW(CalcOperation,
        pstate, op, left, right);
    }

    //debug_ast(left, "left: ");
    //debug_ast(right, "right: ");

    AstNodeObj lhs = left->simplify(logger);
    AstNodeObj rhs = right->simplify(logger);


    Number* lnr = dynamic_cast<Number*>(lhs.ptr());
    Number* rnr = dynamic_cast<Number*>(rhs.ptr());

    if (op == ADD || op == SUB) {
      if (lnr != nullptr && rnr != nullptr) {
        if (inLegacySassFunction)
        {
          if (lnr->canCompareTo(rnr, false)) {
            return op == ADD
              ? lnr->plus(rnr, logger, pstate)
              : lnr->minus(rnr, logger, pstate);
          }
        }
        else
        {
          if (lnr->hasCompatibleUnits(rnr, true)) {
            return op == ADD
              ? lnr->plus(rnr, logger, pstate)
              : lnr->minus(rnr, logger, pstate);
          }
        }
        // inLegacySassFunction
          // ? left.isComparableTo(right)
          // : left.hasCompatibleUnits(right))

        // if (inLegacySassFunction
        //   ? left.isComparableTo(right)
        //   : left.hasCompatibleUnits(right)) {
        //   return operator == CalculationOperator.plus
        //     ? left.plus(right)
        //     : left.minus(right);
        // }
      }

      // std::cerr << "VERIFY NUMBERS " << inLegacySassFunction << "\n";
      _verifyCompatibleNumbers2(logger, pstate, { left, right }, true);
      // std::cerr << "Numbers are verified\n";


      return SASS_MEMORY_NEW(CalcOperation,
        pstate, op, lhs.ptr(), rhs.ptr());


    }
    else if (lnr != nullptr && rnr != nullptr) {
      return op == MUL
        ? lnr->times(rnr, logger, pstate)
        : lnr->dividedBy(rnr, logger, pstate);
    }
    else {
      return SASS_MEMORY_NEW(CalcOperation,
        pstate, op, lhs.ptr(), rhs.ptr());
      throw std::runtime_error("Not Implemented 33");
    }
    // 
  }


  /*


  static Object operateInternal(
      CalculationOperator operator, Object left, Object right,
      {required bool inLegacySassFunction, required bool simplify}) {

      if (!simplify) return CalculationOperation._(operator, left, right);
    left = _simplify(left);
    right = _simplify(right);

    if (operator case CalculationOperator.plus || CalculationOperator.minus) {
      if (left is SassNumber &&
          right is SassNumber &&
          (inLegacySassFunction
              ? left.isComparableTo(right)
              : left.hasCompatibleUnits(right))) {
        return operator == CalculationOperator.plus
            ? left.plus(right)
            : left.minus(right);
      }

      _verifyCompatibleNumbers([left, right]);

      if (right is SassNumber && number_lib.fuzzyLessThan(right.value, 0)) {
        right = right.times(SassNumber(-1));
        operator = operator == CalculationOperator.plus
            ? CalculationOperator.minus
            : CalculationOperator.plus;
      }

      return CalculationOperation._(operator, left, right);
    } else if (left is SassNumber && right is SassNumber) {
      return operator == CalculationOperator.times
          ? left.times(right)
          : left.dividedBy(right);
    } else {
      return CalculationOperation._(operator, left, right);
    }
  }
  */

  // Creates a `round()` calculation with the given [strategyOrNumber],
  // [numberOrStep], and [step]. Strategy must be either nearest,
  // up, down or to-zero.
  //
  // Number and step must be either a [SassNumber], a [SassCalculation],
  // an unquoted [SassString], or a [CalculationOperation].
  //
  // This automatically simplifies the calculation, so it may return a
  // [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  // can determine that the calculation will definitely produce invalid CSS.
  //
  // This may be passed fewer than two arguments, but only if one of the
  // arguments is an unquoted `var()` string.




  Number* Calculation32::fnAbs(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg)
  {
    return SASS_MEMORY_NEW(Number, arg->pstate(), std::abs(arg->value()), arg);
  }

  Number* Calculation32::fnExp(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg)
  {
    return SASS_MEMORY_NEW(Number, arg->pstate(), std::exp(arg->value()), arg);
  }

  Number* Calculation32::fnSign(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg)
  {
    return SASS_MEMORY_NEW(Number, arg->pstate(), arg->value() < 0 ? -1 : arg->value() > 0 ? 1 : 0, arg);
  }

  Value* Calculation32::calc_abs(Logger& logger, const FunctionExpression* pstate, AstNode* argument)
  {
    return singleArgument(logger, pstate, str_abs, argument, fnAbs, true);
  }

  Value* Calculation32::calc_exp(Logger& logger, const FunctionExpression* pstate, AstNode* argument)
  {
    return singleArgument(logger, pstate, str_exp, argument, fnExp, true);
  }

  Value* Calculation32::calc_sign(Logger& logger, const FunctionExpression* pstate, AstNode* argument)
  {
    return singleArgument(logger, pstate, str_sign, argument, fnSign, true);
  }


  Number* Calculation32::fnMin(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg)
  {
    return SASS_MEMORY_NEW(Number, arg->pstate(), std::exp(arg->value()), arg);
  }

  Number* Calculation32::fnMax(Logger& logger, const FunctionExpression* pstate, AstNode* argument, Number* arg)
  {
    return SASS_MEMORY_NEW(Number, arg->pstate(), std::exp(arg->value()), arg);
  }




  /// Creates a `pow()` calculation with the given [base] and [exponent].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation].
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  ///
  /// This may be passed fewer than two arguments, but only if one of the
  /// arguments is an unquoted `var()` string.
  Value* Calculation32::calc_pow(Logger& logger, const FunctionExpression* pstate, AstNode* arg1, AstNode* arg2)
  {
    AstNodeObj base = arg1->simplify(logger);
    AstNodeObj exponent = nullptr;
    if (arg2) exponent = arg2->simplify(logger);
    Number* nr_base = Cast<Number>(base);
    Number* nr_exp = Cast<Number>(exponent);
    if (nr_base && nr_exp) {
      return SASS_MEMORY_NEW(Number, pstate->pstate(),
        std::pow(nr_base->value(), nr_exp->value()));
    }
    // Otherwise return calculation literal
    return SASS_MEMORY_NEW(Calculation,
      pstate->pstate(), str_pow,
      { base, exponent });
  }

  /// Creates a `log()` calculation with the given [number] and [base].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation].
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  ///
  /// If arguments contains exactly a single argument, the base is set to
  /// `math.e` by default.
  Value* Calculation32::calc_log(Logger& logger, const FunctionExpression* pstate, AstNode* number, AstNode* base)
  {
    AstNodeObj arg_nr = number->simplify(logger);
    AstNodeObj arg_base = base ? base->simplify(logger) : nullptr;
    if (Number* nr_value = Cast<Number>(arg_nr)) {
      nr_value->assertNoUnits(logger, str_number);
      if (arg_base == nullptr) {
        return SASS_MEMORY_NEW(Number, pstate->pstate(),
          std::log(nr_value->value())); // Regular log
      }
      if (Number* nr_base = Cast<Number>(arg_base)) {
        nr_base->assertNoUnits(logger, str_base);
        return SASS_MEMORY_NEW(Number, pstate->pstate(),
          std::log(nr_value->value()) / std::log(nr_base->value()));
      }
    }
    // Otherwise return calc literal
    return SASS_MEMORY_NEW(Calculation,
      pstate->pstate(), str_log,
      { arg_nr, arg_base });
  }
  // EO calc_log




  /// Creates a `atan2()` calculation for [y] and [x].
  ///
  /// Each argument must be either a [SassNumber], a [SassCalculation], an
  /// unquoted [SassString], or a [CalculationOperation].
  ///
  /// This automatically simplifies the calculation, so it may return a
  /// [SassNumber] rather than a [SassCalculation]. It throws an exception if it
  /// can determine that the calculation will definitely produce invalid CSS.
  ///
  /// This may be passed fewer than two arguments, but only if one of the
  /// arguments is an unquoted `var()` string.
  Value* Calculation32::calc_atan2(Logger& logger, const SourceSpan& pstate, AstNode* y, AstNode* x)
  {
    AstNodeObj arg_y = y->simplify(logger);
    AstNodeObj arg_x = x ? x->simplify(logger) : nullptr;
    // _verifyLength(args, 2);
    if (Number* nr_y = Cast<Number>(arg_y)) {
      if (!nr_y->isValidCssUnit()) throw Exception::IncompatibleCalcValue(logger, *nr_y, y->pstate());
      if (Number* nr_x = Cast<Number>(arg_x)) {
        if (!nr_x->isValidCssUnit()) throw Exception::IncompatibleCalcValue(logger, *nr_x, x->pstate());
        if (!(unit_percent == nr_y || unit_percent == nr_x)) {
          double factor = nr_x->getUnitConversionFactor(nr_y);
          if (factor != 0) return SASS_MEMORY_NEW(Number, pstate,
            std::atan2(nr_y->value(), nr_x->value() * factor)
              * Constants::Math::RAD_TO_DEG, unit_deg);
        }
      }
    }
    // Otherwise return calc literal
    return SASS_MEMORY_NEW(Calculation,
      pstate, str_atan2, { arg_y, arg_x });
  }
  // EO calc_atan2


  Value* Calculation32::calc_round(Logger& logger, Expression* node, const ValueVector& arguments)
  {
    // std::cerr << "CALCULATE ROUND\n";

    if (arguments.size() == 0)
    {
      throw Exception::MissingArgument(logger, str_number);
    }
    else if (arguments.size() == 1)
    {
      AstNodeObj arg_0 = arguments[0]->simplify(logger);
      if (auto nr_number = dynamic_cast<Number*>(arg_0.ptr())) {
        return _matchUnits(std::round(nr_number->value()), nr_number);
      }
      if (auto str_number = dynamic_cast<String*>(arg_0.ptr())) {
        return SASS_MEMORY_NEW(Calculation, node->pstate(), str_round, { str_number });
      }
      callStackFrame frame(logger, arg_0->pstate());
      throw Exception::SassScriptException("Single argument " +
        arg_0->toString() + " expected to be simplifiable.",
        logger, arg_0->pstate()
      );

    }
    else if (arguments.size() == 2)
    {
      AstNodeObj arg_0 = arguments[0]->simplify(logger);
      AstNodeObj arg_1 = arguments[1]->simplify(logger);
      if (auto nr_number = dynamic_cast<Number*>(arg_0.ptr())) {
        if (auto nr_step = dynamic_cast<Number*>(arg_1.ptr())) {
          _verifyCompatibleNumbers2(logger,
            node->pstate(), { nr_number, nr_step });
          if (nr_number->hasCompatibleUnits(nr_step, true)) {
            return roundWithStep(str_nearest, nr_number, nr_step);
          }
          else {
            return new Calculation(node->pstate(),
              "round", { arg_0, arg_1 });
          }
        } 
      }
      if (String* strategy = dynamic_cast<String*>(arg_0.ptr()))
      {
        const sass::string& method(strategy->value());
        if (method == str_nearest || method == str_up ||
          method == str_to_zero || method == str_down)
        {
          if (String* str_number = dynamic_cast<String*>(arg_1.ptr())) {
            if (str_number->isVar()) {
              return SASS_MEMORY_NEW(Calculation, node->pstate(),
                str_round, { arguments[0].ptr(), arguments[1].ptr() });
            }
          }
          throw Exception::SassScriptException(logger, node->pstate(),
            "If strategy is not null, step is required.");
        }
      }
      return SASS_MEMORY_NEW(Calculation, node->pstate(),
        str_round, { arguments[0].ptr(), arguments[1].ptr() });
    }
    else if (arguments.size() == 3)
    {
      AstNodeObj arg_0 = arguments[0]->simplify(logger);
      AstNodeObj arg_1 = arguments[1]->simplify(logger);
      AstNodeObj arg_2 = arguments[2]->simplify(logger);
      if (String* strategy = dynamic_cast<String*>(arg_0.ptr()))
      {
        const sass::string& method(strategy->value());
        if (method == str_nearest || method == str_up ||
          method == str_to_zero || method == str_down)
        {
          auto nr_number = dynamic_cast<Number*>(arg_1.ptr());
          auto nr_step = dynamic_cast<Number*>(arg_2.ptr());

          if (nr_number != nullptr && nr_step != nullptr) {

            if (nr_number->hasCompatibleUnits(nr_step)) {
              // std::cerr << "valid strategy " << method << "\n";
              return roundWithStep(method, nr_number, nr_step);
              // return SASS_MEMORY_NEW(String, node->pstate(), "TODO 3");
            }
            else {
              return new Calculation(node->pstate(),
                "round", { arg_0, arg_1, arg_2 });
            }

          }
          else if (dynamic_cast<String*>(arg_1.ptr())) {
            return new Calculation(node->pstate(),
              "round", { arg_0, arg_1, arg_2 });
          }
          else if (nr_step == nullptr) {
            return new Calculation(node->pstate(),
              "round", { arg_0, arg_1, arg_2 });
          }
          else if (/* String* rest = */ dynamic_cast<String*>(arg_2.ptr())) {
            return new Calculation(node->pstate(),
              "round", { arg_0, arg_1, arg_2 });
          }
          else if (nr_number == nullptr) {
            return new Calculation(node->pstate(),
              "round", { arg_0, arg_1, arg_2 });
          }
          else {
            throw Exception::SassScriptException(logger, node->pstate(),
              "If strategy is not null, step is required.");
          }
        }
        else if (strategy->isVar()) {
          return new Calculation(node->pstate(),
            "round", { arg_0, arg_1, arg_2 });
        }
        else if (String* rest = arguments[0]->isaString()) {
          return new Calculation(node->pstate(),
            "round", { rest, arg_1, arg_2 });
        }
        else {
          callStackFrame frame(logger, strategy->pstate());
          throw Exception::SassScriptException(method +
            " must be either nearest, up, down or to-zero.",
            logger, strategy->pstate());
        }
      }
      else if (arguments[0] != nullptr) {
        callStackFrame frame(logger, arguments[0]->pstate());
        throw Exception::SassScriptException(arguments[0]->toCss() +
          " must be either nearest, up, down or to-zero.",
          logger, arguments[0]->pstate());
      }
      else {
        // Shouldn't happen, but play safe
        throw Exception::MissingArgument(
          logger, str_number);
      }
    }
    else {
      throw Exception::TooManyArguments(
        logger, arguments.size(), 3);
    }

  }

  Value* Calculation32::calc_mod(Logger& logger, const FunctionExpression* pstate, AstNode* lhs, AstNode* rhs)
  {
    auto dividend = lhs->simplify(logger);
    auto modulus = rhs ? rhs->simplify(logger) : nullptr;
    // return singleArgument(logger, str_tan, argument, fnTan, true);
    if (auto dividend_nr = dynamic_cast<Number*>(dividend)) {
      if (auto modulus_nr = dynamic_cast<Number*>(modulus)) {
        // check compatible units
        return dividend_nr->modulo(modulus_nr, logger, lhs->pstate());
      }
    }
    return SASS_MEMORY_NEW(Calculation, lhs->pstate(), "mod", { dividend, modulus });
  }

  Value* Calculation32::calc_rem(Logger& logger, const FunctionExpression* pstate, AstNode* lhs, AstNode* rhs)
  {
    auto dividend = lhs->simplify(logger);
    auto modulus = rhs ? rhs->simplify(logger) : nullptr;
    // return singleArgument(logger, str_tan, argument, fnTan, true);
    if (auto dividend_nr = dynamic_cast<Number*>(dividend)) {
      if (auto modulus_nr = dynamic_cast<Number*>(modulus)) {
        // check compatible units
        return dividend_nr->remainder(modulus_nr, logger, lhs->pstate());
      }
    }
    return SASS_MEMORY_NEW(Calculation, lhs->pstate(), "mod", { dividend, modulus });
  }


  /*

  static Value round(Object strategyOrNumber,
      [Object? numberOrStep, Object? step]) {
    switch ((
      _simplify(strategyOrNumber),
      numberOrStep.andThen(_simplify),
      step.andThen(_simplify)
    )) {
      case (SassNumber number, null, null):
        return _matchUnits(number.value.round().toDouble(), number);

      case (SassNumber number, SassNumber step, null)
          when !number.hasCompatibleUnits(step):
        _verifyCompatibleNumbers([number, step]);
        return SassCalculation._("round", [number, step]);

      case (SassNumber number, SassNumber step, null):
        _verifyCompatibleNumbers([number, step]);
        return _roundWithStep('nearest', number, step);

      case (
            SassString(text: 'nearest' || 'up' || 'down' || 'to-zero') &&
                var strategy,
            SassNumber number,
            SassNumber step
          )
          when !number.hasCompatibleUnits(step):
        _verifyCompatibleNumbers([number, step]);
        return SassCalculation._("round", [strategy, number, step]);

      case (
          SassString(text: 'nearest' || 'up' || 'down' || 'to-zero') &&
              var strategy,
          SassNumber number,
          SassNumber step
        ):
        _verifyCompatibleNumbers([number, step]);
        return _roundWithStep(strategy.text, number, step);

      case (
          SassString(text: 'nearest' || 'up' || 'down' || 'to-zero') &&
              var strategy,
          SassString rest,
          null
        ):
        return SassCalculation._("round", [strategy, rest]);

      case (
          SassString(text: 'nearest' || 'up' || 'down' || 'to-zero'),
          _?,
          null
        ):
        throw SassScriptException("If strategy is not null, step is required.");

      case (
          SassString(text: 'nearest' || 'up' || 'down' || 'to-zero'),
          null,
          null
        ):
        throw SassScriptException(
            "Number to round and step arguments are required.");

      case (SassString rest, null, null):
        return SassCalculation._("round", [rest]);

      case (var number, null, null):
        throw SassScriptException(
            "Single argument $number expected to be simplifiable.");

      case (var number, var step?, null):
        return SassCalculation._("round", [number, step]);

      case (
          (SassString(text: 'nearest' || 'up' || 'down' || 'to-zero') ||
                  SassString(isVar: true)) &&
              var strategy,
          var number?,
          var step?
        ):
        return SassCalculation._("round", [strategy, number, step]);

      case (_, _?, _?):
        throw SassScriptException(
            "$strategyOrNumber must be either nearest, up, down or to-zero.");

      case (_, null, _?):
      // TODO(pamelalozano): Get rid of this case once dart-lang/sdk#52908 is solved.
      // ignore: unreachable_switch_case
      case (_, _, _):
        throw SassScriptException("Invalid parameters.");
    }
  }
*/

  /*

  // Returns [value] coerced to [number]'s units.
  static SassNumber _matchUnits(double value, SassNumber number) =>
      SassNumber.withUnits(value,
          numeratorUnits: number.numeratorUnits,
          denominatorUnits: number.denominatorUnits);

  /// Returns a rounded [number] based on a selected rounding [strategy],
  /// to the nearest integer multiple of [step].
  static SassNumber _roundWithStep(
      String strategy, SassNumber number, SassNumber step) {
    if (!{'nearest', 'up', 'down', 'to-zero'}.contains(strategy)) {
      throw ArgumentError(
          "$strategy must be either nearest, up, down or to-zero.");
    }

    if (number.value.isInfinite && step.value.isInfinite ||
        step.value == 0 ||
        number.value.isNaN ||
        step.value.isNaN) {
      return _matchUnits(double.nan, number);
    }
    if (number.value.isInfinite) return number;

    if (step.value.isInfinite) {
      return switch ((strategy, number.value)) {
        (_, 0) => number,
        ('nearest' || 'to-zero', > 0) => _matchUnits(0.0, number),
        ('nearest' || 'to-zero', _) => _matchUnits(-0.0, number),
        ('up', > 0) => _matchUnits(double.infinity, number),
        ('up', _) => _matchUnits(-0.0, number),
        ('down', < 0) => _matchUnits(-double.infinity, number),
        ('down', _) => _matchUnits(0, number),
        (_, _) => throw UnsupportedError("Invalid argument: $strategy.")
      };
    }

    var stepWithNumberUnit = step.convertValueToMatch(number);
    return switch (strategy) {
      'nearest' => _matchUnits(
          (number.value / stepWithNumberUnit).round() * stepWithNumberUnit,
          number),
      'up' => _matchUnits(
          (step.value < 0
                  ? (number.value / stepWithNumberUnit).floor()
                  : (number.value / stepWithNumberUnit).ceil()) *
              stepWithNumberUnit,
          number),
      'down' => _matchUnits(
          (step.value < 0
                  ? (number.value / stepWithNumberUnit).ceil()
                  : (number.value / stepWithNumberUnit).floor()) *
              stepWithNumberUnit,
          number),
      'to-zero' => number.value < 0
          ? _matchUnits(
              (number.value / stepWithNumberUnit).ceil() * stepWithNumberUnit,
              number)
          : _matchUnits(
              (number.value / stepWithNumberUnit).floor() * stepWithNumberUnit,
              number),
      _ => _matchUnits(double.nan, number)
    };
  }

  */

}
