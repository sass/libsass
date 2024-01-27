/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_AST_NODES_HPP
#define SASS_AST_NODES_HPP

#include "backtrace.hpp"
#include "source_span.hpp"
#include "ast_containers.hpp"
#include "visitor_value.hpp"
#include "visitor_statement.hpp"
#include "visitor_expression.hpp"
#include "environment_key.hpp"
#include "environment_cnt.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Some helpers in regard to sass value operations.
  /////////////////////////////////////////////////////////////////////////

  uint8_t sass_op_to_precedence(enum SassOperator op);
  const char* sass_op_to_name(enum SassOperator op);
  const char* sass_op_separator(enum SassOperator op);
  const char* sass_list_separator(enum SassSeparator op);

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for all abstract syntax tree nodes.
  /////////////////////////////////////////////////////////////////////////

  class AstNode : public RefCounted
  {
  private:

    ADD_CONSTREF(SourceSpan, pstate);

  public:

    // Value copy constructor
    AstNode(const SourceSpan& pstate)
      : pstate_(pstate)
    {}

    // Value move constructor
    AstNode(SourceSpan&& pstate)
      : pstate_(std::move(pstate))
    {}

    // Copy constructor
    AstNode(const AstNode* ptr)
      : pstate_(ptr->pstate_)
    {}

    // Delete compare operators to make implementation more clear
    // Helps us spot cases where we use undefined implementations
    virtual bool operator==(const AstNode& rhs) const = delete;
    virtual bool operator!=(const AstNode& rhs) const = delete;
    virtual bool operator>=(const AstNode& rhs) const = delete;
    virtual bool operator<=(const AstNode& rhs) const = delete;
    virtual bool operator>(const AstNode& rhs) const = delete;
    virtual bool operator<(const AstNode& rhs) const = delete;

    // Crutches to implement calculation
		virtual AstNode* simplify(Logger& logger);

    // Convert to string (only for debugging)
    sass::string toString() const;

    DECLARE_ISA_CASTER(Value);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CalcItem {

  };

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for items in interpolations.
  // Must be one of ItplString, an Expression or a Value.
  /////////////////////////////////////////////////////////////////////////

  class Interpolant : public AstNode
  {
  public:

    // Value constructors
    Interpolant(SourceSpan&& pstate);
    Interpolant(const SourceSpan& pstate);

    // We know four types
    enum Type {
      ValueInterpolant,
      LiteralInterpolant,
      ExpressionInterpolant,
    };

    // Interface to be implemented
    virtual Type getType() const = 0;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(Value);
    DECLARE_ISA_CASTER(String);
    DECLARE_ISA_CASTER(ItplString);
    DECLARE_ISA_CASTER(Expression);
  };
  // EO Interpolant

  ///////////////////////////////////////////////////////////////////////
  // A native string wrapped as an interpolant
  ///////////////////////////////////////////////////////////////////////
  class ItplString final : public Interpolant
  {
  private:

    ADD_CONSTREF(sass::string, text)

  public:

    ItplString(const SourceSpan& pstate, sass::string&& text);
    ItplString(const SourceSpan& pstate, const sass::string& text);
    Type getType() const override final { return LiteralInterpolant; }

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(ItplString);
  };
  // EO ItplString

  ///////////////////////////////////////////////////////////////////////
  // Interpolation class holding a list of interpolants.
  ///////////////////////////////////////////////////////////////////////

  class Interpolation final : public AstNode,
    public Vectorized<Interpolant>
  {
  public:

    // Value constructor
    Interpolation(const SourceSpan& pstate,
      Interpolant* interpolant = nullptr);

    // // If this contains no interpolated expressions, returns its text contents.
    const sass::string& getPlainString() const;

    // Returns the plain text before the interpolation, or the empty string.
    const sass::string& getInitialPlain() const;

    // Wrap interpolation within a string expression
    StringExpression* wrapInStringExpression();

    // Convert to string (only for debugging)
    sass::string toString() const;

  };
  // EO Interpolation

  //////////////////////////////////////////////////////////////////////
  // Abstract base class for expressions. This side of the AST
  // hierarchy represents elements in value contexts, which
  // exist primarily to be evaluated and returned.
  //////////////////////////////////////////////////////////////////////

  class Expression : public Interpolant,
    public ExpressionVisitable<Value*>
  {
  public:

    // Value constructor
    Expression(SourceSpan&& pstate);

    // C++ does not consider return type for function overloading
    // Therefore we need to differentiate by the function name
    // Basically the same as `ExpressionVisitable<bool>`
    virtual bool isCalcSafe() = 0;

    // Needed here to avoid ambiguity from base-classes (issue seems gone)!??
    // virtual Value* accept(ExpressionVisitor<Value*>* visitor) override = 0;

    // Implementation for parent Interpolant interface
    Type getType() const override final { return ExpressionInterpolant; }

    virtual sass::string toString() const = 0;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(UnaryOpExpression);
    DECLARE_ISA_CASTER(BinaryOpExpression);
    DECLARE_ISA_CASTER(InvocationExpression);
    DECLARE_ISA_CASTER(ParenthesizedExpression);
    DECLARE_ISA_CASTER(FunctionExpression);
    DECLARE_ISA_CASTER(VariableExpression);
    DECLARE_ISA_CASTER(BooleanExpression);
    DECLARE_ISA_CASTER(StringExpression);
    DECLARE_ISA_CASTER(SupportsExpression);
    DECLARE_ISA_CASTER(NumberExpression);
    DECLARE_ISA_CASTER(ColorExpression);
    DECLARE_ISA_CASTER(ValueExpression);
    DECLARE_ISA_CASTER(NullExpression);
    DECLARE_ISA_CASTER(ListExpression);
    DECLARE_ISA_CASTER(MapExpression);
    DECLARE_ISA_CASTER(IfExpression);
    // Implement our up-casting
    IMPLEMENT_ISA_CASTER(Expression);
  };
  // EO Expression

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for statements. This side of the AST hierarchy
  // represents elements in expansion contexts, which exist primarily to be
  // rewritten and macro-expanded.
  /////////////////////////////////////////////////////////////////////////
  class Statement : public AstNode,
    public StatementVisitable<Value*>,
    public StatementVisitable<void>
  {
  private:

    ADD_CONSTREF(size_t, tabs)

  public:

    // Value copy constructor
    Statement(const SourceSpan& pstate) :
      AstNode(pstate),
      tabs_(0)
    {}

    // Value move constructor
    Statement(SourceSpan&& pstate) :
      AstNode(std::move(pstate)),
      tabs_(0)
    {}

    // Copy constructor
    Statement(const Statement* ptr) :
      AstNode(ptr),
      tabs_(ptr->tabs_)
    {}

    // Interface to be implemented by content rule
    virtual bool hasContent() const { return false; }

    // Needed here to avoid ambiguity from base-classes!??
    // virtual void accept(ExpressionVisitor<void>* visitor) override = 0;
    virtual Value* accept(StatementVisitor<Value*>* visitor) override = 0;
    virtual void accept(StatementVisitor<void>* visitor) override = 0;

    // Declare up-casting methods
    DECLARE_ISA_CASTER(StyleRule);
  };

  //////////////////////////////////////////////////////////////////////
  // Base class for all imports
  //////////////////////////////////////////////////////////////////////

  class ImportBase : public AstNode
  {
  public:
    // Value constructor
    ImportBase(const SourceSpan& pstate);
    // Copy constructor
    ImportBase(const ImportBase* ptr);
    // Declare up-casting methods
    DECLARE_ISA_CASTER(StaticImport);
    DECLARE_ISA_CASTER(IncludeImport);
  };

  //////////////////////////////////////////////////////////////////////
  // Helper class to iterator over different Value types.
  // Depending on the type of the Value (e.g. List vs String),
  // we either want to iterate over a container or a single value.
  // In order to avoid unnecessary copies, we use this iterator.
  //////////////////////////////////////////////////////////////////////

  class Iterator final
  {
  public:

    // We know four iterator types
    enum ItType {
      MapIterator,
      ListIterator,
      SingleIterator,
      NullPtrIterator,
    };

  private:

    // The value we are iterating over
    Value* val;
    // The detected value/iterator type
    ItType type;
    // The final item to iterate to
    // For null ptr this is zero, for
    // single items this is 1 and for
    // lists/maps the container size.
    size_t last;
    // The current iteration item
    size_t cur;

  public:

    // Some typedefs to satisfy C++ type traits
    typedef std::input_iterator_tag iterator_category;
    typedef Iterator self_type;
    typedef Value* value_type;
    typedef Value* reference;
    typedef Value* pointer;
    typedef int difference_type;

    // Create iterator for start (or end)
    Iterator(Value* val, bool end);

    // Copy constructor
    Iterator(const Iterator& it) :
      val(it.val),
      type(it.type),
      last(it.last),
      cur(it.cur) {}

    // Dereference current item
    reference operator*();
    reference operator->();

    // Move to the next item
    Iterator& operator++();
    Iterator& operator+=(size_t offset);
    Iterator& operator-=(size_t offset);
    Iterator operator-(size_t offset);

    // Check if it's the last item
    bool isLast() const;

    // Compare operators
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

    // Get iterators to support regular C++ loops
    const Iterator& begin() const { return *this; }
    Iterator end() const { return Iterator(val, true); }

  };
  // EO class Iterator

  //////////////////////////////////////////////////////////////////////
  // base class for values that support operations
  //////////////////////////////////////////////////////////////////////

  class Value : public Interpolant,
    public ValueVisitable<void>,
    public ValueVisitable<Value*>
  {
  public:

    // Needed here to avoid ambiguity from base-classes!??
    virtual void accept(ValueVisitor<void>* visitor) override = 0;
    virtual Value* accept(ValueVisitor<Value*>* visitor) override = 0;
    sass::string inspect(int precision = SassDefaultPrecision, bool quotes = true) const;

    // Getters to avoid need for dynamic cast (slightly faster)
    Type getType() const override final { return ValueInterpolant; }

    virtual AstNode* simplify(Logger& logger) override;

  protected:

    // Hash is only calculated once and afterwards the value
    // must not be mutated, which is the case with how sass
    // works, although we must be a bit careful not to alter
    // any value that has already been added to a set or map.
    // Must create a copy if you need to alter such an object.
    mutable size_t hash_;

  public:

    // Default implementation does nothing
    virtual Value* cloneChildren(SASS_MEMORY_ARGS_VOID) { return this; }

  public:

    // Standard value constructor
    Value(const SourceSpan& pstate);

    // Copy constructor
    Value(const Value* ptr);

    // Use macro to allow location debugging
    virtual Value* copy(SASS_MEMORY_ARGS bool childless = false) const {
      throw std::runtime_error("Copy not implemented");
    }

  public:

    // Hash value when used as key in hash table
    virtual size_t hash() const = 0;

    // Interface to be implemented by our classes
    virtual enum SassValueType getTag() const = 0;

    // Whether the value will be represented in CSS as the empty string.
    virtual bool isBlank() const { return false; }

    // Return the length of this item as a list
    virtual size_t lengthAsList() const { return 1; }

    // Get iterators for values. We couldn't use begin and end
    // since list and map already define these methods.
    virtual Iterator start() { return Iterator(this, false); }
    virtual Iterator stop() { return Iterator(this, true); }

    // Get the type in string format (for output)
    virtual const sass::string& type() const = 0;

    // Search the position of the given value
    virtual size_t indexOf(Value* value) {
      return *this == *value ? 0 : sass::string::npos;
    }

    // Return the list separator
    virtual SassSeparator separator() const {
      return SASS_UNDEF;
    }

    // Check if we has comma separator
    bool hasCommaSeparator() const {
      return separator() == SASS_COMMA;
    }

    // Check if we has space separator
    bool hasSpaceSeparator() const {
      return separator() == SASS_SPACE;
    }

    // Check if we has space separator
    bool hasSlashSeparator() const {
      return separator() == SASS_DIV;
    }

    // Check if we are bracketed
    virtual bool hasBrackets() {
      return false;
    }

    // Check if it evaluates to true
    virtual bool isTruthy() const {
      return true;
    }

    // Check if it is null
    virtual bool isNull() const {
      return false;
    }

    // Reset delayed value
    virtual Value* withoutSlash() {
      return this;
    }

    // The SassScript `==` operation (never throws).
    virtual bool operator== (const Value& rhs) const = 0;

    // The SassScript `>` operation.
    virtual bool greaterThan(Value* other, Logger& logger, const SourceSpan& pstate) const;

    // The SassScript `>=` operation.
    virtual bool greaterThanOrEquals(Value* other, Logger& logger, const SourceSpan& pstate) const;

    // The SassScript `<` operation.
    virtual bool lessThan(Value* other, Logger& logger, const SourceSpan& pstate) const;

    // The SassScript `<=` operation.
    virtual bool lessThanOrEquals(Value* other, Logger& logger, const SourceSpan& pstate) const;

    // The SassScript `*` operation.
    virtual Value* times(Value* other, Logger& logger, const SourceSpan& pstate) const;

    // The SassScript `%` operation.
    virtual Value* modulo(Value* other, Logger& logger, const SourceSpan& pstate) const;

    // The SassScript `rem` operation.
    virtual Value* remainder(Value* other, Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript `=` operation.
    virtual Value* singleEquals(Value* other, Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript `+` operation.
    virtual Value* plus(Value* other, Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript `-` operation.
    virtual Value* minus(Value* other, Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript `/` operation.
    virtual Value* dividedBy(Value* other, Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript unary `+` operation.
    virtual Value* unaryPlus(Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript unary `-` operation.
    virtual Value* unaryMinus(Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript unary `/` operation.
    virtual Value* unaryDivide(Logger& logger, const SourceSpan& pstate) const;

    /// The SassScript unary `not` operation.
    virtual Value* unaryNot(Logger& logger, const SourceSpan& pstate) const;

    // Assert and return a value or throws if incompatible
    virtual Value* assertValue(Logger& logger, const sass::string& name);

    // Assert and return a color or throws if incompatible
    virtual const Color* assertColor(Logger& logger, const sass::string& name) const;

    // Assert and return a function or throws if incompatible
    virtual Function* assertFunction(Logger& logger, const sass::string& name);

    // Assert and return a map or throws if incompatible
    virtual Map* assertMap(Logger& logger, const sass::string& name);

    // Assert and return a number or throws if incompatible
    virtual Number* assertNumber(Logger& logger, const sass::string& name);

    // Assert and return a number/nullptr or throws if incompatible
    virtual Number* assertNumberOrNull(Logger& logger, const sass::string& name);

    // Assert and return a string or throws if incompatible
    virtual String* assertString(Logger& logger, const sass::string& name);

    // Assert and return a string/nullptr or throws if incompatible
    String* assertStringOrNull(Logger& logger, const sass::string& name);

    // Assert and return a string/nullptr or throws if incompatible
    Map* assertMapOrNull(Logger& logger, const sass::string& name);

    // Assert and return an argument list or throws if incompatible
    virtual ArgumentList* assertArgumentList(Logger& logger, const sass::string& name);

    // Assert and return a calculation value or throws if incompatible
    virtual Calculation* assertCalculation(Logger& logger, const sass::string& name);

    // Assert and return a mixin value or throws if incompatible
    virtual Mixin* assertMixin(Logger& logger, const sass::string& name);

    // Only used for nth sass function
    // Single values act like lists with 1 item
    // Doesn't allow overflow of index (throw error)
    // Allows negative index but no overflow either
    virtual Value* getValueAt(Value* index, Logger& logger);

    // Return normalized index for vector from overflowable sass index
    size_t sassIndexToListIndex(Value* sassIndex, Logger& logger, const sass::string& name);

    /// Parses [this] as a selector list, in the same manner as the
    /// `selector-parse()` function.
    ///
    /// Throws a [SassScriptException] if this isn't a type that can be parsed as a
    /// selector, or if parsing fails. If [allowParent] is `true`, this allows
    /// [ParentSelector]s. Otherwise, they're considered parse errors.
    ///
    /// If this came from a function argument, [name] is the argument name
    /// (without the `$`). It's used for error reporting.
    SelectorList* assertSelector(Compiler& ctx, const sass::string& name = Strings::empty, bool allowParent = false) const;

    /// Parses [this] as a compound selector, in the same manner as the
    /// `selector-parse()` function.
    ///
    /// Throws a [SassScriptException] if this isn't a type that can be parsed as a
    /// selector, or if parsing fails. If [allowParent] is `true`, this allows
    /// [ParentSelector]s. Otherwise, they're considered parse errors.
    ///
    /// If this came from a function argument, [name] is the argument name
    /// (without the `$`). It's used for error reporting.
    CompoundSelector* assertCompoundSelector(Compiler& ctx, const sass::string& name = Strings::empty, bool allowParent = false) const;

    /// Returns a valid CSS representation of [this].
    ///
    /// Throws a [SassScriptException] if [this] can't be represented in plain
    /// CSS. Use [toString] instead to get a string representation even if this
    /// isn't valid CSS.
    ///
    /// If [quote] is `false`, quoted strings are emitted without quotes.
    sass::string toCss(bool quote = true) const;

  private:

    // Converts a `selector-parse()`-style input into a string that
    // can be parsed. Returns `false` if [this] isn't a type or a
    // structure that can be parsed as a selector.
    bool selectorStringOrNull(Logger& logger, sass::string& rv) const;

    // Converts a `selector-parse()`-style input into a string that
    // can be parsed. Throws a [SassScriptException] if [this] isn't
    // a type or a structure that can be parsed as a selector.
    sass::string getSelectorString(Logger& logger, const sass::string& name = Strings::empty) const;

  public:

    // Declare further up-casting methods
    DECLARE_ISA_CASTER(Map);
    DECLARE_ISA_CASTER(List);
    DECLARE_ISA_CASTER(Null);
    DECLARE_ISA_CASTER(Number);
    DECLARE_ISA_CASTER(Color);
    DECLARE_ISA_CASTER(ColorRgba);
    DECLARE_ISA_CASTER(ColorHsla);
    DECLARE_ISA_CASTER(ColorHwba);
    DECLARE_ISA_CASTER(Boolean);
    DECLARE_ISA_CASTER(Function);
    DECLARE_ISA_CASTER(CustomError);
    DECLARE_ISA_CASTER(CustomWarning);
    DECLARE_ISA_CASTER(ArgumentList);
    DECLARE_ISA_CASTER(Calculation);
    DECLARE_ISA_CASTER(CalcOperation);
    DECLARE_ISA_CASTER(Mixin);
    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(Value);
    // Expose class as SassValue struct to C
    CAPI_WRAPPER(Value, SassValue);
  };


  /////////////////////////////////////////////////////////////////////////
  // A query for the `@at-root` rule.
  /////////////////////////////////////////////////////////////////////////

  class AtRootQuery final : public AstNode
  {
  private:

    // The names of the rules included or excluded by this query. There are 
    // two special names. "all" indicates that all rules are included or
    // excluded, and "rule" indicates style rules are included or excluded.
    ADD_CONSTREF(StringSet, names);
    // Whether the query includes or excludes rules with the specified names.
    ADD_CONSTREF(bool, include);

  public:

    // Value constructor
    AtRootQuery(
      SourceSpan&& pstate,
      StringSet&& names,
      bool include);

    // Whether this includes or excludes *all* rules.
    bool all() const;

    // Whether this includes or excludes style rules.
    bool rule() const;

    // Whether this includes or excludes media rules.
    bool media() const;

    // Returns whether [this] excludes a node with the given [name].
    bool excludesName(const sass::string& name) const;

    // Returns whether [this] excludes [node].
    bool excludes(CssParentNode* node) const;

    // Whether this excludes `@media` rules.
    // Note that this takes [include] into account.
    bool excludesMedia() const;

    // Whether this excludes style rules.
    // Note that this takes [include] into account.
    bool excludesStyleRules() const;

    // Parses an at-root query from [contents]. If passed, [url]
    // is the name of the file from which [contents] comes.
    // Throws a [SassFormatException] if parsing fails.
    static AtRootQuery* parse(
      SourceData* contents, Compiler& ctx);

    // The default at-root query, which excludes only style rules.
    static AtRootQuery* defaultQuery(SourceSpan&& pstate);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
