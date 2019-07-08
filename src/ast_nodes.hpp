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

  //////////////////////////////////////////////////////////////////////
  // define cast template now (need complete type)
  //////////////////////////////////////////////////////////////////////

  template<class T>
  T* Cast(AstNode* ptr) {
    return dynamic_cast<T*>(ptr);
  };

  template<class T>
  const T* Cast(const AstNode* ptr) {
    return dynamic_cast<const T*>(ptr);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  uint8_t sass_op_to_precedence(enum SassOperator op);
  const char* sass_op_to_name(enum SassOperator op);
  const char* sass_op_separator(enum SassOperator op);

  typedef LocalStack<EnvFrame*> ScopedStack;
  typedef LocalStack<ImportObj> ScopedImport;
  typedef LocalStack<SelectorListObj> ScopedSelector;

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for all abstract syntax tree nodes.
  /////////////////////////////////////////////////////////////////////////
  class AstNode : public SharedObj
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
    // virtual bool operator==(const AstNode& rhs) const = delete;
    // virtual bool operator!=(const AstNode& rhs) const = delete;
    // virtual bool operator>=(const AstNode& rhs) const = delete;
    // virtual bool operator<=(const AstNode& rhs) const = delete;
    // virtual bool operator>(const AstNode& rhs) const = delete;
    // virtual bool operator<(const AstNode& rhs) const = delete;

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
    virtual const sass::string& getText() const { return Strings::empty; };

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
    const sass::string& getText() const override final { return text_; }

    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(ItplString);
  };
  // EO ItplString

  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  class Interpolation final : public AstNode,
    public Vectorized<Interpolant>
  {
  public:

    // Value constructor
    Interpolation(const SourceSpan& pstate,
      Interpolant* interpolant = nullptr);

    const sass::string& getPlainString() const;
    const sass::string& getInitialPlain() const;

    StringExpression* wrapInStringExpression();

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

    // Needed here to avoid ambiguity from base-classes!??
    virtual Value* accept(ExpressionVisitor<Value*>* visitor) override = 0;
    // virtual void accept(ExpressionVisitor<void>* visitor) override = 0;

    // Implementation for parent Interpolant interface
    Type getType() const override final { return ExpressionInterpolant; }

    // Declare up-casting methods
    DECLARE_ISA_CASTER(VariableExpression);
    DECLARE_ISA_CASTER(FunctionExpression);
    DECLARE_ISA_CASTER(NumberExpression);
    // Implement our up-casting
    IMPLEMENT_ISA_CASTER(Expression);
  };

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for statements. This side of the AST hierarchy
  // represents elements in expansion contexts, which exist primarily to be
  // rewritten and macro-expanded.
  /////////////////////////////////////////////////////////////////////////
  class Statement : public AstNode,
    public StatementVisitable<Value*>
  {
  private:

    ADD_CONSTREF(size_t, tabs)

  public:

    // Value copy constructor
    Statement(const SourceSpan& pstate)
      : AstNode(pstate)
    {}

    // Value move constructor
    Statement(SourceSpan&& pstate)
      : AstNode(std::move(pstate))
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

    // Declare up-casting methods
    DECLARE_ISA_CASTER(StyleRule);
  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class ImportBase : public AstNode
  {
  public:
    ImportBase(const SourceSpan& pstate);
    ImportBase(const ImportBase* ptr);
    // Declare up-casting methods
    DECLARE_ISA_CASTER(StaticImport);
    DECLARE_ISA_CASTER(IncludeImport);
  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class StaticImport final : public ImportBase
  {
  private:

    // The URL for this import.
    // This already contains quotes.
    ADD_CONSTREF(InterpolationObj, url);

    // The supports condition attached to this import,
    // or `null` if no condition is attached.
    ADD_CONSTREF(SupportsConditionObj, supports);

    // The media query attached to this import,
    // or `null` if no condition is attached.
    ADD_CONSTREF(InterpolationObj, media);

    // Flag to hoist import to the top.
    ADD_CONSTREF(bool, outOfOrder);

  public:

    // Object constructor by values
    StaticImport(const SourceSpan& pstate,
      InterpolationObj url,
      SupportsConditionObj supports = {},
      InterpolationObj media = {});
    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(StaticImport);
  };
  // EO class StaticImport

  // Dynamic import beside its name must have a static url
  // We do not support to load sass partials programmatic
  // They also don't allow any supports or media queries.
  class IncludeImport final : public ImportBase
  {
  private:

    ADD_CONSTREF(StyleSheetObj, sheet);

  public:

    IncludeImport(const SourceSpan& pstate, StyleSheet* sheet);
    // Implement final up-casting method
    IMPLEMENT_ISA_CASTER(IncludeImport);
  };

  //////////////////////////////////////////////////////////////////////
  // Helper class to iterator over different Value types.
  // Depending on the type of the Value (e.g. List vs String),
  // we either want to iterate over a container or a single value.
  // In order to avoid unnecessary copies, we use this iterator.
  //////////////////////////////////////////////////////////////////////
  class Values final {

  public:

    // We know four types
    enum Type {
      MapIterator,
      ListIterator,
      SingleIterator,
      NullPtrIterator,
    };

    // Internal iterator helper class
    class iterator
    {

    private:

      // The value we are iterating over
      Value* val;
      // The detected value/iterator type
      Type type;
      // The final item to iterator to
      // For null ptr this is zero, for
      // single items this is 1 and for
      // lists/maps the container size.
      size_t last;
      // The current iteration item
      size_t cur;

    public:

      // Some typedefs to satisfy C++ type traits
      typedef std::input_iterator_tag iterator_category;
      typedef iterator self_type;
      typedef Value* value_type;
      typedef Value* reference;
      typedef Value* pointer;
      typedef int difference_type;

      // Create iterator for start (or end)
      iterator(Value* val, bool end);

      // Copy constructor
      iterator(const iterator& it) :
        val(it.val),
        type(it.type),
        last(it.last),
        cur(it.cur) {}

      // Dereference current item
      reference operator*();

      // Move to the next item
      iterator& operator++();

      // Compare operators
      bool operator==(const iterator& other) const;
      bool operator!=(const iterator& other) const;

    };
    // EO class iterator

    // The value we are iterating over
    Value* val;
    // The detected value/iterator type
    Type type;

    // Standard value constructor
    Values(Value* val) : val(val), type(SingleIterator) {}

    // Get iterator at the given position
    iterator begin() { return iterator(val, false); }
    iterator end() { return iterator(val, true); }

  };
  // EO class Values

  //////////////////////////////////////////////////////////////////////
  // base class for values that support operations
  //////////////////////////////////////////////////////////////////////
  class Value : public Interpolant,
    public ValueVisitable<void>,
    public ValueVisitable<Value*> {

  public:

    // Needed here to avoid ambiguity from base-classes!??
    virtual void accept(ValueVisitor<void>* visitor) override = 0;
    virtual Value* accept(ValueVisitor<Value*>* visitor) override = 0;
    sass::string inspect(int precision = SassDefaultPrecision, bool quotes = true) const;

    // Getters to avoid need for dynamic cast (slightly faster)
    Type getType() const override final { return ValueInterpolant; }

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

    // Get the type in string format (for output)
    virtual const sass::string& type() const = 0;

    // Get a values iterator
    virtual Values iterator() {
      return Values{ this };
    }

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
    virtual String* assertStringOrNull(Logger& logger, const sass::string& name);

    // Assert and return an argument list or throws if incompatible
    virtual ArgumentList* assertArgumentList(Logger& logger, const sass::string& name);

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
    sass::string toCss(Logger& logger, bool quote = true) const;

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

    // Declare up-casting methods
    DECLARE_ISA_CASTER(Map);
    DECLARE_ISA_CASTER(List);
    DECLARE_ISA_CASTER(Number);
    DECLARE_ISA_CASTER(Color);
    DECLARE_ISA_CASTER(Boolean);
    DECLARE_ISA_CASTER(Function);
    DECLARE_ISA_CASTER(CustomError);
    DECLARE_ISA_CASTER(CustomWarning);
    DECLARE_ISA_CASTER(ArgumentList);
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

    // Returns the at-rule name for [node], or `null` if it's not an at-rule.
    sass::string _nameFor(CssNode* node) const;

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

  class Root final : public AstNode,
    public Vectorized<Statement>
  {
  public:

    Root(const SourceSpan& pstate, size_t reserve = 0)
      : AstNode(pstate), Vectorized<Statement>(reserve)
    {}

    Root(const SourceSpan& pstate, StatementVector&& vec)
      : AstNode(pstate), Vectorized<Statement>(std::move(vec))
    {}

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
