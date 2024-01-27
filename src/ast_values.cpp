/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_values.hpp"

#include "logger.hpp"
#include "fn_utils.hpp"
#include "exceptions.hpp"
#include "dart_helpers.hpp"
#include "ast_nodes.hpp"
#include "unicode.hpp"

#include <algorithm>

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // This should be thread-safe
  static std::hash<bool> boolHasher;
  static std::hash<double> doubleHasher;
  static std::hash<std::size_t> sizetHasher;
  static std::hash<sass::string> stringHasher;

  const double NaN = std::numeric_limits<double>::quiet_NaN();

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
    throw std::runtime_error("CustomError::accept<void> not implemented");
  }
  Value* CustomError::accept(ValueVisitor<Value*>* visitor) {
    throw std::runtime_error("CustomError::accept<Value> not implemented");
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
    throw std::runtime_error("CustomWarning::accept<void> not implemented");
  }
  Value* CustomWarning::accept(ValueVisitor<Value*>* visitor) {
    throw std::runtime_error("CustomWarning::accept<Value> not implemented");
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Calculation::Calculation(const SourceSpan& pstate, const sass::string& name,
    const sass::vector<AstNodeObj> arguments) :
    Value(pstate),
    name_(name),
    arguments_(arguments)
  {}

  Calculation::Calculation(const Calculation* ptr)
    : Value(ptr)
  {}
  /// Returns whether [character] intrinsically needs parentheses if it appears
  /// in the unquoted string argument of a `calc()` being embedded in another
  /// calculation.
  static bool _charNeedsParentheses(uint8_t character) {
    return Character::isWhitespace(character)
      || character == Character::$asterisk
      || character == Character::$slash;
  }


  /// Returns whether [text] needs parentheses if it's the contents of a
  /// `calc()` being embedded in another calculation.
  static bool _needsParentheses(const sass::string& text) {
    auto first = text[0]; // .codeUnitAt(0);
    if (_charNeedsParentheses(first)) return true;
    auto couldBeVar = text.size() >= 4 &&
      Character::characterEqualsIgnoreCase(first, Character::$v);

    if (text.size() < 2) return false;
    auto second = text[1]; // .codeUnitAt(1);
    if (_charNeedsParentheses(second)) return true;
    couldBeVar = couldBeVar && Character::characterEqualsIgnoreCase(second, Character::$a);

    if (text.size() < 3) return false;
    auto third = text[2]; // .codeUnitAt(2);
    if (_charNeedsParentheses(third)) return true;
    couldBeVar = couldBeVar && Character::characterEqualsIgnoreCase(third, Character::$r);

    if (text.size() < 4) return false;
    auto fourth = text[3]; // .codeUnitAt(3);
    if (couldBeVar && fourth == Character::$lparen) return true;
    if (_charNeedsParentheses(fourth)) return true;

    for (size_t i = 4; i < text.size(); i++) {
      if (_charNeedsParentheses(text[i]/*.codeUnitAt(i)*/)) return true;
    }
    return false;
  }


  AstNode* Calculation::simplify(Logger& logger)
  {
    if (name_ == str_calc) {
      if (arguments_.empty()) std::cerr << "arguments empty!!!\n";
      else if (arguments_.size() > 1) std::cerr << "too many args!!!\n";
      else {
        const AstNode* arg = arguments_[0];
        const String* str = dynamic_cast<const String*>(arg);
        if (str != nullptr && str->hasQuotes() == false) {
          if (_needsParentheses(str->value())) {
            sass::string quoted("(" + str->value() + ")");
            return new String(pstate_, std::move(quoted), false);
          }
        }
        return arguments_[0];
      }
    }
    // Or return ourself again
    return this;
  }

  bool Calculation::operator== (const Value& rhs) const
  {
    throw "not implemented Calc==";
    return false; // rhs.isNull();
  }

  Value* Calculation::plus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (auto str = other->isaString()) return Value::plus(other, logger, pstate);
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(logger, pstate,
      "Undefined operation \"" + toCss() + " + " + other->toCss() + "\".");
  }

  Value* Calculation::minus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (auto str = other->isaString()) return Value::minus(other, logger, pstate);
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(logger, pstate,
      "Undefined operation \"" + toCss() + " - " + other->toCss() + "\".");
  }

  // The SassScript unary `+` operation.
  Value* Calculation::unaryPlus(Logger& logger, const SourceSpan& pstate) const
  {
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(logger, pstate,
      "Undefined operation \"+" + toCss() + "\".");
  }

  // The SassScript unary `-` operation.
  Value* Calculation::unaryMinus(Logger& logger, const SourceSpan& pstate) const
  {
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(logger, pstate,
      "Undefined operation \"-" + toCss() + "\".");
  }

  size_t Calculation::hash() const
  {
    return typeid(Calculation).hash_code();
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
    const sass::string& disp,
    bool parsed) :
    Value(pstate),
    disp_(disp),
    a_(alpha),
    parsed_(parsed)
  {}

  Color::Color(const Color* ptr)
    : Value(ptr),
    // Reset on copy
    // disp_(ptr->disp_),
    a_(ptr->a_),
    parsed_(false) // safe to assume?
    // ptr->parsed_
  {}

  /////////////////////////////////////////////////////////////////////////
  // Implement value operators for color
  /////////////////////////////////////////////////////////////////////////

  Value* Color::plus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (other->isaNumber() || other->isaColor()) {
      callStackFrame csf(logger, pstate);
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
      callStackFrame csf(logger, pstate);
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
      callStackFrame csf(logger, pstate);
      throw Exception::SassScriptException(
        "Undefined operation \"" + inspect()
        + " / " + other->inspect() + "\".",
        logger, pstate);
    }
    return Value::dividedBy(other, logger, pstate);
  }

  Value* Color::modulo(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " % " + other->inspect() + "\".",
      logger, pstate);
  }

  Value* Color::remainder(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    callStackFrame csf(logger, pstate);
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
    const sass::string& disp,
    bool parsed) :
    Color(pstate, alpha, disp, parsed),
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
    const sass::string& disp,
    bool parsed) :
    Color(pstate, alpha, disp, parsed),
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
    const sass::string& disp,
    bool parsed) :
    Color(pstate, alpha, disp, parsed),
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
    auto col = SASS_MEMORY_COPY(this);
    col->parsed(false); // Do better
    return col;
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

  // Value constructor
  Number::Number(
    const SourceSpan& pstate,
    double value,
    Units units) :
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
      return NEAR_EQUAL_INF(value(), rhs.value());
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
      NEAR_EQUAL_INF(l.value(), r.value());
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
      callStackFrame csf(logger, pstate);
      throw Exception::UnitMismatch(
        logger, this, rhs);
    }
    callStackFrame csf(logger, pstate);
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
      callStackFrame csf(logger, pstate);
      throw Exception::UnitMismatch(
        logger, this, rhs);
    }
    callStackFrame csf(logger, pstate);
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
      callStackFrame csf(logger, pstate);
      throw Exception::UnitMismatch(
        logger, this, rhs);
    }
    callStackFrame csf(logger, pstate);
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
      callStackFrame csf(logger, pstate);
      throw Exception::UnitMismatch(
        logger, this, rhs);
    }
    callStackFrame csf(logger, pstate);
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

    // ToDo: move this special case to mod operator
//          else if (op == mod && std::isinf(rval) && std::isinf(copy->value())) {
//      copy->value(std::numeric_limits<double>::quiet_NaN());
//      }

    //if (std::isinf(x) && std::isinf(y)) {
    //  return NaN;
    //}

    // Always the case in dart sass
    if (std::isinf(x)) return NaN;
    // Next case is a bit complicated and not super well defined in Math
    if (std::isinf(y) && std::signbit(y) != std::signbit(x)) return NaN;

    if ((x > 0 && y < 0) || (x < 0 && y > 0)) {
      double ret = std::fmod(x, y);
      return ret ? ret + y : ret;
    }
    else {
      double ret = std::fmod(x, y);
      return ret;
    }
  }
  inline double rem(double x, double y)
  {
    if ((x > 0 && y < 0) || (x < 0 && y > 0)) {
      double ret = std::remainder(x, y);
      return ret ? ret + y : ret;
    }
    else {
      return std::remainder(x, y);
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
    /*if (op == mod && rval == 0) {
      return SASS_MEMORY_NEW(Number, pstate,
        std::numeric_limits<double>::quiet_NaN());
    }
    // Catch division by zero
    else */ if (op == div && rval == 0) {
      Units units(this); // Copy left units
      units.numerators.insert(units.numerators.end(),
        rhs.denominators.begin(), rhs.denominators.end());
      units.denominators.insert(units.denominators.end(),
        rhs.numerators.begin(), rhs.numerators.end());
      // Do a logical XOR to have one or the other side negative
      if (std::signbit(lval) != std::signbit(rval)) return SASS_MEMORY_NEW(
        Number, pstate, - std::numeric_limits<double>::infinity(), units);
      else if (lval != 0) return SASS_MEMORY_NEW(Number, pstate,
        std::numeric_limits<double>::infinity(), units);
      else return SASS_MEMORY_NEW(Number, pstate,
        std::numeric_limits<double>::quiet_NaN(), units);
    }

    // Simplest case with no units
    // Just operate on the values
    if (r_units == 0 && l_units <= 1) {
      Number* copy = SASS_MEMORY_COPY(this);
      copy->value(op(lval, rval));
      copy->pstate(pstate);
      return copy;
    }
    // Left hand has no unit, so we can just copy
    // the units from the right hand side. If units
    // are not compatible, op function will throw!
    if (l_units == 0 && r_units == 1) {
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
    // Both sides have exactly one unit
    // Most used case, so optimize it too!
    if (l_units == 1 && r_units == 1) {
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

    // std::cerr << "OPERATE " << inspect() << " " << rhs.inspect() << "\n";

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
      double f(right.getUnitConversionFactor(left));
      // Returns zero on incompatible units
      if (f == 0.0) {
        callStackFrame csf(logger, pstate);
        throw Exception::UnitMismatch(
          logger, left, right);
      }
      // Now apply the conversion factor
      copy->value(op(lval, right.value() * f));
    }

    copy->pstate(pstate);
    //std::cerr << "RESULT " << copy->inspect() << "\n";
    return copy.detach();
  }
  // EO operate

  Value* Number::plus(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(add, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::plus(other, logger, pstate);
    callStackFrame csf(logger, pstate);
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
    callStackFrame csf(logger, pstate);
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
    callStackFrame csf(logger, pstate);
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
    if (!other->isaColor()) return Value::modulo(other, logger, pstate);
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " % " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO modulo

  Value* Number::remainder(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      return operate(rem, *nr, logger, pstate);
    }
    if (!other->isaColor()) return Value::remainder(other, logger, pstate);
    callStackFrame csf(logger, pstate);
    throw Exception::SassScriptException(
      "Undefined operation \"" + inspect()
      + " %% " + other->inspect() + "\".",
      logger, pstate);
  }
  // EO remainder

  Value* Number::dividedBy(Value* other, Logger& logger, const SourceSpan& pstate) const
  {
    if (const Number* nr = other->isaNumber()) {
      if (!nr->hasUnits()) {
        double result = value();
        if (double divisor = nr->value()) {
          result /= divisor;
        }
        else {
          if ((result < 0) != std::signbit(divisor)) return SASS_MEMORY_NEW(
            Number, pstate, -std::numeric_limits<double>::infinity(), this);
          else if (result > 0) return SASS_MEMORY_NEW(Number, pstate,
            std::numeric_limits<double>::infinity(), this);
          else return SASS_MEMORY_NEW(Number, pstate,
            std::numeric_limits<double>::quiet_NaN(), this);
        }
        return SASS_MEMORY_NEW(Number,
          pstate, result, this);
      }
      else {
        return operate(div, *nr, logger, pstate);
      }
    }
    return Value::dividedBy(other, logger, pstate);
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
    callStackFrame csf(logger, span);
    throw Exception::SassScriptException(
      inspect() + " is not an int.",
      logger, span, name);
  }

  Number* Number::assertUnitless(Logger& logger, const sass::string& name)
  {
    if (!hasUnits()) return this;
    SourceSpan span(this->pstate());
    callStackFrame csf(logger, span);
    throw Exception::SassScriptException(
      "Expected " + inspect() + " to have no units.",
      logger, span, name);
  }

  Number* Number::assertHasUnits(Logger& logger, const sass::string& unit, const sass::string& name)
  {
    if (hasUnit(unit)) return this;
    SourceSpan span(this->pstate());
    callStackFrame csf(logger, span);
    throw Exception::SassScriptException(
      "Expected " + inspect() + " to have unit \"" + unit + "\".",
      logger, span, name);
  }

  Number* Number::assertNoUnits(Logger& logger, const sass::string& name)
  {
    if (numerators.empty() && denominators.empty()) return this;
    SourceSpan span(this->pstate());
    callStackFrame csf(logger, span);
    throw Exception::SassScriptException(
      "Expected " + inspect() + " to have no units.",
      logger, span, name);
  }

  double Number::assertRange(double min, double max, const Units& units, Logger& logger, const sass::string& name) const
  {
    if (!fuzzyCheckRange(value_, min, max, logger.epsilon)) {
      sass::sstream msg;
      msg << "Expected " << inspect() << " to be within "
        << min << units.unit() << " and "
        << max << units.unit() << ".";
      SourceSpan span(this->pstate());
      callStackFrame csf(logger, span);
      throw Exception::SassScriptException(
        msg.str(), logger, span, name);
    }
    return value_;
  }

  const Number* Number::checkPercent(Logger& logger, const sass::string& name) const
  {
    if (!hasUnit("%")) {
      sass::sstream msg;
      StringVector dif(numerators);
      StringVector mul(denominators);
      // ToDo: don't report percentage twice!?
      for (auto& unit : mul) unit = " * 1" + unit;
      for (auto& unit : dif) unit = " / 1" + unit;
      sass::string reunit(StringUtils::join(mul, "") + StringUtils::join(dif, ""));
      msg << "$" << name << ": Passing a number without unit % (" << inspect() << ") is deprecated." << STRMLF;
      msg << "To preserve current behavior: $" << name << reunit << " * 1%" << STRMLF;
      auto add = msg.str();
      logger.addDeprecation(add, pstate(), Logger::WARN_NUMBER_PERCENT);
    }
    return this;
  }

  Number* Number::coerce(Logger& logger, Number& lhs)
  {
    if (this->Units::operator==(lhs)) return this;
    double factor = getUnitConversionFactor(lhs);
    if (factor == 0.0) throw Exception::UnitMismatch(logger, lhs, *this);
    return SASS_MEMORY_NEW(Number, pstate(), value() * factor, lhs);
  }

  double Number::factorToUnits(const Units& units)
  {
    if (this->Units::operator==(units)) return 1;
    return getUnitConversionFactor(units);
  }

  /////////////////////////////////////////////////////////////////////////
  // Implement delayed value fetcher
  /////////////////////////////////////////////////////////////////////////

  // The original value may not be returned
  // Therefore make sure original is collected
  Value* Number::withoutSlash()
  {
    if (!hasAsSlash()) return this;
    // we are the only holder of this item
    // therefore should be safe to alter it
    if (this->refcount <= 1) {
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

  AstNode* String::simplify(Logger& logger) {
    if (hasQuotes_ == false) return this;
    callStackFrame csf(logger, pstate_);
    throw Exception::SassScriptException(logger, pstate_,
      "Quoted string " + inspect() + " can't be used in a calculation.");
  }

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

  bool String::isVar() const
  {
    return !hasQuotes_ && value_.size() > 7 &&
      StringUtils::startsWithIgnoreCase(value_, "var(", 4);
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
    sass::string text(value() + other->toCss());
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
      itpair->set(0, kv->first);
      itpair->set(1, kv->second);
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

  Function::Function(
    const SourceSpan& pstate,
    const sass::string& cssName) :
    Value(pstate),
    cssName_(cssName)
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

  CalcOperation::CalcOperation(
    const SourceSpan& pstate,
    const SassOperator op,
    AstNode* left,
    AstNode* right) :
    Value(pstate),
    op_(op),
    left_(left),
    right_(right)
  {}

  CalcOperation::CalcOperation(
    const CalcOperation * ptr) :
    Value(ptr),
    op_(ptr->op()),
    left_(ptr->left()),
    right_(ptr->right())
  {
  }

  size_t CalcOperation::hash() const
  {
    return 123;
  }

  bool CalcOperation::operator==(const Value& rhs) const
  {
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Mixin::Mixin(
    const SourceSpan& pstate,
    Callable* callable) :
    Value(pstate),
    callable_(callable)
  {}

  Mixin::Mixin(
    const Mixin * ptr) :
    Value(ptr),
    callable_(ptr->callable())
  {}

  size_t Mixin::hash() const
  {
    return callable_->hash();
  }

  bool Mixin::operator==(const Value& rhs) const
  {
    if (const Mixin* mixin = rhs.isaMixin()) {
      return *this == *mixin;
    }
    return false;
  }

  bool Mixin::operator== (const Mixin& rhs) const
  {
    return ObjEqualityFn(callable_, rhs.callable());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
