/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_values.hpp"

#include "logger.hpp"
#include "fn_utils.hpp"
#include "exceptions.hpp"
#include "dart_helpers.hpp"
#include <algorithm>

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // This should be thread-safe
  static std::hash<bool> boolHasher;
  static std::hash<double> doubleHasher;
  static std::hash<std::size_t> sizetHasher;
  static std::hash<sass::string> stringHasher;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CustomError::CustomError(
    const SourceSpan& pstate,
    const sass::string& msg) :
    Value(pstate),
    message_(msg)
  {}

  CustomError::CustomError(const CustomError* ptr)
    : Value(ptr), message_(ptr->message_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool CustomError::operator== (const Value& rhs) const
  {
    if (auto right = rhs.isaCustomError()) {
      return *this == *right;
    }
    return false;
  }

  bool CustomError::operator== (const CustomError& rhs) const
  {
    return message() == rhs.message();
  }

  /////////////////////////////////////////////////////////////////////////

  void CustomError::accept(ValueVisitor<void>* visitor) {
    throw std::runtime_error("CustomError::accept not implemented");
  }
  Value* CustomError::accept(ValueVisitor<Value*>* visitor) {
    throw std::runtime_error("CustomError::accept not implemented");
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CustomWarning::CustomWarning(
    const SourceSpan& pstate,
    const sass::string& msg) :
    Value(pstate),
    message_(msg)
  {}

  CustomWarning::CustomWarning(const CustomWarning* ptr)
    : Value(ptr), message_(ptr->message_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool CustomWarning::operator== (const Value& rhs) const
  {
    if (auto right = rhs.isaCustomWarning()) {
      return *this == *right;
    }
    return false;
  }

  bool CustomWarning::operator== (const CustomWarning& rhs) const
  {
    return message() == rhs.message();
  }

  /////////////////////////////////////////////////////////////////////////

  void CustomWarning::accept(ValueVisitor<void>* visitor) {
    throw std::runtime_error("CustomWarning::accept not implemented");
  }
  Value* CustomWarning::accept(ValueVisitor<Value*>* visitor) {
    throw std::runtime_error("CustomWarning::accept not implemented");
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Null::Null(const SourceSpan& pstate)
    : Value(pstate)
  {}

  Null::Null(const Null* ptr)
    : Value(ptr)
  {}

  bool Null::operator== (const Value& rhs) const
  {
    return rhs.isNull();
  }

  size_t Null::hash() const
  {
    return typeid(Null).hash_code();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Color::Color(
    const SourceSpan& pstate,
    double alpha,
    const sass::string& disp) :
    Value(pstate),
    disp_(disp),
    a_(alpha)
  {}

  Color::Color(const Color* ptr)
    : Value(ptr),
    // Reset on copy
    // disp_(ptr->disp_),
    a_(ptr->a_)
  {}

  /////////////////////////////////////////////////////////////////////////
  // Implement value operators for color
  /////////////////////////////////////////////////////////////////////////

  Value* Color::plus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (other->isaNumber() || other->isaColor()) {
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Undefined operation \"" + inspect()
        + " + " + other->inspect() + "\".",
        logger, pstate);
    }
    return Value::plus(other, logger, pstate);
  }

  Value* Color::minus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (other->isaNumber() || other->isaColor()) {
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Undefined operation \"" + inspect()
        + " - " + other->inspect() + "\".",
        logger, pstate);
    }
    return Value::minus(other, logger, pstate);
  }

  Value* Color::dividedBy(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (other->isaNumber() || other->isaColor()) {
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Undefined operation \"" + inspect()
        + " / " + other->inspect() + "\".",
        logger, pstate);
    }
    return Value::dividedBy(other, logger, pstate);
  }

  Value* Color::modulo(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " % " + other->inspect() + "\".",
      logger, pstate);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ColorRgba::ColorRgba(
    const SourceSpan& pstate,
    double red,
    double green,
    double blue,
    double alpha,
    const sass::string& disp) :
    Color(pstate, alpha, disp),
    r_(red),
    g_(green),
    b_(blue)
  {}

  ColorRgba::ColorRgba(const ColorRgba* ptr)
    : Color(ptr),
    r_(ptr->r_),
    g_(ptr->g_),
    b_(ptr->b_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool ColorRgba::operator== (const Value& rhs) const
  {
    if (const Color* color = rhs.isaColor()) {
      ColorRgba* rgba = color->toRGBA();
      return *this == *rgba;
    }
    return false;
  }

  bool ColorRgba::operator== (const ColorRgba& rhs) const
  {
    return r_ == rhs.r() &&
      g_ == rhs.g() &&
      b_ == rhs.b() &&
      a_ == rhs.a();
  }

  size_t ColorRgba::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(ColorRgba).hash_code());
      hash_combine(hash_, doubleHasher(a_));
      hash_combine(hash_, doubleHasher(r_));
      hash_combine(hash_, doubleHasher(g_));
      hash_combine(hash_, doubleHasher(b_));
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////

  ColorHsla* ColorRgba::copyAsHSLA() const
  {

    // Algorithm from http://en.wikipedia.org/wiki/wHSL_and_HSV#Conversion_from_RGB_to_HSL_or_HSV
    double r = r_ / 255.0;
    double g = g_ / 255.0;
    double b = b_ / 255.0;

    double max = std::max(r, std::max(g, b));
    double min = std::min(r, std::min(g, b));
    double delta = max - min;

    double h = 0;
    double s;
    double l = (max + min) / 2.0;

    if (NEAR_EQUAL(max, min)) {
      h = s = 0; // achromatic
    }
    else {
      if (l < 0.5) s = delta / (max + min);
      else         s = delta / (2.0 - max - min);

      if (r == max) h = (g - b) / delta + (g < b ? 6 : 0);
      else if (g == max) h = (b - r) / delta + 2;
      else if (b == max) h = (r - g) / delta + 4;
    }

    // HSL hsl_struct;
    h = h * 60;
    s = s * 100;
    l = l * 100;

    return SASS_MEMORY_NEW(ColorHsla,
      pstate(), h, s, l, a(), ""
    );
  }

  ColorHwba* ColorRgba::copyAsHWBA() const
  {

    // Algorithm from http://en.wikipedia.org/wiki/wHSL_and_HSV#Conversion_from_RGB_to_HSL_or_HSV
    double r = r_ / 255.0;
    double g = g_ / 255.0;
    double b = b_ / 255.0;

    double max = std::max(r, std::max(g, b));
    double min = std::min(r, std::min(g, b));
    double delta = max - min;

    double h = 0;

    if (NEAR_EQUAL(max, min)) {
      h = 0; // achromatic
    }
    else {
      if (r == max) h = (g - b) / delta + (g < b ? 6 : 0);
      else if (g == max) h = (b - r) / delta + 2;
      else if (b == max) h = (r - g) / delta + 4;
    }

    double _w = std::min(r, std::min(g, b));
    double _b = 1.0 - std::max(r, std::max(g, b));

    // HSL hsl_struct;
    h = h * 60;
    _w *= 100;
    _b *= 100;

    return SASS_MEMORY_NEW(ColorHwba, pstate_, h, _w, _b, a_);

  }

  ColorHsla* ColorRgba::toHSLA() const
  {
    return copyAsHSLA();
  }

  ColorHwba* ColorRgba::toHWBA() const
  {
    return copyAsHWBA();
  }

  ColorRgba* ColorRgba::copyAsRGBA() const
  {
    return SASS_MEMORY_COPY(this);
  }

  ColorRgba* ColorRgba::toRGBA() const
  {
    // This is safe, I know what I do!
    return const_cast<ColorRgba*>(this);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ColorHsla::ColorHsla(
    const SourceSpan& pstate,
    double hue,
    double saturation,
    double lightness,
    double alpha,
    const sass::string& disp) :
    Color(pstate, alpha, disp),
    h_(absmod(hue, 360.0)),
    s_(clamp(saturation, 0.0, 100.0)),
    l_(clamp(lightness, 0.0, 100.0))
  {}

  ColorHsla::ColorHsla(const ColorHsla* ptr)
    : Color(ptr),
    h_(ptr->h_),
    s_(ptr->s_),
    l_(ptr->l_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool ColorHsla::operator== (const Value& rhs) const
  {
    if (const Color* color = rhs.isaColor()) {
      ColorHsla* hsla = color->toHSLA();
      return *this == *hsla;
    }
    return false;
  }

  bool ColorHsla::operator== (const ColorHsla& rhs) const
  {
    return h_ == rhs.h() &&
      s_ == rhs.s() &&
      l_ == rhs.l() &&
      a_ == rhs.a();
  }

  size_t ColorHsla::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(ColorHsla).hash_code());
      hash_combine(hash_, doubleHasher(a_));
      hash_combine(hash_, doubleHasher(h_));
      hash_combine(hash_, doubleHasher(s_));
      hash_combine(hash_, doubleHasher(l_));
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ColorHwba::ColorHwba(
    const SourceSpan& pstate,
    double hue,
    double whiteness,
    double blackness,
    double alpha,
    const sass::string& disp) :
    Color(pstate, alpha, disp),
    h_(absmod(hue, 360.0)),
    w_(clamp(whiteness, 0.0, 100.0)),
    b_(clamp(blackness, 0.0, 100.0))
  {}

  ColorHwba::ColorHwba(const ColorHwba* ptr)
    : Color(ptr),
    h_(ptr->h_),
    w_(ptr->w_),
    b_(ptr->b_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool ColorHwba::operator== (const Value& rhs) const
  {
    if (const Color* color = rhs.isaColor()) {
      ColorHwba* hwba = color->toHWBA();
      return *this == *hwba;
    }
    return false;
  }

  bool ColorHwba::operator== (const ColorHwba& rhs) const
  {
    return h_ == rhs.h() &&
      w_ == rhs.w() &&
      b_ == rhs.b() &&
      a_ == rhs.a();
  }

  size_t ColorHwba::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(ColorHsla).hash_code());
      hash_combine(hash_, doubleHasher(a_));
      hash_combine(hash_, doubleHasher(h_));
      hash_combine(hash_, doubleHasher(w_));
      hash_combine(hash_, doubleHasher(b_));
    }
    return hash_;
  }


  ColorHwba* ColorHwba::copyAsHWBA() const
  {
    return SASS_MEMORY_COPY(this);
  }

  ColorRgba* ColorHwba::copyAsRGBA() const
  {
    double h = h_ / 360.0;
    double wh = w_ / 100.0;
    double bl = b_ / 100.0;
    double ratio = wh + bl;
    double v, f, n;
    if (ratio > 1) {
      wh /= ratio;
      bl /= ratio;
    }
    int i = (int)floor(6.0 * h);
    v = 1.0 - bl;
    f = 6.0 * h - i;
    if ((i & 1) != 0) {
       f = 1 - f;
     }
    n = wh + f * (v - wh);
    double r, g, b;
    switch (i) {
    default:
    case 6:
    case 0: r = v; g = n; b = wh; break;
    case 1: r = n; g = v; b = wh; break;
    case 2: r = wh; g = v; b = n; break;
    case 3: r = wh; g = n; b = v; break;
    case 4: r = n; g = wh; b = v; break;
    case 5: r = v; g = wh; b = n; break;
    }
    return SASS_MEMORY_NEW(ColorRgba,
      pstate_, r * 255.0, g * 255.0, b * 255.0, a_);
  }

  ColorHsla* ColorHwba::copyAsHSLA() const
  {
    ColorRgbaObj rgba(copyAsRGBA());
    return rgba->copyAsHSLA();
  }

  ColorHsla* ColorHwba::toHSLA() const
  {
    return copyAsHSLA();
  }

  ColorHwba* ColorHwba::toHWBA() const
  {
    return const_cast<ColorHwba*>(this);;
  }

  ColorRgba* ColorHwba::toRGBA() const
  {
    return copyAsRGBA();
  }



  /////////////////////////////////////////////////////////////////////////

  // hue to RGB helper function
  double h_to_rgb(double m1, double m2, double h)
  {
    h = absmod(h, 1.0);
    if (h * 6.0 < 1) return m1 + (m2 - m1) * h * 6;
    if (h * 2.0 < 1) return m2;
    if (h * 3.0 < 2) return m1 + (m2 - m1) * (2.0 / 3.0 - h) * 6;
    return m1;
  }

  ColorRgba* ColorHsla::copyAsRGBA() const
  {
    double h = absmod(h_ / 360.0, 1.0);
    double s = clamp(s_ / 100.0, 0.0, 1.0);
    double l = clamp(l_ / 100.0, 0.0, 1.0);

    // Algorithm from the CSS3 spec: http://www.w3.org/TR/css3-color/#hsl-color.
    double m2;
    if (l <= 0.5) m2 = l * (s + 1.0);
    else m2 = (l + s) - (l * s);
    double m1 = (l * 2.0) - m2;
    // round the results -- consider moving this into the Color constructor
    double r = (h_to_rgb(m1, m2, h + 1.0 / 3.0) * 255.0);
    double g = (h_to_rgb(m1, m2, h) * 255.0);
    double b = (h_to_rgb(m1, m2, h - 1.0 / 3.0) * 255.0);

    return SASS_MEMORY_NEW(ColorRgba,
      pstate(), r, g, b, a(), ""
    );
  }

  ColorHwba* ColorHsla::copyAsHWBA() const
  {
    ColorRgbaObj rgba(copyAsRGBA());
    return rgba->copyAsHWBA();

    throw std::runtime_error("invalid");
    return nullptr;
  }

  ColorHsla* ColorHsla::copyAsHSLA() const
  {
    return SASS_MEMORY_COPY(this);
  }

  ColorRgba* ColorHsla::toRGBA() const
  {
    return copyAsRGBA();
  }

  ColorHwba* ColorHsla::toHWBA() const
  {
    return copyAsHWBA();
  }

  ColorHsla* ColorHsla::toHSLA() const
  {
    // This is safe, I know what I do!
    return const_cast<ColorHsla*>(this);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  Number::Number(
    const SourceSpan& pstate,
    double value,
    const sass::string& units) :
    Value(pstate),
    Units(units),
    value_(value),
    lhsAsSlash_(),
    rhsAsSlash_()
  {}

  // Copy constructor
  Number::Number(const Number* ptr) :
    Value(ptr),
    Units(ptr),
    value_(ptr->value_),
    lhsAsSlash_(ptr->lhsAsSlash_),
    rhsAsSlash_(ptr->rhsAsSlash_)
  {}

  /////////////////////////////////////////////////////////////////////////
  // Implement base value equality comparator
  /////////////////////////////////////////////////////////////////////////

  // Helper to determine if we can work with both numbers directly
  bool isSimpleNumberComparison(const Number& lhs, const Number& rhs)
  {
    // Gather statistics from the units
    size_t l_n_units = lhs.numerators.size();
    size_t r_n_units = rhs.numerators.size();
    size_t l_d_units = lhs.denominators.size();
    size_t r_d_units = rhs.denominators.size();
    size_t l_units = l_n_units + l_d_units;
    size_t r_units = r_n_units + r_d_units;

    // Old ruby sass behavior (deprecated)
    if (l_units == 0) return true;
    if (r_units == 0) return true;

    // check if both sides have exactly the same units
    if (l_n_units == r_n_units && l_d_units == r_d_units) {
      return (lhs.numerators == rhs.numerators)
        && (lhs.denominators == rhs.denominators);
    }

    return false;
  }
  // EO isSimpleNumberComparison

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool Number::operator== (const Value& rhs) const
  {
    if (const Number* number = rhs.isaNumber()) {
      return *this == *number;
    }
    return false;
  }

  bool Number::operator== (const Number& rhs) const
  {
    if (isUnitless() && rhs.isUnitless()) {
      return NEAR_EQUAL(value(), rhs.value());
    }
    // Ignore units in certain cases
    // if (isSimpleNumberComparison(*this, rhs)) {
    //   return NEAR_EQUAL(value(), rhs.value());
    // }
    // Otherwise we need copies
    Number l(*this), r(rhs);
    // Reduce and normalize
    l.reduce(); r.reduce();
    l.normalize(); r.normalize();
    // Ensure both have same units
    return l.Units::operator==(r) &&
      NEAR_EQUAL(l.value(), r.value());
  }

  size_t Number::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, doubleHasher(value_));
      for (const auto& numerator : numerators)
        hash_combine(hash_, stringHasher(numerator));
      for (const auto& denominator : denominators)
        hash_combine(hash_, stringHasher(denominator));
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  // Implement value comparators for number
  /////////////////////////////////////////////////////////////////////////

  bool Number::greaterThan(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (Number* rhs = other->isaNumber()) {
      // Ignore units in certain cases
      if (isSimpleNumberComparison(*this, *rhs)) {
        return value() > rhs->value();
      }
      // Otherwise we need copies
      Number l(*this), r(*rhs);
      // Reduce and normalize
      l.reduce(); r.reduce();
      l.normalize(); r.normalize();
      // Ensure both have same units
      if (l.Units::operator==(r)) {
        return l.value() > r.value();
      }
      // Throw error, unit are incompatible
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Incompatible units " + l.unit()
        + " and " + r.unit() + ".",
        logger, pstate);
    }
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " > " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO greaterThan

  bool Number::greaterThanOrEquals(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (Number* rhs = other->isaNumber()) {
      // Ignore units in certain cases
      if (isSimpleNumberComparison(*this, *rhs)) {
        return value() >= rhs->value();
      }
      // Otherwise we need copies
      Number l(*this), r(*rhs);
      // Reduce and normalize
      l.reduce(); r.reduce();
      l.normalize(); r.normalize();
      // Ensure both have same units
      if (l.Units::operator==(r)) {
        return l.value() >= r.value();
      }
      // Throw error, unit are incompatible
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Incompatible units " + l.unit()
        + " and " + r.unit() + ".",
        logger, pstate);
    }
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " >= " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO greaterThanOrEquals

  bool Number::lessThan(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (Number* rhs = other->isaNumber()) {
      // Ignore units in certain cases
      if (isSimpleNumberComparison(*this, *rhs)) {
        return value() < rhs->value();
      }
      // Otherwise we need copies
      Number l(*this), r(*rhs);
      // Reduce and normalize
      l.reduce(); r.reduce();
      l.normalize(); r.normalize();
      // Ensure both have same units
      if (l.Units::operator==(r)) {
        return l.value() < r.value();
      }
      // Throw error, unit are incompatible
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Incompatible units " + l.unit()
        + " and " + r.unit() + ".",
        logger, pstate);
    }
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " < " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO lessThan

  bool Number::lessThanOrEquals(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (Number* rhs = other->isaNumber()) {
      // Ignore units in certain cases
      if (isSimpleNumberComparison(*this, *rhs)) {
        return value() <= rhs->value();
      }
      // Otherwise we need copies
      Number l(*this), r(*rhs);
      // Reduce and normalize
      l.reduce(); r.reduce();
      l.normalize(); r.normalize();
      // Ensure both have same units
      if (l.Units::operator==(r)) {
        return l.value() <= r.value();
      }
      // Throw error, unit are incompatible
      logger.addFinalStackTrace(pstate);
      throw Exception::SassScriptException(
        "Incompatible units " + l.unit()
        + " and " + r.unit() + ".",
        logger, pstate);
    }
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " <= " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO lessThanOrEquals

  /////////////////////////////////////////////////////////////////////////
  // Helper functions to do the raw value operations
  /////////////////////////////////////////////////////////////////////////

  // Local functions that implement the value operation
  inline double add(double x, double y) { return x + y; }
  inline double sub(double x, double y) { return x - y; }
  inline double mul(double x, double y) { return x * y; }
  inline double div(double x, double y) { return x / y; }
  inline double mod(double x, double y)
  {
    if ((x > 0 && y < 0) || (x < 0 && y > 0)) {
      double ret = std::fmod(x, y);
      return ret ? ret + y : ret;
    }
    else {
      return std::fmod(x, y);
    }
  }

  /////////////////////////////////////////////////////////////////////////
  // Implement value operators for number
  /////////////////////////////////////////////////////////////////////////

  Value* Number::operate(double (*op)(double, double), const Number& rhs, Logger& logger, const SourceSpan& pstate) const
  {

    size_t l_n_units = numerators.size();
    size_t l_d_units = denominators.size();
    size_t r_n_units = rhs.numerators.size();
    size_t r_d_units = rhs.denominators.size();
    size_t l_units = l_n_units + l_d_units;
    size_t r_units = r_n_units + r_d_units;

    double lval = value();
    double rval = rhs.value();

    // Catch modulo by zero
    if (op == mod && rval == 0) {
      return SASS_MEMORY_NEW(Number, pstate,
        std::numeric_limits<double>::quiet_NaN());
    }
    // Catch division by zero
    else if (op == div && rval == 0) {
      if (lval) return SASS_MEMORY_NEW(Number, pstate,
        std::numeric_limits<double>::infinity());
      else return SASS_MEMORY_NEW(Number, pstate,
        std::numeric_limits<double>::quiet_NaN());
    }

    // Simplest case with no units
    // Just operate on the values
    if (r_units == 0) {
      if (l_units <= 1) {
        Number* copy = SASS_MEMORY_COPY(this);
        copy->value(op(lval, rval));
        copy->pstate(pstate);
        return copy;
      }
    }
    // Left hand has no unit, so we can just copy
    // the units from the right hand side. If units
    // are not compatible, op function will throw!
    else if (l_units == 0) {
      if (r_units == 1) {
        Number* copy = SASS_MEMORY_COPY(this);
        copy->value(op(lval, rval));
        // Switch units for division
        if (op == div) {
          copy->numerators = rhs.denominators;
          copy->denominators = rhs.numerators;
        }
        else {
          copy->numerators = rhs.numerators;
          copy->denominators = rhs.denominators;
        }
        copy->pstate(pstate);
        return copy;
      }
    }
    // Both sides have exactly one unit
    // Most used case, so optimize it too!
    else if (l_units == 1 && r_units == 1) {
      if (numerators == rhs.numerators) {
        if (denominators == rhs.denominators) {
          Number* copy = SASS_MEMORY_COPY(this);
          copy->value(op(lval, rval));
          if (op == div) {
            copy->numerators.clear();
            copy->denominators.clear();
          }
          else if (op == mul) {
            copy->numerators.insert(copy->numerators.end(),
              rhs.numerators.begin(), rhs.numerators.end());
            copy->denominators.insert(copy->denominators.end(),
              rhs.denominators.begin(), rhs.denominators.end());
          }
          copy->pstate(pstate);
          return copy;
        }
      }
    }

    // Otherwise we go into the generic operation
    NumberObj copy = SASS_MEMORY_COPY(this);

    // Move right units for some operations if left has none yet
    if (isUnitless() && (op == add || op == sub || op == mod)) {
      copy->numerators = rhs.numerators;
      copy->denominators = rhs.denominators;
    }

    if (op == mul) {
      // Multiply the values
      copy->value(op(lval, rval));
      // Add all units for multiplications
      copy->numerators.insert(copy->numerators.end(),
        rhs.numerators.begin(), rhs.numerators.end());
      copy->denominators.insert(copy->denominators.end(),
        rhs.denominators.begin(), rhs.denominators.end());
      // Do logical unit cleanup
      copy->reduce();
    }
    else if (op == div) {
      // Divide the values
      copy->value(op(lval, rval));
      // Add reversed units for division
      copy->numerators.insert(copy->numerators.end(),
        rhs.denominators.begin(), rhs.denominators.end());
      copy->denominators.insert(copy->denominators.end(),
        rhs.numerators.begin(), rhs.numerators.end());
      // Do logical unit cleanup
      copy->reduce();
    }
    else {
      // Only needed if at least two units are used
      // Can work directly if both sides are equal
      Number left(this), right(rhs);
      left.reduce(); right.reduce();
      // Get the necessary conversion factor
      double f(right.getUnitConvertFactor(left));
      // Returns zero on incompatible units
      if (f == 0.0) {
        logger.addFinalStackTrace(pstate);
        throw Exception::SassScriptException(
          "Incompatible units " + rhs.unit()
          + " and " + unit() + ".",
          logger, pstate);
      }
      // Now apply the conversion factor
      copy->value(op(lval, right.value() * f));
    }

    copy->pstate(pstate);
    return copy.detach();
  }
  // EO operate

  Value* Number::plus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(add, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::plus(other, logger, pstate);
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " + " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO plus

  Value* Number::minus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(sub, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::minus(other, logger, pstate);
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " - " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO minus

  Value* Number::times(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(mul, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::times(other, logger, pstate);
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " * " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO times

  Value* Number::modulo(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(mod, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::plus(other, logger, pstate);
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " % " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO modulo

  Value* Number::dividedBy(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(div, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::dividedBy(other, logger, pstate);
    logger.addFinalStackTrace(pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " / " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO dividedBy

  /////////////////////////////////////////////////////////////////////////
  // Implement unary operations for base value class
  /////////////////////////////////////////////////////////////////////////

  Value* Number::unaryPlus(Logger& logger, const SourceSpan& pstate) const
  {
    return SASS_MEMORY_COPY(this);
  }

  Value* Number::unaryMinus(Logger& logger, const SourceSpan& pstate) const
  {
    Number* cpy = SASS_MEMORY_COPY(this);
    cpy->value(cpy->value() * -1.0);
    return cpy;
  }

  /////////////////////////////////////////////////////////////////////////
  // Implement number specific assertions
  /////////////////////////////////////////////////////////////////////////

  long Number::assertInt(Logger& logger, const sass::string& name)
  {
    if (fuzzyIsInt(value_, logger.epsilon)) {
      return lround(value_);
    }
    SourceSpan span(this->pstate());
    logger.addFinalStackTrace(span);
    throw Exception::SassScriptException(
      inspect() + " is not an int.",
      logger, span, name);
  }

  Number* Number::assertUnitless(Logger& logger, const sass::string& name)
  {
    if (!hasUnits()) return this;
    SourceSpan span(this->pstate());
    logger.addFinalStackTrace(span);
    throw Exception::SassScriptException(
      "Expected " + inspect() + " to have no units.",
      logger, span, name);
  }

  Number* Number::assertHasUnits(Logger& logger, const sass::string& unit, const sass::string& name)
  {
    if (hasUnit(unit)) return this;
    SourceSpan span(this->pstate());
    logger.addFinalStackTrace(span);
    throw Exception::SassScriptException(
      "Expected " + inspect() + " to have unit \"" + unit + "\".",
      logger, span, name);
  }

  double Number::assertRange(double min, double max, Logger& logger, const sass::string& name) const
  {
    if (!fuzzyCheckRange(value_, min, max, logger.epsilon)) {
      sass::sstream msg;
      msg << "Expected " << inspect() << " to be within ";
      msg << min << unit() << " and " << max << unit() << ".";
      SourceSpan span(this->pstate());
      logger.addFinalStackTrace(span);
      throw Exception::SassScriptException(
        msg.str(), logger, span, name);
    }
    return value_;
  }

  /////////////////////////////////////////////////////////////////////////
  // Implement delayed value fetcher
  /////////////////////////////////////////////////////////////////////////

  Value* Number::withoutSlash()
  {
    if (!hasAsSlash()) return this;
    // we are the only holder of this item
    // therefore should be safe to alter it
    if (this->refcount == 1) {
      lhsAsSlash_.clear();
      rhsAsSlash_.clear();
      return this;
    }
    // Otherwise we need to make a copy first
    Number* copy = SASS_MEMORY_COPY(this);
    copy->lhsAsSlash_.clear();
    copy->rhsAsSlash_.clear();
    return copy;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Boolean::Boolean(
    const SourceSpan& pstate,
    bool value) :
    Value(pstate),
    value_(value)
  {}

  Boolean::Boolean(
    const Boolean* ptr) :
    Value(ptr),
    value_(ptr->value_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool Boolean::operator== (const Value& rhs) const
  {
    if (auto right = rhs.isaBoolean()) {
      return *this == *right;
    }
    return false;
  }

  bool Boolean::operator== (const Boolean& rhs) const
  {
    return value() == rhs.value();
  }

  size_t Boolean::hash() const
  {
    if (hash_ == 0) {
      hash_ = boolHasher(value_);
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  String::String(
    const SourceSpan& pstate,
    const char* value,
    bool hasQuotes) :
    Value(pstate),
    value_(value),
    hasQuotes_(hasQuotes)
  {}

  String::String(
    const SourceSpan& pstate,
    sass::string&& value,
    bool hasQuotes) :
    Value(pstate),
    value_(std::move(value)),
    hasQuotes_(hasQuotes)
  {}

  String::String(const String* ptr) :
    Value(ptr),
    value_(ptr->value_),
    hasQuotes_(ptr->hasQuotes_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool String::operator== (const Value& rhs) const
  {
    if (auto right = rhs.isaString()) {
      return *this == *right;
    }
    return false;
  }

  bool String::operator== (const String& rhs) const
  {
    return value() == rhs.value();
  }

  size_t String::hash() const
  {
    if (hash_ == 0) {
      hash_ = stringHasher(value_);
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////

  Value* String::plus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const String* str = other->isaString()) {
      sass::string text(value() + str->value());
      return SASS_MEMORY_NEW(String,
        pstate, std::move(text), hasQuotes());
    }
    sass::string text(value() + other->toCss(logger));
    return SASS_MEMORY_NEW(String,
      pstate, std::move(text), hasQuotes());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Map::Map(
    const SourceSpan& pstate,
    Hashed::ordered_map_type&& move) :
    Value(pstate),
    Hashed(std::move(move))
  {}

  Map::Map(const Map* ptr) :
    Value(ptr),
    Hashed(*ptr)
  {}

  /////////////////////////////////////////////////////////////////////////

  // Maps are equal if they have the same items
  // at the same key, order is not important.
  bool Map::operator== (const Value& rhs) const
  {
    if (const Map* right = rhs.isaMap()) {
      return *this == *right;
    }
    if (const List* right = rhs.isaList()) {
      return right->empty() && empty();
    }
    return false;
  }

  // Maps are equal if they have the same items
  // at the same key, order is not important.
  bool Map::operator== (const Map& rhs) const
  {
    if (size() != rhs.size()) return false;
    for (auto kv : elements_) {
      auto lv = kv.second;
      auto rv = rhs.at(kv.first);
      return ObjEqualityFn(lv, rv);
    }
    return true;
  }

  size_t Map::hash() const
  {
    if (Hashed<ValueObj, ValueObj>::hash_ == 0) {
      hash_start(Value::hash_, typeid(Map).hash_code());
      hash_combine(Value::hash_, Hashed<ValueObj, ValueObj>::hash());
    }
    return Value::hash_;
  }

  /////////////////////////////////////////////////////////////////////////

  // Search the position of the given value
  size_t Map::indexOf(Value* value)
  {
    if (List* list = value->isaList()) {
      if (list->size() == 2) {
        Value* key = list->get(0);
        Value* val = list->get(1);
        size_t idx = 0;
        for (auto kv : elements_) {
          if (*kv.first == *key) {
            if (*kv.second == *val) {
              return idx;
            }
          }
          ++idx;
        }
      }
    }
    return NPOS;
  }

  // Return list with two items (key and value)
  Value* Map::getPairAsList(size_t idx)
  {
    auto kv = elements_.begin() + idx;
    // ToDo: really can't re-use memory?
    if (false && itpair->size() == 2) {
      itpair->get(0) = kv->first;
      itpair->get(1) = kv->second;
    }
    else {
      itpair = SASS_MEMORY_NEW(
        List, pstate(),
        { kv->first, kv->second },
        SASS_SPACE);
    }
    return itpair.detach();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  List::List(
    const SourceSpan& pstate,
    const ValueVector& values,
    SassSeparator separator,
    bool hasBrackets) :
    Value(pstate),
    Vectorized(values),
    separator_(separator),
    hasBrackets_(hasBrackets)
  {}

  List::List(const SourceSpan& pstate,
    ValueVector&& values,
    SassSeparator separator,
    bool hasBrackets) :
    Value(pstate),
    Vectorized(std::move(values)),
    separator_(separator),
    hasBrackets_(hasBrackets)
  {}

  List::List(
    const List* ptr) :
    Value(ptr),
    Vectorized(ptr),
    separator_(ptr->separator_),
    hasBrackets_(ptr->hasBrackets_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool List::operator==(const Value& rhs) const
  {
    if (const List* right = rhs.isaList()) {
      return *this == *right;
    }
    if (const Map* right = rhs.isaMap()) {
      return empty() && right->empty();
    }
    return false;
  }

  bool List::operator==(const List& rhs) const
  {
    if (size() != rhs.size()) return false;
    if (separator() != rhs.separator()) return false;
    if (hasBrackets() != rhs.hasBrackets()) return false;
    for (size_t i = 0, L = size(); i < L; ++i) {
      auto rv = rhs.get(i);
      auto lv = this->get(i);
      if (!lv && rv) return false;
      else if (!rv && lv) return false;
      else if (!(*lv == *rv)) return false;
    }
    return true;
  }

  size_t List::hash() const
  {
    if (Vectorized<Value>::hash_ == 0) {
      hash_start(Value::hash_, typeid(List).hash_code());
      hash_combine(Value::hash_, Vectorized<Value>::hash());
      hash_combine(Value::hash_, sizetHasher(separator()));
      hash_combine(Value::hash_, boolHasher(hasBrackets()));
    }
    return Value::hash_;
  }

  /////////////////////////////////////////////////////////////////////////

  Map* List::assertMap(Logger& logger, const sass::string& name)
  {
    if (!empty()) { return Value::assertMap(logger, name); }
    else { return SASS_MEMORY_NEW(Map, pstate()); }
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  ArgumentList::ArgumentList(
    const SourceSpan& pstate,
    SassSeparator separator,
    const ValueVector& values,
    const ValueFlatMap& keywords) :
    List(pstate,
      values,
      separator,
      false),
    _keywords(keywords),
    _wereKeywordsAccessed(false)
  {}

  ArgumentList::ArgumentList(
    const SourceSpan& pstate,
    SassSeparator separator,
    ValueVector&& values,
    ValueFlatMap&& keywords) :
    List(pstate,
      std::move(values),
      separator,
      false),
    _keywords(std::move(keywords)),
    _wereKeywordsAccessed(false)
  {}

  ArgumentList::ArgumentList(
    const ArgumentList* ptr) :
    List(ptr),
    _keywords(ptr->_keywords),
    _wereKeywordsAccessed(ptr->_wereKeywordsAccessed)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool ArgumentList::operator==(const Value& rhs) const
  {
    if (const ArgumentList* right = rhs.isaArgumentList()) {
      return *this == *right;
    }
    return List::operator==(rhs);
  }

  bool ArgumentList::operator==(const ArgumentList& rhs) const
  {
    return _keywords == rhs._keywords;
  }

  size_t ArgumentList::hash() const
  {
    if (Vectorized<Value>::hash_ == 0) {
      hash_start(Value::hash_, typeid(ArgumentList).hash_code());
      hash_combine(Value::hash_, Vectorized<Value>::hash());
      for (auto child : _keywords) {
        hash_combine(Value::hash_, child.first.hash());
        hash_combine(Value::hash_, child.second->hash());
      }
    }
    return Value::hash_;
  }

  /////////////////////////////////////////////////////////////////////////

  // Convert native string keys to sass strings
  Map* ArgumentList::keywordsAsSassMap() const
  {
    Map* map = SASS_MEMORY_NEW(Map, pstate());
    for (auto kv : _keywords) {
      String* keystr = SASS_MEMORY_NEW(
        String, kv.second->pstate(),
        sass::string(kv.first.orig()));
      map->insert(keystr, kv.second);
    }
    return map;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Function::Function(
    const SourceSpan& pstate,
    CallableObj callable) :
    Value(pstate),
    callable_(callable)
  {}

  Function::Function(const Function* ptr) :
    Value(ptr),
    callable_(ptr->callable_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool Function::operator== (const Value& rhs) const
  {
    if (const Function* fn = rhs.isaFunction()) {
      return *this == *fn;
    }
    return false;
  }

  bool Function::operator== (const Function& rhs) const
  {
    return ObjEqualityFn(callable_, rhs.callable());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
