#include "fn_colors.hpp"

#include <iomanip>
#include "compiler.hpp"
#include "exceptions.hpp"
#include "ast_values.hpp"
#include "util_string.hpp"

namespace Sass {

  namespace Functions {

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Import string utility functions
    using namespace StringUtils;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Create typedef for value function callback
    typedef Value* (*colFn)(
      const sass::string& name,
      const ValueVector& arguments,
      const SourceSpan& pstate,
      Logger& logger);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Returns whether [value] is an unquoted string
    // that start with `var(` and contains `/`.
    bool isVarSlash(Value* value)
    {
      if (value == nullptr) return false;
      const String* str = value->isaString();
      if (str == nullptr) return false;
      if (str->hasQuotes()) return false;
      return startsWith(str->value(), "var(") &&
        str->value().find('/') != NPOS;
    }
    // EO isVarSlash

    // Returns whether [value] is an unquoted
    // string that start with `var(`.
    bool isVar(const Value* value)
    {
      if (value == nullptr) return false;
      const String* str = value->isaString();
      if (str == nullptr) return false;
      if (str->hasQuotes()) return false;
      return startsWith(str->value(), "var(");
    }
    // EO isVar

    // Returns whether [value] is an unquoted
    // string that start either with `calc(`,
    // "var(", "env(", "min(" or "max(".
    bool isSpecialNumber(const Value* value)
    {
      if (value == nullptr) return false;
      const String* str = value->isaString();
      if (str == nullptr) return false;
      if (str->hasQuotes()) return false;
      if (str->value().size() < 6) return false;
      return startsWith(str->value(), "calc(", 5)
        || startsWith(str->value(), "var(", 4)
        || startsWith(str->value(), "env(", 4)
        || startsWith(str->value(), "min(", 4)
        || startsWith(str->value(), "max(", 4);
    }
    // EO isSpecialNumber

    // Implements regex check against /^[a-zA-Z]+\s*=/
    bool isMsFilterStart(const sass::string& text)
    {
      auto it = text.begin();
      // The filter must start with alpha
      if (!Character::isAlphabetic(*it)) return false;
      while (it != text.end() && Character::isAlphabetic(*it)) ++it;
      while (it != text.end() && Character::isWhitespace(*it)) ++it;
      return it != text.end() && *it == '=';
    }
    // EO isMsFilterStart

    // Helper function for debugging
    // ToDo return EnvKey?
    const sass::string& getColorArgName(
      size_t idx, const sass::string& name)
    {
      switch (idx) {
      case 0: return name[0] == Character::$h ? Strings::hue : Strings::red;
      case 1: return name[0] == Character::$h ? name[1] == Character::$s ? Strings::saturation : Strings::whiteness : Strings::green;
      case 2: return name[0] == Character::$h ? name[1] == Character::$s ? Strings::lightness : Strings::blackness : Strings::blue;
      default: throw std::runtime_error("Invalid input argument");
      }
    }
    // EO getColorArgName

    // Return value that will render as-is in css
    String* getFunctionString(
      const sass::string& name,
      const SourceSpan& pstate,
      const ValueVector& arguments = {},
      SassSeparator separator = SASS_COMMA)
    {
      bool addComma = false;
      sass::sstream fncall;
      fncall << name << "(";
      sass::string sep(" ");
      if (separator == SASS_COMMA) sep = ", ";
      for (Value* arg : arguments) {
        if (addComma) fncall << sep;
        fncall << arg->inspect();
        addComma = true;
      }
      fncall << ")";
      return SASS_MEMORY_NEW(String,
        pstate, fncall.str());
    }
    // EO getFunctionString

    Value* parseColorChannels(
      const sass::string& name,
      Value* channels,
      const SourceSpan& pstate,
      Compiler& compiler)
    {
      // Check for css var
      if (isVar(channels)) {
        return SASS_MEMORY_NEW(
          String, pstate, name + "(" +
          channels->inspect() + ")");
      }
      // Check if argument is already a list
      ListObj list = channels->isaList();
      // If not create one and wrap value in it
      if (!list) {
        list = SASS_MEMORY_NEW(List,
          pstate, { channels });
      }
      // Check for invalid input arguments
      bool isBracketed = list->hasBrackets();
      bool isCommaSeparated = list->hasCommaSeparator();
      if (isCommaSeparated || isBracketed) {
        sass::sstream msg;
        msg << "$channels must be";
        if (isBracketed) msg << " an unbracketed";
        if (isCommaSeparated) {
          msg << (isBracketed ? "," : " a");
          msg << " space-separated";
        }
        msg << " list.";
        compiler.addFinalStackTrace(list->pstate());
        throw Exception::RuntimeException(compiler, msg.str());
      }
      // Check if we have too many arguments
      if (list->size() > 3) {
        compiler.addFinalStackTrace(list->pstate());
        throw Exception::TooManyArguments(compiler, list->size(), 3);
      }
      // Check for not enough arguments
      if (list->size() < 3) {
        // Check if we have any css vars
        bool hasVar = false;
        for (Value* item : list->elements()) {
          if (isVar(item)) {
            hasVar = true;
            break;
          }
        }
        // Return function as-is back to be rendered as css
        if (hasVar || (!list->empty() && isVarSlash(list->last()))) {
          return getFunctionString(name, pstate, list->elements(), list->separator());
        }
        // Throw error for missing argument
        throw Exception::MissingArgument(compiler,
          getColorArgName(list->size(), name));
      }
      // Check for the second argument
      Number* secondNumber = list->get(2)->isaNumber();
      String* secondString = list->get(2)->isaString();
      if (secondNumber && secondNumber->hasAsSlash()) {
        return SASS_MEMORY_NEW(List, pstate, {
          list->get(0), list->get(1),
          secondNumber->lhsAsSlash(),
          secondNumber->rhsAsSlash()
        });
      }
      if (secondString && !secondString->hasQuotes()
        && secondString->value().find('/') != NPOS) {
        return getFunctionString(name, pstate,
          list->elements(), list->separator());
      }
      // Return arguments
      return list.detach();
    }
    // EO parseColorChannels

    // Handle one argument function invocation
    // Used by color functions rgb, hsl and hwb
    Value* handleOneArgColorFn(
      const sass::string& name,
      Value* argument,
      colFn function,
      Compiler& compiler,
      SourceSpan pstate)
    {
      // Parse the color channel arguments
      ValueObj parsed = parseColorChannels(
        name, argument, pstate, compiler);
      // Return if it is a string
      if (parsed->isaString()) {
        return parsed.detach();
      }
      // Execute function with list of arguments
      if (const List* list = parsed->isaList()) {
        return (*function)(name, list->elements(), pstate, compiler);
      }
      // Otherwise return
      return argument;
      // Not sure if we must stringify
      // return SASS_MEMORY_NEW(String,
      //   pstate, argument->inspect());
    }
    // EO handleOneArgColorFn

    /// Returns [color1] and [color2], mixed
    // together and weighted by [weight].
    ColorRgba* mixColors(
      const Color* color1,
      const Color* color2,
      const Number* weight,
      const SourceSpan& pstate,
      Logger& logger)
    {
      ColorRgbaObj lhs(color1->toRGBA());
      ColorRgbaObj rhs(color2->toRGBA());
      // This algorithm factors in both the user-provided weight (w) and the
      // difference between the alpha values of the two colors (a) to decide how
      // to perform the weighted average of the two RGB values.
      // It works by first normalizing both parameters to be within [-1, 1], where
      // 1 indicates "only use color1", -1 indicates "only use color2", and all
      // values in between indicated a proportionately weighted average.
      // Once we have the normalized variables w and a, we apply the formula
      // (w + a)/(1 + w*a) to get the combined weight (in [-1, 1]) of color1. This
      // formula has two especially nice properties:
      //   * When either w or a are -1 or 1, the combined weight is also that
      //     number (cases where w * a == -1 are undefined, and handled as a
      //     special case).
      //   * When a is 0, the combined weight is w, and vice versa.
      // Finally, the weight of color1 is renormalized to be within [0, 1] and the
      // weight of color2 is given by 1 minus the weight of color1.
      double weightScale = weight->assertRange(
        0.0, 100.0, logger, "weight") / 100.0;
      double normalizedWeight = weightScale * 2.0 - 1.0;
      double alphaDistance = lhs->a() - color2->a();
      double combinedWeight1 = normalizedWeight * alphaDistance == -1
        ? normalizedWeight : (normalizedWeight + alphaDistance) /
        (1.0 + normalizedWeight * alphaDistance);
      double weight1 = (combinedWeight1 + 1.0) / 2.0;
      double weight2 = 1.0 - weight1;
      return SASS_MEMORY_NEW(ColorRgba, pstate,
        fuzzyRound(lhs->r() * weight1 + rhs->r() * weight2, logger.epsilon),
        fuzzyRound(lhs->g() * weight1 + rhs->g() * weight2, logger.epsilon),
        fuzzyRound(lhs->b() * weight1 + rhs->b() * weight2, logger.epsilon),
        lhs->a() * weightScale + rhs->a() * (1 - weightScale));
    }
    // EO mixColor

    double scaleValue(
      double current,
      double scale,
      double max)
    {
      return current + (scale > 0.0 ? max - current : current) * scale;
    }

    // Asserts that [number] is a percentage or has no units, and normalizes the
    // value. If [number] has no units, its value is clamped to be greater than `0`
    // or less than [max] and returned. If [number] is a percentage, it's scaled to
    // be within `0` and [max]. Otherwise, this throws a [SassScriptException].
    // [name] is used to identify the argument in the error message.
    double _percentageOrUnitless(
      const Number* number, double max,
      const sass::string& name,
      Logger& traces)
    {
      double value = 0.0;
      if (!number->hasUnits()) {
        value = number->value();
      }
      else if (number->hasUnit(Strings::percent)) {
        value = max * number->value() / 100;
      }
      else {
        traces.addFinalStackTrace(number->pstate());
        throw Exception::RuntimeException(traces,
          name + ": Expected " + number->inspect()
          + " to have no units or \"%\".");
      }
      if (value < 0.0) return 0.0;
      if (value > max) return max;
      return value;
    }

    String* _functionRgbString(sass::string name, ColorRgba* color, Value* alpha, const SourceSpan& pstate)
    {
      sass::sstream fncall;
      fncall << name << "(";
      fncall << color->r() << ", ";
      fncall << color->g() << ", ";
      fncall << color->b() << ", ";
      fncall << alpha->inspect() << ")";
      return SASS_MEMORY_NEW(String,
        pstate, fncall.str());
    }

    Value* handleTwoArgRgb(sass::string name, ValueVector arguments, const SourceSpan& pstate, Logger& logger)
    {
      // Check if any `calc()` or `var()` are passed
      if (isVar(arguments[0])) {
        return getFunctionString(
          name, pstate, arguments);
      }
      else if (isVar(arguments[1])) {
        if (const Color* first = arguments[0]->isaColor()) {
          ColorRgbaObj rgba = first->toRGBA();
          return _functionRgbString(name,
            rgba, arguments[1], pstate);
        }
        else {
          return getFunctionString(
            name, pstate, arguments);
        }
      }
      else if (isSpecialNumber(arguments[1])) {
        if (const Color* color = arguments[0]->assertColor(logger, Strings::color)) {
          ColorRgbaObj rgba = color->toRGBA();
          return _functionRgbString(name,
            rgba, arguments[1], pstate);
        }
      }

      const Color* color = arguments[0]->assertColor(logger, Strings::color);
      const Number* alpha = arguments[1]->assertNumber(logger, Strings::alpha);
      ColorObj copy = SASS_MEMORY_COPY(color);
      copy->a(_percentageOrUnitless(
        alpha, 1.0, "$alpha", logger));
      return copy.detach();
    }

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    namespace Colors {

      /*******************************************************************/

      BUILT_IN_FN(rgb4arg)
      {
        return rgbFn(Strings::rgb,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(rgb3arg)
      {
        return rgbFn(Strings::rgb,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(rgb2arg)
      {
        return handleTwoArgRgb(Strings::rgb,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(rgb1arg)
      {
        return handleOneArgColorFn(Strings::rgb,
          arguments[0], &rgbFn, compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(rgba4arg)
      {
        return rgbFn(Strings::rgba,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(rgba3arg)
      {
        return rgbFn(Strings::rgba,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(rgba2arg)
      {
        return handleTwoArgRgb(Strings::rgba,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(rgba1arg)
      {
        return handleOneArgColorFn(Strings::rgba,
          arguments[0], &rgbFn, compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(hsl4arg)
      {
        return hslFn(Strings::hsl,
          arguments, pstate, compiler);
      }


      BUILT_IN_FN(hsl3arg)
      {
        return hslFn(Strings::hsl,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(hsl2arg)
      {
        // hsl(123, var(--foo)) is valid CSS because --foo might be `10%, 20%`
        // and functions are parsed after variable substitution.
        if (isVar(arguments[0]) || isVar(arguments[1])) {
          return getFunctionString(Strings::hsl, pstate, arguments);
        }
        // Otherwise throw error for missing argument
        throw Exception::MissingArgument(compiler, Keys::lightness);
      }

      BUILT_IN_FN(hsl1arg)
      {
        return handleOneArgColorFn(Strings::hsl,
          arguments[0], &hslFn, compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(hsla4arg)
      {
        return hslFn(Strings::hsla, arguments, pstate, compiler);
      }

      BUILT_IN_FN(hsla3arg)
      {
        return hslFn(Strings::hsla, arguments, pstate, compiler);
      }

      BUILT_IN_FN(hsla2arg)
      {
        // hsl(123, var(--foo)) is valid CSS because --foo might be `10%, 20%`
        // and functions are parsed after variable substitution.
        if (isVar(arguments[0]) || isVar(arguments[1])) {
          return getFunctionString(Strings::hsla, pstate, arguments);
        }
        // Otherwise throw error for missing argument
        throw Exception::MissingArgument(compiler, Keys::lightness);
      }

      BUILT_IN_FN(hsla1arg)
      {
        return handleOneArgColorFn(Strings::hsla,
          arguments[0], &hslFn, compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(hwb4arg)
      {
        return hwbFn(Strings::hwb,
          arguments, pstate, compiler);
      }


      BUILT_IN_FN(hwb3arg)
      {
        return hwbFn(Strings::hwb,
          arguments, pstate, compiler);
      }

      BUILT_IN_FN(hwb2arg)
      {
        // hwb(123, var(--foo)) is valid CSS because --foo might be `10%, 20%`
        // and functions are parsed after variable substitution.
        if (isVar(arguments[0]) || isVar(arguments[1])) {
          return getFunctionString(Strings::hwb, pstate, arguments);
        }
        // Otherwise throw error for missing argument
        throw Exception::MissingArgument(compiler, Keys::lightness);
      }

      BUILT_IN_FN(hwb1arg)
      {
        return handleOneArgColorFn(Strings::hwb,
          arguments[0], &hwbFn, compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(hwba4arg)
      {
        return hwbFn(Strings::hwba, arguments, pstate, compiler);
      }

      BUILT_IN_FN(hwba3arg)
      {
        return hwbFn(Strings::hwba, arguments, pstate, compiler);
      }

      BUILT_IN_FN(hwba2arg)
      {
        // hwb(123, var(--foo)) is valid CSS because --foo might be `10%, 20%`
        // and functions are parsed after variable substitution.
        if (isVar(arguments[0]) || isVar(arguments[1])) {
          return getFunctionString(Strings::hwba, pstate, arguments);
        }
        // Otherwise throw error for missing argument
        throw Exception::MissingArgument(compiler, Keys::lightness);
      }

      BUILT_IN_FN(hwba1arg)
      {
        return handleOneArgColorFn(Strings::hwba,
          arguments[0], &hwbFn, compiler, pstate);
      }

      /*******************************************************************/

      BUILT_IN_FN(red)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorRgbaObj rgba(color->toRGBA()); // This might create a copy
        return SASS_MEMORY_NEW(Number, pstate, Sass::round32(rgba->r()));
      }

      BUILT_IN_FN(green)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorRgbaObj rgba(color->toRGBA()); // This might create a copy
        return SASS_MEMORY_NEW(Number, pstate, Sass::round32(rgba->g()));
      }

      BUILT_IN_FN(blue)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorRgbaObj rgba(color->toRGBA()); // This might create a copy
        return SASS_MEMORY_NEW(Number, pstate, Sass::round32(rgba->b()));
      }

      /*******************************************************************/

      BUILT_IN_FN(invert)
      {
        if (arguments[0]->isaNumber()) {
          // Allow only the value `100` or a percentage (unit == `% `)
          const Number* weight = arguments[1]->assertNumber(compiler, Strings::weight);
          if (weight->value() != 100 || !weight->hasUnit(Strings::percent)) {
            throw Exception::RuntimeException(compiler,
              "Only one argument may be passed "
              "to the plain-CSS invert() function.");
          }
          // Return function string since first argument was a number
          // Need to remove the weight argument as it has a default value
          return getFunctionString(Strings::invert, pstate, { arguments[0] });
        }
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* weight = arguments[1]->assertNumber(compiler, Strings::weight);
        ColorRgbaObj inverse(color->copyAsRGBA()); // Make a copy!
        inverse->r(clamp(255.0 - inverse->r(), 0.0, 255.0));
        inverse->g(clamp(255.0 - inverse->g(), 0.0, 255.0));
        inverse->b(clamp(255.0 - inverse->b(), 0.0, 255.0));
        // Note: mixColors will create another unnecessary copy!
        return mixColors(inverse, color, weight, pstate, compiler);
      }

      /*******************************************************************/

      BUILT_IN_FN(hue)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorHslaObj hsla(color->toHSLA()); // This probably creates a copy
        return SASS_MEMORY_NEW(Number, pstate, hsla->h(), Strings::deg);
      }

      BUILT_IN_FN(saturation)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorHslaObj hsla(color->toHSLA()); // This probably creates a copy
        return SASS_MEMORY_NEW(Number, pstate, hsla->s(), Strings::percent);
      }

      BUILT_IN_FN(lightness)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorHslaObj hsla(color->toHSLA()); // This probably creates a copy
        return SASS_MEMORY_NEW(Number, pstate, hsla->l(), Strings::percent);
      }

      BUILT_IN_FN(whiteness)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::whiteness);
        ColorHwbaObj hwba(color->toHWBA()); // This probably creates a copy
        return SASS_MEMORY_NEW(Number, pstate, hwba->w(), Strings::percent);
      }

      BUILT_IN_FN(blackness)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::blackness);
        ColorHwbaObj hwba(color->toHWBA()); // This probably creates a copy
        return SASS_MEMORY_NEW(Number, pstate, hwba->b(), Strings::percent);
      }

      /*******************************************************************/

      BUILT_IN_FN(adjustHue)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* degrees = arguments[1]->assertNumber(compiler, Strings::degrees);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->h(absmod(copy->h() + degrees->value(), 360.0));
        return copy.detach();
      }

      BUILT_IN_FN(complement)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->h(absmod(copy->h() + 180.0, 360.0));
        return copy.detach();
      }

      /*******************************************************************/

      BUILT_IN_FN(grayscale)
      {
        // Gracefully handle if number is passed
        if (arguments[0]->isaNumber()) {
          return getFunctionString(
            Strings::grayscale,
            pstate, arguments);
        }
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->s(0.0); // Simply reset the saturation
        return copy.detach(); // Return HSLA
      }

      BUILT_IN_FN(lighten)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* amount = arguments[1]->assertNumber(compiler, Strings::amount);
        double nr = amount->assertRange(0.0, 100.0, compiler, Strings::amount);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->l(clamp(copy->l() + nr, 0.0, 100.0));
        return copy.detach(); // Return HSLA
      }

      BUILT_IN_FN(darken)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* amount = arguments[1]->assertNumber(compiler, Strings::amount);
        double nr = amount->assertRange(0.0, 100.0, compiler, Strings::amount);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->l(clamp(copy->l() - nr, 0.0, 100.0));
        return copy.detach(); // Return HSLA
      }

      /*******************************************************************/

      BUILT_IN_FN(saturate2arg)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* amount = arguments[1]->assertNumber(compiler, Strings::amount);
        double nr = amount->assertRange(0.0, 100.0, compiler, Strings::amount);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        if (copy->h() == 0 && nr > 0.0) copy->h(100.0);
        copy->s(clamp(copy->s() + nr, 0.0, 100.0));
        return copy.detach(); // Return HSLA
      }

      BUILT_IN_FN(saturate1arg)
      {
        arguments[0]->assertNumber(compiler, Strings::amount);
        return getFunctionString(Strings::saturate, pstate, { arguments[0] });
      }

      BUILT_IN_FN(desaturate)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* amount = arguments[1]->assertNumber(compiler, Strings::amount);
        double nr = amount->assertRange(0.0, 100.0, compiler, Strings::amount);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->s(clamp(copy->s() - nr, 0.0, 100.0));
        return copy.detach(); // Return HSLA
      }

      /*******************************************************************/

      BUILT_IN_FN(opacify)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* amount = arguments[1]->assertNumber(compiler, Strings::amount);
        double nr = amount->assertRange(0.0, 1.0, compiler, Strings::amount);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->a(clamp(copy->a() + nr, 0.0, 1.0));
        return copy.detach(); // Return HSLA
      }

      BUILT_IN_FN(transparentize)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        const Number* amount = arguments[1]->assertNumber(compiler, Strings::amount);
        double nr = amount->assertRange(0.0, 1.0, compiler, Strings::amount);
        ColorHslaObj copy(color->copyAsHSLA()); // Must make a copy!
        copy->a(clamp(copy->a() - nr, 0.0, 1.0));
        return copy.detach(); // Return HSLA
      }

      /*******************************************************************/

      BUILT_IN_FN(alphaOne)
      {
        if (String * string = arguments[0]->isaString()) {
          if (!string->hasQuotes() && isMsFilterStart(string->value())) {
            return getFunctionString(Strings::alpha, pstate, arguments);
          }
        }
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        return SASS_MEMORY_NEW(Number, pstate, color->a());
      }

      BUILT_IN_FN(alphaAny)
      {
        size_t size = arguments[0]->lengthAsList();

        if (size == 0) {
          throw Exception::MissingArgument(compiler, Keys::color);
        }

        bool isOnlyIeFilters = true;
        for (Value* value : arguments[0]->iterator()) {
          if (String* string = value->isaString()) {
            if (!isMsFilterStart(string->value())) {
              isOnlyIeFilters = false;
              break;
            }
          }
          else {
            isOnlyIeFilters = false;
            break;
          }
        }
        if (isOnlyIeFilters) {
          // Support the proprietary Microsoft alpha() function.
          return getFunctionString(Strings::alpha, pstate, arguments);
        }
        compiler.addFinalStackTrace(arguments[0]->pstate());
        throw Exception::TooManyArguments(compiler, size, 1);
      }


      BUILT_IN_FN(opacity)
      {
        // Gracefully handle if number is passed
        if (arguments[0]->isaNumber()) {
          return getFunctionString("opacity",
            pstate, arguments);
        }
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        return SASS_MEMORY_NEW(Number, pstate, color->a());
      }


      BUILT_IN_FN(ieHexStr)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ColorRgbaObj rgba = color->toRGBA(); // This might create a copy
        // clamp should not be needed here
        double r = clamp(rgba->r(), 0.0, 255.0);
        double g = clamp(rgba->g(), 0.0, 255.0);
        double b = clamp(rgba->b(), 0.0, 255.0);
        double a = clamp(rgba->a(), 0.0, 1.0) * 255.0;
        sass::sstream ss;
        ss << '#' << std::setw(2) << std::setfill('0') << std::uppercase;
        ss << std::hex << std::setw(2) << fuzzyRound(a, compiler.epsilon);
        ss << std::hex << std::setw(2) << fuzzyRound(r, compiler.epsilon);
        ss << std::hex << std::setw(2) << fuzzyRound(g, compiler.epsilon);
        ss << std::hex << std::setw(2) << fuzzyRound(b, compiler.epsilon);
        return SASS_MEMORY_NEW(String, pstate, ss.str());
      }

      Number* getKwdArg(ValueFlatMap& keywords, const EnvKey& name, Logger& logger)
      {
        EnvKey variable(name.norm());
        auto kv = keywords.find(variable);
        // Return null since args are optional
        if (kv == keywords.end()) return nullptr;
        // Get the number object from found keyword
        Number* num = kv->second->assertNumber(logger, name.orig());
        // Only consume keyword once
        keywords.erase(kv);
        // Return the number
        return num;
      }

      BUILT_IN_FN(adjust)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ArgumentList* argumentList = arguments[1]
          ->assertArgumentList(compiler, "kwargs");
        if (!argumentList->empty()) {
          SourceSpan span(color->pstate());
          callStackFrame frame(compiler, BackTrace(
            span, Strings::colorAdjust));
          throw Exception::RuntimeException(compiler,
            "Only one positional argument is allowed. All "
            "other arguments must be passed by name.");
        }

        // ToDo: solve without erase ...
        ValueFlatMap& keywords(argumentList->keywords());

        Number* nr_r = getKwdArg(keywords, Keys::red, compiler);
        Number* nr_g = getKwdArg(keywords, Keys::green, compiler);
        Number* nr_b = getKwdArg(keywords, Keys::blue, compiler);
        Number* nr_h = getKwdArg(keywords, Keys::hue, compiler);
        Number* nr_s = getKwdArg(keywords, Keys::saturation, compiler);
        Number* nr_l = getKwdArg(keywords, Keys::lightness, compiler);
        Number* nr_a = getKwdArg(keywords, Keys::alpha, compiler);
        Number* nr_wn = getKwdArg(keywords, Keys::whiteness, compiler);
        Number* nr_bn = getKwdArg(keywords, Keys::blackness, compiler);

        double r = nr_r ? nr_r->assertRange(-255.0, 255.0, compiler, Strings::red) : 0.0;
        double g = nr_g ? nr_g->assertRange(-255.0, 255.0, compiler, Strings::green) : 0.0;
        double b = nr_b ? nr_b->assertRange(-255.0, 255.0, compiler, Strings::blue) : 0.0;
        double s = nr_s ? nr_s->assertRange(-100.0, 100.0, compiler, Strings::saturation) : 0.0;
        double l = nr_l ? nr_l->assertRange(-100.0, 100.0, compiler, Strings::lightness) : 0.0;

        double wn = nr_wn ? nr_wn->assertHasUnits(compiler, Strings::percent, Strings::whiteness)->assertRange(-100.0, 100.0, compiler, Strings::whiteness) : 0.0;
        double bn = nr_bn ? nr_bn->assertHasUnits(compiler, Strings::percent, Strings::blackness)->assertRange(-100.0, 100.0, compiler, Strings::blackness) : 0.0;

        double a = nr_a ? nr_a->assertRange(-1.0, 1.0, compiler, Strings::alpha) : 0.0;

        double h = nr_h ? nr_h->value() : 0.0; // Hue is a very special case

        if (!keywords.empty()) {
          throw Exception::UnknownNamedArgument(compiler, keywords);
        }

        bool hasRgb = nr_r || nr_g || nr_b;
        bool hasHsl = nr_s || nr_l;
        bool hasHwb = nr_wn || nr_bn;
        bool hasHue = nr_h;

        if (hasRgb && hasHsl && hasHwb) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL", "HWB" });
        else if (hasRgb && hasHue) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL/HWB" });
        else if (hasRgb && hasHsl) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL" });
        else if (hasRgb && hasHwb) throw Exception::MixedParamGroups(compiler, "RGB", { "HWB" });
        else if (hasHsl && hasHwb) throw Exception::MixedParamGroups(compiler, "HSL", { "HWB" });
        else if (hasHwb && hasHsl) throw Exception::MixedParamGroups(compiler, "HSL", { "HWB" });

        if (hasRgb) {
          ColorRgbaObj rgba = color->copyAsRGBA();
          if (nr_r) rgba->r(clamp(rgba->r() + r, 0.0, 255.0));
          if (nr_g) rgba->g(clamp(rgba->g() + g, 0.0, 255.0));
          if (nr_b) rgba->b(clamp(rgba->b() + b, 0.0, 255.0));
          if (nr_a) rgba->a(clamp(rgba->a() + a, 0.0, 1.0));
          return rgba.detach();
        }
        else if (hasHsl) {
          ColorHslaObj hsla = color->copyAsHSLA();
          if (nr_h) hsla->h(absmod(hsla->h() + h, 360.0));
          if (nr_s) hsla->s(clamp(hsla->s() + s, 0.0, 100.0));
          if (nr_l) hsla->l(clamp(hsla->l() + l, 0.0, 100.0));
          if (nr_a) hsla->a(clamp(hsla->a() + a, 0.0, 1.0));
          return hsla.detach();
        } else if (hasHwb || nr_h) { // hue can be shared!
          ColorHwbaObj hwba = color->copyAsHWBA();
          if (nr_h) hwba->h(absmod(hwba->h() + h, 360.0));
          if (nr_wn) hwba->w(clamp(hwba->w() + wn, 0.0, 100.0));
          if (nr_bn) hwba->b(clamp(hwba->b() + bn, 0.0, 100.0));
          if (nr_a) hwba->a(clamp(hwba->a() + a, 0.0, 1.0));
          return hwba.detach();
        }
        else if (nr_a) {
          ColorObj copy = SASS_MEMORY_COPY(color);
          if (nr_a) copy->a(clamp(copy->a() + a, 0.0, 1.0));
          return copy.detach();
        }
        return arguments[0];
      }

      BUILT_IN_FN(change)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ArgumentList* argumentList = arguments[1]
          ->assertArgumentList(compiler, "kwargs");
        if (!argumentList->empty()) {
          SourceSpan span(color->pstate());
          callStackFrame frame(compiler, BackTrace(
            span, Strings::colorChange));
          throw Exception::RuntimeException(compiler,
            "Only one positional argument is allowed. All "
            "other arguments must be passed by name.");
        }

        // ToDo: solve without erase ...
        ValueFlatMap& keywords(argumentList->keywords());

        Number* nr_r = getKwdArg(keywords, Keys::red, compiler);
        Number* nr_g = getKwdArg(keywords, Keys::green, compiler);
        Number* nr_b = getKwdArg(keywords, Keys::blue, compiler);
        Number* nr_h = getKwdArg(keywords, Keys::hue, compiler);
        Number* nr_s = getKwdArg(keywords, Keys::saturation, compiler);
        Number* nr_l = getKwdArg(keywords, Keys::lightness, compiler);
        Number* nr_a = getKwdArg(keywords, Keys::alpha, compiler);
        Number* nr_wn = getKwdArg(keywords, Keys::whiteness, compiler);
        Number* nr_bn = getKwdArg(keywords, Keys::blackness, compiler);

        double r = nr_r ? nr_r->assertRange(0.0, 255.0, compiler, Strings::red) : 0.0;
        double g = nr_g ? nr_g->assertRange(0.0, 255.0, compiler, Strings::green) : 0.0;
        double b = nr_b ? nr_b->assertRange(0.0, 255.0, compiler, Strings::blue) : 0.0;
        double s = nr_s ? nr_s->assertRange(0.0, 100.0, compiler, Strings::saturation) : 0.0;
        double l = nr_l ? nr_l->assertRange(0.0, 100.0, compiler, Strings::lightness) : 0.0;
        double a = nr_a ? nr_a->assertRange(0.0, 1.0, compiler, Strings::alpha) : 0.0;
        double wn = nr_wn ? nr_wn->assertHasUnits(compiler, Strings::percent, Strings::whiteness)->assertRange(0.0, 100.0, compiler, Strings::whiteness) : 0.0;
        double bn = nr_bn ? nr_bn->assertHasUnits(compiler, Strings::percent, Strings::blackness)->assertRange( 0.0, 100.0, compiler, Strings::blackness) : 0.0;
        double h = nr_h ? nr_h->value() : 0.0; // Hue is a very special case

        if (!keywords.empty()) {
          throw Exception::UnknownNamedArgument(compiler, keywords);
        }

        bool hasRgb = nr_r || nr_g || nr_b;
        bool hasHsl = nr_s || nr_l;
        bool hasHwb = nr_wn || nr_bn;
        bool hasHue = nr_h;

        if (hasRgb && hasHsl && hasHwb) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL", "HWB" });
        else if (hasRgb && hasHue) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL/HWB" });
        else if (hasRgb && hasHsl) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL" });
        else if (hasRgb && hasHwb) throw Exception::MixedParamGroups(compiler, "RGB", { "HWB" });
        else if (hasHsl && hasHwb) throw Exception::MixedParamGroups(compiler, "HSL", { "HWB" });
        else if (hasHwb && hasHsl) throw Exception::MixedParamGroups(compiler, "HSL", { "HWB" });

        if (hasRgb) {
          ColorRgbaObj rgba = color->copyAsRGBA();
          if (nr_r) rgba->r(clamp(r, 0.0, 255.0));
          if (nr_g) rgba->g(clamp(g, 0.0, 255.0));
          if (nr_b) rgba->b(clamp(b, 0.0, 255.0));
          if (nr_a) rgba->a(clamp(a, 0.0, 1.0));
          return rgba.detach();
        }
        else if (hasHsl) {
          ColorHslaObj hsla = color->copyAsHSLA();
          if (nr_h) hsla->h(absmod(h, 360.0));
          if (nr_s) hsla->s(clamp(s, 0.0, 100.0));
          if (nr_l) hsla->l(clamp(l, 0.0, 100.0));
          if (nr_a) hsla->a(clamp(a, 0.0, 1.0));
          return hsla.detach();
        }
        else if (hasHwb || nr_h) { // hue can be shared!
          ColorHwbaObj hwba = color->copyAsHWBA();
          if (nr_h) hwba->h(absmod(h, 360.0));
          if (nr_wn) hwba->w(clamp(wn, 0.0, 100.0));
          if (nr_bn) hwba->b(clamp(bn, 0.0, 100.0));
          if (nr_a) hwba->a(clamp(a, 0.0, 1.0));
          return hwba.detach();
        }
        else if (nr_a) {
          ColorObj copy = SASS_MEMORY_COPY(color);
          if (nr_a) copy->a(clamp(a, 0.0, 1.0));
          return copy.detach();
        }
        return arguments[0];
      }

      BUILT_IN_FN(scale)
      {
        const Color* color = arguments[0]->assertColor(compiler, Strings::color);
        ArgumentList* argumentList = arguments[1]
          ->assertArgumentList(compiler, "kwargs");
        if (!argumentList->empty()) {
          SourceSpan span(color->pstate());
          callStackFrame frame(compiler, BackTrace(
            span, Strings::scaleColor));
          throw Exception::RuntimeException(compiler,
            "Only one positional argument is allowed. All "
            "other arguments must be passed by name.");
        }

        // ToDo: solve without erase ...
        ValueFlatMap& keywords(argumentList->keywords());

        Number* nr_r = getKwdArg(keywords, Keys::red, compiler);
        Number* nr_g = getKwdArg(keywords, Keys::green, compiler);
        Number* nr_b = getKwdArg(keywords, Keys::blue, compiler);
        Number* nr_s = getKwdArg(keywords, Keys::saturation, compiler);
        Number* nr_l = getKwdArg(keywords, Keys::lightness, compiler);
        Number* nr_wn = getKwdArg(keywords, Keys::whiteness, compiler);
        Number* nr_bn = getKwdArg(keywords, Keys::blackness, compiler);
        Number* nr_a = getKwdArg(keywords, Keys::alpha, compiler);

        double r = nr_r ? nr_r->assertHasUnits(compiler, Strings::percent, Strings::red)->assertRange(-100.0, 100.0, compiler, Strings::red) / 100.0 : 0.0;
        double g = nr_g ? nr_g->assertHasUnits(compiler, Strings::percent, Strings::green)->assertRange(-100.0, 100.0, compiler, Strings::green) / 100.0 : 0.0;
        double b = nr_b ? nr_b->assertHasUnits(compiler, Strings::percent, Strings::blue)->assertRange(-100.0, 100.0, compiler, Strings::blue) / 100.0 : 0.0;
        double s = nr_s ? nr_s->assertHasUnits(compiler, Strings::percent, Strings::saturation)->assertRange(-100.0, 100.0, compiler, Strings::saturation) / 100.0 : 0.0;
        double l = nr_l ? nr_l->assertHasUnits(compiler, Strings::percent, Strings::lightness)->assertRange(-100.0, 100.0, compiler, Strings::lightness) / 100.0 : 0.0;
        double wn = nr_wn ? nr_wn->assertHasUnits(compiler, Strings::percent, Strings::whiteness)->assertRange(-100.0, 100.0, compiler, Strings::whiteness) / 100.0 : 0.0;
        double bn = nr_bn ? nr_bn->assertHasUnits(compiler, Strings::percent, Strings::blackness)->assertRange(-100.0, 100.0, compiler, Strings::blackness) / 100.0 : 0.0;
        double a = nr_a ? nr_a->assertHasUnits(compiler, Strings::percent, Strings::alpha)->assertRange(-100.0, 100.0, compiler, Strings::alpha) / 100.0 : 0.0;

        if (!keywords.empty()) {
          throw Exception::UnknownNamedArgument(compiler, keywords);
        }

        bool hasRgb = nr_r || nr_g || nr_b;
        bool hasHsl = nr_s || nr_l;
        bool hasHwb = nr_wn || nr_bn;

        if (hasRgb && hasHsl && hasHwb) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL", "HWB" });
        else if (hasRgb && hasHsl) throw Exception::MixedParamGroups(compiler, "RGB", { "HSL" });
        else if (hasRgb && hasHwb) throw Exception::MixedParamGroups(compiler, "RGB", { "HWB" });
        else if (hasHsl && hasHwb) throw Exception::MixedParamGroups(compiler, "HSL", { "HWB" });
        else if (hasHwb && hasHsl) throw Exception::MixedParamGroups(compiler, "HSL", { "HWB" });

        if (hasRgb) {
          ColorRgbaObj rgba = color->copyAsRGBA();
          if (nr_r) rgba->r(scaleValue(rgba->r(), r, 255.0));
          if (nr_g) rgba->g(scaleValue(rgba->g(), g, 255.0));
          if (nr_b) rgba->b(scaleValue(rgba->b(), b, 255.0));
          if (nr_a) rgba->a(scaleValue(rgba->a(), a, 1.0));
          return rgba.detach();
        }
        else if (hasHsl) {
          ColorHslaObj hsla = color->copyAsHSLA();
          if (nr_s) hsla->s(scaleValue(hsla->s(), s, 100.0));
          if (nr_l) hsla->l(scaleValue(hsla->l(), l, 100.0));
          if (nr_a) hsla->a(scaleValue(hsla->a(), a, 1.0));
          return hsla.detach();
        }
        else if (hasHwb) { // hue can be shared!
          ColorHwbaObj hwba = color->copyAsHWBA();
          if (nr_wn) hwba->w(scaleValue(hwba->w(), wn, 100.0));
          if (nr_bn) hwba->b(scaleValue(hwba->b(), bn, 100.0));
          if (nr_a) hwba->a(scaleValue(hwba->a(), a, 1.0));
          return hwba.detach();
        }
        else if (nr_a) {
          ColorObj copy = SASS_MEMORY_COPY(color);
          if (nr_a) copy->a(scaleValue(copy->a(), a, 1.0));
          return copy.detach();
        }
        return arguments[0];
      }

      BUILT_IN_FN(mix)
      {
        const Color* color1 = arguments[0]->assertColor(compiler, "color1");
        const Color* color2 = arguments[1]->assertColor(compiler, "color2");
        const Number* weight = arguments[2]->assertNumber(compiler, "weight");
        return mixColors(color1, color2, weight, pstate, compiler);
      }

      void registerFunctions(Compiler& ctx)
      {

        ctx.registerBuiltInOverloadFns("rgb", {
          std::make_pair("$red, $green, $blue, $alpha", rgb4arg),
          std::make_pair("$red, $green, $blue", rgb3arg),
          std::make_pair("$color, $alpha", rgb2arg),
          std::make_pair("$channels", rgb1arg),
        });
        ctx.registerBuiltInOverloadFns("rgba", {
          std::make_pair("$red, $green, $blue, $alpha", rgba4arg),
          std::make_pair("$red, $green, $blue", rgba3arg),
          std::make_pair("$color, $alpha", rgba2arg),
          std::make_pair("$channels", rgba1arg),
        });
        ctx.registerBuiltInOverloadFns("hsl", {
          std::make_pair("$hue, $saturation, $lightness, $alpha", hsl4arg),
          std::make_pair("$hue, $saturation, $lightness", hsl3arg),
          std::make_pair("$color, $alpha", hsl2arg),
          std::make_pair("$channels", hsl1arg),
        });
        ctx.registerBuiltInOverloadFns("hsla", {
          std::make_pair("$hue, $saturation, $lightness, $alpha", hsla4arg),
          std::make_pair("$hue, $saturation, $lightness", hsla3arg),
          std::make_pair("$color, $alpha", hsla2arg),
          std::make_pair("$channels", hsla1arg),
        });
        ctx.registerBuiltInOverloadFns("hwb", {
          std::make_pair("$hue, $whiteness, $blackness, $alpha", hwb4arg),
          std::make_pair("$hue, $whiteness, $blackness", hwb3arg),
          std::make_pair("$color, $alpha", hwb2arg),
          std::make_pair("$channels", hwb1arg),
        });
        ctx.registerBuiltInOverloadFns("hwba", {
          std::make_pair("$hue, $whiteness, $blackness, $alpha", hwba4arg),
          std::make_pair("$hue, $whiteness, $blackness", hwba3arg),
          std::make_pair("$color, $alpha", hwba2arg),
          std::make_pair("$channels", hwba1arg),
        });

        ctx.registerBuiltInFunction("red", "$color", red);
        ctx.registerBuiltInFunction("green", "$color", green);
        ctx.registerBuiltInFunction("blue", "$color", blue);
        ctx.registerBuiltInFunction("hue", "$color", hue);
        ctx.registerBuiltInFunction("lightness", "$color", lightness);
        ctx.registerBuiltInFunction("saturation", "$color", saturation);
        //ctx.registerBuiltInFunction("blackness", "$color", blackness);
        //ctx.registerBuiltInFunction("whiteness", "$color", whiteness);
        ctx.registerBuiltInFunction("invert", "$color, $weight: 100%", invert);
        ctx.registerBuiltInFunction("grayscale", "$color", grayscale);
        ctx.registerBuiltInFunction("complement", "$color", complement);
        ctx.registerBuiltInFunction("lighten", "$color, $amount", lighten);
        ctx.registerBuiltInFunction("darken", "$color, $amount", darken);
        ctx.registerBuiltInFunction("desaturate", "$color, $amount", desaturate);
        ctx.registerBuiltInOverloadFns("saturate", {
          std::make_pair("$amount", saturate1arg),
          std::make_pair("$color, $amount", saturate2arg),
          });

        ctx.registerBuiltInFunction("adjust-hue", "$color, $degrees", adjustHue);
        ctx.registerBuiltInFunction("adjust-color", "$color, $kwargs...", adjust);
        ctx.registerBuiltInFunction("change-color", "$color, $kwargs...", change);
        ctx.registerBuiltInFunction("scale-color", "$color, $kwargs...", scale);
        ctx.registerBuiltInFunction("mix", "$color1, $color2, $weight: 50%", mix);

        ctx.registerBuiltInFunction("opacify", "$color, $amount", opacify);
        ctx.registerBuiltInFunction("fade-in", "$color, $amount", opacify);
        ctx.registerBuiltInFunction("fade-out", "$color, $amount", transparentize);
        ctx.registerBuiltInFunction("transparentize", "$color, $amount", transparentize);
        ctx.registerBuiltInFunction("ie-hex-str", "$color", ieHexStr);
        ctx.registerBuiltInOverloadFns("alpha", {
          std::make_pair("$color", alphaOne),
          std::make_pair("$args...", alphaAny),
          });
        ctx.registerBuiltInFunction("opacity", "$color", opacity);

      }

    }

    Value* rgbFn(const sass::string& name, const ValueVector& arguments, const SourceSpan& pstate, Logger& logger)
    {
      Value* _r = arguments[0];
      Value* _g = arguments[1];
      Value* _b = arguments[2];
      Value* _a = nullptr;
      if (arguments.size() > 3) {
        _a = arguments[3];
      }
      // Check if any `calc()` or `var()` are passed
      if (isSpecialNumber(_r) || isSpecialNumber(_g) || isSpecialNumber(_b) || isSpecialNumber(_a)) {
        sass::sstream fncall;
        fncall << name << "(";
        fncall << _r->inspect() << ", ";
        fncall << _g->inspect() << ", ";
        fncall << _b->inspect();
        if (_a) { fncall << ", " << _a->inspect(); }
        fncall << ")";
        return SASS_MEMORY_NEW(String, pstate, fncall.str());
      }

      Number* r = _r->assertNumber(logger, Strings::red);
      Number* g = _g->assertNumber(logger, Strings::green);
      Number* b = _b->assertNumber(logger, Strings::blue);
      Number* a = _a ? _a->assertNumber(logger, Strings::alpha) : nullptr;

      return SASS_MEMORY_NEW(ColorRgba, pstate,
        fuzzyRound(_percentageOrUnitless(r, 255, "$red", logger), logger.epsilon),
        fuzzyRound(_percentageOrUnitless(g, 255, "$green", logger), logger.epsilon),
        fuzzyRound(_percentageOrUnitless(b, 255, "$blue", logger), logger.epsilon),
        _a ? _percentageOrUnitless(a, 1.0, "$alpha", logger) : 1.0);

    }

    Value* hwbFn(const sass::string& name, const ValueVector& arguments, const SourceSpan& pstate, Logger& logger)
    {
      Value* _h = arguments[0];
      Value* _w = arguments[1];
      Value* _b = arguments[2];
      Value* _a = nullptr;
      if (arguments.size() > 3) {
        _a = arguments[3];
      }
      // Check if any `calc()` or `var()` are passed
      if (isSpecialNumber(_h) || isSpecialNumber(_w) || isSpecialNumber(_b) || isSpecialNumber(_a)) {
        sass::sstream fncall;
        fncall << name << "(";
        fncall << _h->inspect() << ", ";
        fncall << _w->inspect() << ", ";
        fncall << _b->inspect();
        if (_a) { fncall << ", " << _a->inspect(); }
        fncall << ")";
        return SASS_MEMORY_NEW(String, pstate, fncall.str());
      }

      Number* h = _h->assertNumber(logger, Strings::hue);
      Number* w = _w->assertNumber(logger, Strings::whiteness);
      Number* b = _b->assertNumber(logger, Strings::blackness);
      Number* a = _a ? _a->assertNumber(logger, Strings::alpha) : nullptr;

      return SASS_MEMORY_NEW(ColorHwba, pstate,
        h->value(),
        clamp(w->value(), 0.0, 100.0),
        clamp(b->value(), 0.0, 100.0),
        _a ? _percentageOrUnitless(a, 1.0, "$alpha", logger) : 1.0);

    }

    Value* hslFn(const sass::string& name, const ValueVector& arguments, const SourceSpan& pstate, Logger& logger)
    {
      Value* _h = arguments[0];
      Value* _s = arguments[1];
      Value* _l = arguments[2];
      Value* _a = nullptr;
      if (arguments.size() > 3) {
        _a = arguments[3];
      }
      // Check if any `calc()` or `var()` are passed
      if (isSpecialNumber(_h) || isSpecialNumber(_s) || isSpecialNumber(_l) || isSpecialNumber(_a)) {
        sass::sstream fncall;
        fncall << name << "(";
        fncall << _h->inspect() << ", ";
        fncall << _s->inspect() << ", ";
        fncall << _l->inspect();
        if (_a) { fncall << ", " << _a->inspect(); }
        fncall << ")";
        return SASS_MEMORY_NEW(String, pstate, fncall.str());
      }

      Number* h = _h->assertNumber(logger, Strings::hue);
      Number* s = _s->assertNumber(logger, Strings::saturation);
      Number* l = _l->assertNumber(logger, Strings::lightness);
      Number* a = _a ? _a->assertNumber(logger, Strings::alpha) : nullptr;

      return SASS_MEMORY_NEW(ColorHsla, pstate,
        h->value(),
        clamp(s->value(), 0.0, 100.0),
        clamp(l->value(), 0.0, 100.0),
        _a ? _percentageOrUnitless(a, 1.0, "$alpha", logger) : 1.0);
    }

  }

}
