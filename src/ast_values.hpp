/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_VALUES_HPP
#define SASS_AST_VALUES_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "units.hpp"
#include "ast_nodes.hpp"
#include "ast_callables.hpp"

namespace Sass {

  class Compiler;

  /////////////////////////////////////////////////////////////////////////
  // Errors from Sass_Values.
  /////////////////////////////////////////////////////////////////////////
  class CustomError final : public Value
  {
  private:

    ADD_CONSTREF(sass::string, message)

  public:

    // Value constructor
    CustomError(
      const SourceSpan& pstate,
      const sass::string& message);

    // Copy constructor
    CustomError(const CustomError* ptr);

    // Implement interface for base Value class
    size_t hash() const override final { return 0; }
    enum SassValueType getTag() const override final { return SASS_ERROR; }
    const sass::string& getText() const override final { return message_; }
    const sass::string& type() const override final { return Strings::error; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const CustomError& rhs) const;

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final;
    Value* accept(ValueVisitor<Value*>* visitor) override final;

    // Copy operations for childless items
    CustomError* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CustomError, this);
    }

    IMPLEMENT_ISA_CASTER(CustomError);
  };

  /////////////////////////////////////////////////////////////////////////
  // Warnings from Sass_Values.
  /////////////////////////////////////////////////////////////////////////
  class CustomWarning final : public Value
  {
  private:

    ADD_CONSTREF(sass::string, message)

  public:

    // Value constructor
    CustomWarning(
      const SourceSpan& pstate,
      const sass::string& message);

    // Copy constructor
    CustomWarning(const CustomWarning* ptr);

    // Implement interface for base Value class
    size_t hash() const override final { return 0; }
    enum SassValueType getTag() const override final { return SASS_WARNING; }
    const sass::string& getText() const override final { return message_; }
    const sass::string& type() const override final { return Strings::warning; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const CustomWarning& rhs) const;

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final;
    Value* accept(ValueVisitor<Value*>* visitor) override final;

    // Copy operations for childless items
    CustomWarning* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CustomWarning, this);
    }

  };

  ///////////////////////////////////////////////////////////////////////
  // The null value.
  ///////////////////////////////////////////////////////////////////////
  class Null final : public Value
  {
  public:

    // Value constructor
    Null(const SourceSpan& pstate);

    // Copy constructor
    Null(const Null* ptr);

    // Implement simple checkers for base value class
    bool isNull() const override final { return true; }
    bool isBlank() const override final { return true; }
    bool isTruthy() const override final { return false; }

    // Implement interface for base Value class
    size_t hash() const override final;

    enum SassValueType getTag() const override final { return SASS_NULL; }
    const sass::string& type() const override final { return Strings::null; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitNull(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitNull(this);
    }

    // Copy operations for childless items
    Null* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(Null, this);
    }
  };

  ///////////////////////////////////////////////////////////////////////
  // Base class for colors (either rgba or hsla).
  ///////////////////////////////////////////////////////////////////////
  class Color : public Value
  {
  private:

    ADD_CONSTREF(sass::string, disp);
    ADD_CONSTREF(double, a);

  public:

    // Value constructor
    Color(const SourceSpan& pstate,
      double alpha = 1,
      const sass::string& disp = Strings::empty);

    // Copy constructor
    Color(const Color* ptr);

    // Convert and copy only if necessary
    virtual ColorRgba* toRGBA() const = 0;
    virtual ColorHsla* toHSLA() const = 0;
    virtual ColorHwba* toHWBA() const = 0;
    // Convert if necessary and return a copy
    virtual ColorRgba* copyAsRGBA() const = 0;
    virtual ColorHsla* copyAsHSLA() const = 0;
    virtual ColorHwba* copyAsHWBA() const = 0;

    // Implement interface for base Value class
    virtual size_t hash() const override = 0;
    enum SassValueType getTag() const override final { return SASS_COLOR; }
    const sass::string& type() const override final { return Strings::color; }

    // Implement some operations for base value class
    Value* plus(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* minus(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* dividedBy(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* modulo(Value* other, Logger& logger, const SourceSpan& pstate) const override final;

    // Implement type fetcher for base value class (throws in base implementation)
    const Color* assertColor(Logger& logger, const sass::string& name = Strings::empty) const override final { return this; }

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitColor(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitColor(this);
    }

    // This is a very interesting line, as it seems pointless, since the base class
    // already marks this as an unimplemented interface methods, but by defining this
    // line here, we make sure that callers know the return is a bit more specific.
    virtual Color* copy(SASS_MEMORY_ARGS bool childless = false) const override = 0;

    IMPLEMENT_ISA_CASTER(Color);
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass color in RGBA representation.
  ///////////////////////////////////////////////////////////////////////
  class ColorRgba final : public Color
  {
  private:

    ADD_CONSTREF(double, r);
    ADD_CONSTREF(double, g);
    ADD_CONSTREF(double, b);

  public:

    // Value constructor
    ColorRgba(const SourceSpan& pstate,
      double red, double green, double blue, double alpha = 1.0,
      const sass::string& disp = Strings::empty);

    // Copy constructor
    ColorRgba(const ColorRgba* ptr);

    // Convert and copy only if necessary
    ColorRgba* toRGBA() const override final;
    ColorHsla* toHSLA() const override final;
    ColorHwba* toHWBA() const override final;
    // Convert if necessary and return a copy
    ColorRgba* copyAsRGBA() const override final;
    ColorHsla* copyAsHSLA() const override final;
    ColorHwba* copyAsHWBA() const override final;

    // Implement interface for base color class
    size_t hash() const override final;

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const ColorRgba& rhs) const;

    // Copy operations for childless items
    ColorRgba* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(ColorRgba, this);
    }
  };


  ///////////////////////////////////////////////////////////////////////
  // A sass color in HSLA representation.
  ///////////////////////////////////////////////////////////////////////
  class ColorHsla final : public Color
  {
  private:

    ADD_CONSTREF(double, h);
    ADD_CONSTREF(double, s);
    ADD_CONSTREF(double, l);

  public:

    // Value constructor
    ColorHsla(const SourceSpan& pstate,
      double hue, double saturation, double lightness, double alpha = 1,
      const sass::string& disp = Strings::empty);

    // Copy constructor
    ColorHsla(const ColorHsla* ptr);

    // Convert and copy only if necessary
    ColorRgba* toRGBA() const override final;
    ColorHsla* toHSLA() const override final;
    ColorHwba* toHWBA() const override final;
    // Convert if necessary and return a copy
    ColorRgba* copyAsRGBA() const override final;
    ColorHsla* copyAsHSLA() const override final;
    ColorHwba* copyAsHWBA() const override final;

    // Implement interface for base color class
    size_t hash() const override final;

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const ColorHsla& rhs) const;

    // Copy operations for childless items
    ColorHsla* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(ColorHsla, this);
    }
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass color in HSLA representation.
  ///////////////////////////////////////////////////////////////////////
  class ColorHwba final : public Color
  {
  private:

    ADD_CONSTREF(double, h);
    ADD_CONSTREF(double, w);
    ADD_CONSTREF(double, b);

  public:

    // Value constructor
    ColorHwba(const SourceSpan& pstate,
      double hue, double whiteness, double blackness, double alpha = 1,
      const sass::string& disp = Strings::empty);

    // Copy constructor
    ColorHwba(const ColorHwba* ptr);

    // Convert and copy only if necessary
    ColorRgba* toRGBA() const override final;
    ColorHsla* toHSLA() const override final;
    ColorHwba* toHWBA() const override final;
    // Convert if necessary and return a copy
    ColorRgba* copyAsRGBA() const override final;
    ColorHsla* copyAsHSLA() const override final;
    ColorHwba* copyAsHWBA() const override final;

    // Implement interface for base color class
    size_t hash() const override final;

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const ColorHwba& rhs) const;

    // Copy operations for childless items
    ColorHwba* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(ColorHwba, this);
    }
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass number with optional units
  ///////////////////////////////////////////////////////////////////////

  class Number final : public Value, public Units
  {
  private:

    ADD_CONSTREF(double, value);
    // The representation of this number as two
    // slash-separated numbers, if it has one.
    ADD_CONSTREF(NumberObj, lhsAsSlash);
    ADD_CONSTREF(NumberObj, rhsAsSlash);

  public:

    // Value constructor
    Number(
      const SourceSpan& pstate,
      double value = 0.0,
      const sass::string& units = "");

    // Copy constructor
    Number(const Number* ptr);

    // Check if we have delayed value info
    bool hasAsSlash() {
      return !lhsAsSlash_.isNull()
        && !rhsAsSlash_.isNull();
    }

    // Check if number matches [unit]
    bool hasUnit(const sass::string& unit) const {
      return numerators.size() == 1 &&
        denominators.empty() &&
        numerators.front() == unit;
    }

    // cancel out unnecessary units
    // result will be in input units
    void reduce()
    {
      // apply conversion factor
      value_ *= this->Units::reduce();
    }

    // normalize units to defaults
    // needed to compare two numbers
    void normalize()
    {
      // apply conversion factor
      value_ *= this->Units::normalize();
    }

    // Implement delayed value fetcher
    Value* withoutSlash() override final;

    // Implement interface for base Value class
    size_t hash() const override final;
    enum SassValueType getTag() const override final { return SASS_NUMBER; }
    const sass::string& type() const override final { return Strings::number; }

    // Implement some comparators for base value class
    bool greaterThan(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    bool greaterThanOrEquals(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    bool lessThan(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    bool lessThanOrEquals(Value* other, Logger& logger, const SourceSpan& pstate) const override final;

    // Implement some operations for base value class
    Value* plus(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* minus(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* times(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* modulo(Value* other, Logger& logger, const SourceSpan& pstate) const override final;
    Value* dividedBy(Value* other, Logger& logger, const SourceSpan& pstate) const override final;

    // Implement unary operations for base value class
    Value* unaryPlus(Logger& logger, const SourceSpan& pstate) const override final;
    Value* unaryMinus(Logger& logger, const SourceSpan& pstate) const override final;

    // Implement type fetcher for base value class (throws in base implementation)
    Number* assertNumber(Logger& logger, const sass::string& name = Strings::empty) override final { return this; }

    // Implement number specific assertions
    long assertInt(Logger& logger, const sass::string& name = Strings::empty);
    Number* assertUnitless(Logger& logger, const sass::string& name = Strings::empty);
    Number* assertHasUnits(Logger& logger, const sass::string& unit, const sass::string& name = Strings::empty);
    double assertRange(double min, double max, Logger& logger, const sass::string& name = Strings::empty) const;

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const Number& rhs) const;

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitNumber(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitNumber(this);
    }

    // Copy operations for childless items
    Number* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(Number, this);
    }

  private:

    Value* operate(double (*op)(double, double), const Number& rhs, Logger& logger, const SourceSpan& pstate) const;

    IMPLEMENT_ISA_CASTER(Number);
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass boolean (either true or false)
  ///////////////////////////////////////////////////////////////////////

  class Boolean final : public Value
  {
  private:

    ADD_CONSTREF(bool, value)

  public:

    // Value constructor
    Boolean(
      const SourceSpan& pstate,
      bool value = false);

    // Copy constructor
    Boolean(const Boolean* ptr);

    // Implement simple checkers for base value class
    bool isTruthy() const override final { return value_; }

    // Implement interface for base Value class
    size_t hash() const override final;
    enum SassValueType getTag() const override final { return SASS_BOOLEAN; }
    const sass::string& type() const override final { return Strings::boolean; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const Boolean& rhs) const;

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitBoolean(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitBoolean(this);
    }

    // Copy operations for childless items
    Boolean* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(Boolean, this);
    }

    IMPLEMENT_ISA_CASTER(Boolean);
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass string (optionally quoted on rendering)
  ///////////////////////////////////////////////////////////////////////
  class String final : public Value
  {
  private:

    ADD_CONSTREF(sass::string, value);
    ADD_CONSTREF(bool, hasQuotes);

  public:

    // Value constructor
    String(
      const SourceSpan& pstate,
      const char* value,
      bool hasQuotes = false);

    String(
      const SourceSpan& pstate,
      sass::string&& value,
      bool hasQuotes = false);

    // Copy constructor
    String(const String* ptr);

    // Check if value would render empty
    bool isBlank() const override final {
      if (hasQuotes_) return false;
      return value_.empty();
    }

    // Implement interface for base Value class
    size_t hash() const override final;
    enum SassValueType getTag() const override final { return SASS_STRING; }
    const sass::string& getText() const override final { return value_; }
    const sass::string& type() const override { return Strings::string; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const String& rhs) const;

    // Implement type fetcher for base value class (throws in base implementation)
    String* assertString(Logger& logger, const sass::string& name = Strings::empty) override final { return this; }

    // Implement some operations for base value class
    Value* plus(Value* other, Logger& logger, const SourceSpan& pstate) const override final;

    // Main entry point for Value Visitor pattern
    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitString(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitString(this);
    }

    // Copy operations for childless items
    String* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(String, this);
    }

    IMPLEMENT_ISA_CASTER(String);
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass map (which keeps the insertion order)
  ///////////////////////////////////////////////////////////////////////
  class Map final : public Value, public Hashed<ValueObj, ValueObj>
  {
  private:

    // Helper for getPairAsList to avoid memory leaks
    // Returned by `Values::iterator::operator*()`
    ListObj itpair;

  public:

    // Value constructor
    Map(
      const SourceSpan& pstate,
      Hashed::ordered_map_type&& move = {});

    // Copy constructor
    Map(const Map* ptr);

    // Return the list separator
    SassSeparator separator() const override final {
      return empty() ? SASS_UNDEF : SASS_COMMA;
    }

    // Return the length of this item as a list
    size_t lengthAsList() const override {
      return size();
    }

    // Search the position of the given value
    size_t indexOf(Value* value) override final;

    // Return list with two items (key and value)
    Value* getPairAsList(size_t idx);

    // Only used for nth sass function
    // Doesn't allow overflow of index (throw error)
    // Allows negative index but no overflow either
    Value* getValueAt(Value* index, Logger& logger) override final;

    // Implement interface for base Value class
    size_t hash() const override final;
    enum SassValueType getTag() const override final { return SASS_MAP; }
    const sass::string& type() const override final { return Strings::map; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const Map& rhs) const;

    // Implement type fetcher for base value class (throws in base implementation)
    Map* assertMap(Logger& logger, const sass::string& name) override { return this; }

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitMap(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitMap(this);
    }
    // Copy operations for childless items
    Map* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(Map, this);
    }

  protected:

    // Clone all items in-place
    Map* cloneChildren(SASS_MEMORY_ARGS_VOID) override final {
      for (auto it = begin(); it != end(); ++it) {
        it.value() = it.value()->copy(SASS_MEMORY_PARAMS_VOID);
        it.value()->cloneChildren(SASS_MEMORY_PARAMS_VOID);
      }
      return this;
    }

    IMPLEMENT_ISA_CASTER(Map);
  };

  ///////////////////////////////////////////////////////////////////////
  // Lists of values, both comma- and space-separated (distinguished by a
  // type-tag.) Also used to represent variable-length argument lists.
  ///////////////////////////////////////////////////////////////////////

  class List : public Value, public Vectorized<Value>
  {
  private:

    enum SassSeparator separator_;
    ADD_CONSTREF(bool, hasBrackets);

  public:

    // Value constructor
    List(const SourceSpan& pstate,
      const ValueVector& values = {},
      enum SassSeparator separator = SASS_SPACE,
      bool hasBrackets = false);

    // Value constructor
    List(const SourceSpan& pstate,
      ValueVector&& values,
      enum SassSeparator separator = SASS_SPACE,
      bool hasBrackets = false);

    // Copy constructor
    List(const List* ptr);

    // Return the list separator
    SassSeparator separator() const override final {
      return separator_;
    }

    // Set the list separator
    void separator(SassSeparator separator) {
      separator_ = separator;
    }

    // Return the length of this item as a list
    size_t lengthAsList() const override final {
      return size();
    }

    // Check if list has surrounding brackets
    bool hasBrackets() override final {
      return hasBrackets_;
    }

    // Check if value would render empty
    bool isBlank() const override final {
      for (const Value* value : elements()) {
        if (!value->isBlank()) return false;
      }
      return true;
    }

    // Search the position of the given value
    size_t indexOf(Value* value) override final;

    // Only used for nth sass function
    // Allows negative index but no overflow either
    // Doesn't allow overflow of index (throw error)
    Value* getValueAt(Value* index, Logger& logger) override final;

    // Implement interface for base Value class
    virtual size_t hash() const override;
    enum SassValueType getTag() const override final { return SASS_LIST; }
    virtual const sass::string& type() const override { return Strings::list; }

    // Implement equality comparators for base value class
    virtual bool operator== (const Value& rhs) const override;
    // Implement same class compare operator
    bool operator== (const List& rhs) const;

    // Implement type fetcher for base value class (throws in base implementation)
    Map* assertMap(Logger& logger, const sass::string& name) override final;

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitList(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitList(this);
    }
    // Copy operations for childless items
    List* copy(SASS_MEMORY_ARGS bool childless) const override {
      return SASS_MEMORY_NEW_DBG(List, this);
    }

  protected:

    // Clone all items in-place
    List* cloneChildren(SASS_MEMORY_ARGS_VOID) override {
      for (ValueObj& entry : elements_) {
        entry = entry->copy(SASS_MEMORY_PARAMS_VOID);
        entry->cloneChildren(SASS_MEMORY_PARAMS_VOID);
      }
      return this;
    }

    IMPLEMENT_ISA_CASTER(List);
  };

  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  class ArgumentList final : public List
  {
  private:

    ValueFlatMap _keywords;
    mutable bool _wereKeywordsAccessed;

  public:

    // Value copy constructor
    ArgumentList(const SourceSpan& pstate,
      SassSeparator sep = SASS_SPACE,
      ValueVector&& values = {},
      ValueFlatMap&& keywords = {});

    // Value move constructor
    ArgumentList(const SourceSpan& pstate,
      SassSeparator sep = SASS_SPACE,
      const ValueVector& values = {},
      const ValueFlatMap& keywords = {});

    // Copy constructor
    ArgumentList(const ArgumentList* ptr);

    ValueFlatMap& keywords() {
      _wereKeywordsAccessed = true;
      return _keywords;
    }

    bool wereKeywordsAccessed() const {
      return _wereKeywordsAccessed;
    }

    bool hasAllKeywordsConsumed() const {
      return _keywords.empty() ||
        _wereKeywordsAccessed;
    }

    Map* keywordsAsSassMap() const;

    // Implement interface for base Value class
    size_t hash() const override final;
    const sass::string& type() const override final { return Strings::arglist; }

    ArgumentList* assertArgumentList(Logger& logger, const sass::string& name = Strings::empty) override final {
      return this;
    }


    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const ArgumentList& rhs) const;

    // Copy operations for childless items
    ArgumentList* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(ArgumentList, this);
    }

  protected:

    // Clone all items in-place
    ArgumentList* cloneChildren(SASS_MEMORY_ARGS_VOID) override final {
      for (auto it : _keywords) {
        it.second = it.second->copy(SASS_MEMORY_PARAMS_VOID);
        it.second->cloneChildren(SASS_MEMORY_PARAMS_VOID);
      }
      return this;
    }

    IMPLEMENT_ISA_CASTER(ArgumentList);
  };

  ///////////////////////////////////////////////////////////////////////
  // A sass function reference.
  ///////////////////////////////////////////////////////////////////////
  class Function final : public Value
  {
  private:

    ADD_CONSTREF(CallableObj, callable);

  public:

    // Value constructor
    Function(
      const SourceSpan& pstate,
      CallableObj callable);

    // Copy constructor
    Function(const Function* ptr);

    // Implement interface for base Value class
    size_t hash() const override final { return 0; }
    enum SassValueType getTag() const override final { return SASS_FUNCTION; }
    const sass::string& type() const override final { return Strings::function; }

    // Implement equality comparators for base value class
    bool operator== (const Value& rhs) const override final;
    // Implement same class compare operator
    bool operator== (const Function& rhs) const;

    Function* assertFunction(Logger& logger, const sass::string& name = Strings::empty) override final { return this; }

    // Main entry point for Value Visitor pattern
    void accept(ValueVisitor<void>* visitor) override final {
      return visitor->visitFunction(this);
    }
    Value* accept(ValueVisitor<Value*>* visitor) override final {
      return visitor->visitFunction(this);
    }
    // Copy operations for childless items
    Function* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(Function, this);
    }

    IMPLEMENT_ISA_CASTER(Function);
  };

  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

}

#endif
