#ifndef SASS_AST_H
#define SASS_AST_H

// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include <vector>
#include <typeinfo>
#include <unordered_map>

#include "sass/base.h"
#include "ast_helpers.hpp"
#include "ast_fwd_decl.hpp"
#include "ast_def_macros.hpp"
#include "ordered_map.hpp"

#include "file.hpp"
#include "memory.hpp"
#include "mapping.hpp"
#include "position.hpp"
#include "operation.hpp"
#include "environment.hpp"
#include "fn_utils.hpp"
#include "environment_stack.hpp"

#include "ordered-map/ordered_map.h"

namespace Sass {

  uint8_t sass_op_to_precedence(enum Sass_OP op);

  const char* sass_op_to_name(enum Sass_OP op);

  const char* sass_op_separator(enum Sass_OP op);

  //////////////////////////////////////////////////////////
  // Abstract base class for all abstract syntax tree nodes.
  //////////////////////////////////////////////////////////
  class AST_Node : public SharedObj {
    ADD_PROPERTY(SourceSpan, pstate);
  public:
    AST_Node(const SourceSpan& pstate)
    : SharedObj(), pstate_(pstate)
    { }
    AST_Node(SourceSpan&& pstate)
      : SharedObj(), pstate_(std::move(pstate))
    { }
    AST_Node(const AST_Node* ptr)
    : pstate_(ptr->pstate_)
    { }

    // void* operator new(size_t nbytes) {
    //   return ::operator new(nbytes);
    // }
    // 
    // void operator delete(void* ptr) {
    //   return ::operator delete(ptr);
    // }

    // allow implicit conversion to string
    // needed for by SharedPtr implementation
    operator sass::string() {
      return to_string();
    }

    // AST_Node(AST_Node& ptr) = delete;

    virtual ~AST_Node() = 0;
    virtual size_t hash() const { return 0; }
    virtual sass::string inspect() const { return to_string({ INSPECT, 5 }); }
    virtual sass::string to_sass() const { return to_string({ TO_SASS, 5 }); }
    virtual sass::string to_string(Sass_Inspect_Options opt) const;
    virtual sass::string to_css(Sass_Inspect_Options opt, bool quotes = false) const;
    virtual sass::string to_css(Sass_Inspect_Options opt, sass::vector<Mapping>& mappings, bool quotes = false) const;
    virtual sass::string to_string() const;
    virtual sass::string to_css(sass::vector<Mapping>& mappings, bool quotes = false) const;
    virtual sass::string to_css(bool quotes = false) const;
    virtual void cloneChildren() {};
    // generic find function (not fully implemented yet)
    // ToDo: add specific implementions to all children
    virtual bool find ( bool (*f)(AST_Node_Obj) ) { return f(this); };
    // Offset off() { return pstate(); }
    // Position pos() { return pstate(); }

    // Subclasses should only override these methods
    // The full set is emulated by calling only those
    // Make sure the left side is resonably upcasted!
    virtual bool operator< (const AST_Node& rhs) const {
      throw std::runtime_error("operator< not implemented");
    }
    virtual bool operator== (const AST_Node& rhs) const {
      throw std::runtime_error("operator== not implemented");
    }


    // We can give some reasonable implementations by using
    // inverst operators on the specialized implementations
    virtual bool operator>(const AST_Node& rhs) const { return rhs < *this; };
    virtual bool operator<=(const AST_Node& rhs) const { return !(rhs < *this); };
    virtual bool operator>=(const AST_Node& rhs) const { return !(*this < rhs); };
    virtual bool operator!=(const AST_Node& rhs) const { return !(*this == rhs); }

    ATTACH_ABSTRACT_COPY_OPERATIONS(AST_Node);
    ATTACH_ABSTRACT_CRTP_PERFORM_METHODS()
  };
  inline AST_Node::~AST_Node() { }

  //////////////////////////////////////////////////////////////////////
  // define cast template now (need complete type)
  //////////////////////////////////////////////////////////////////////

  template<class T>
  T* Cast(AST_Node* ptr) {
    return ptr && typeid(T) == typeid(*ptr) ?
           static_cast<T*>(ptr) : NULL;
  };

  template<class T>
  const T* Cast(const AST_Node* ptr) {
    return ptr && typeid(T) == typeid(*ptr) ?
           static_cast<const T*>(ptr) : NULL;
  };

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class SassNode : public AST_Node {
  public:
    SassNode(const SourceSpan& pstate) :
      AST_Node(pstate) {};
    SassNode(SourceSpan&& pstate) :
      AST_Node(std::move(pstate)) {};
    ATTACH_VIRTUAL_COPY_OPERATIONS(SassNode);
    ATTACH_CRTP_PERFORM_METHODS();
  };

  class CallableInvocation {
    // The arguments passed to the callable.
    ADD_PROPERTY(ArgumentInvocationObj, arguments);
  public:
    CallableInvocation(
      ArgumentInvocation* arguments) :
      arguments_(arguments) {}
  };


  class ArgumentDeclaration : public SassNode {

    // The arguments that are taken.
    ADD_CONSTREF(sass::vector<ArgumentObj>, arguments);

    // The name of the rest argument (as in `$args...`),
    // or `null` if none was declared.
    ADD_CONSTREF(sass::string, restArg);

  public:

    ArgumentDeclaration(
      const SourceSpan& pstate,
      const sass::vector<ArgumentObj>& arguments,
      const sass::string& restArg = "");

    bool isEmpty() const {
      return arguments_.empty()
        && restArg_.empty();
    }

    static ArgumentDeclaration* parse(
      Context& context,
      const sass::string& contents);

    void verify(
      size_t positional,
      const KeywordMap<ValueObj>& names,
      const SourceSpan& pstate,
      const Backtraces& traces);

    bool matches(
      size_t positional,
      const KeywordMap<ValueObj>& names);

    sass::string toString2() const;

  };


  /// The result of evaluating arguments to a function or mixin.
  class ArgumentResults2 {

    // Arguments passed by position.
    ADD_REF(sass::vector<ValueObj>, positional);

    // The [AstNode]s that hold the spans for each [positional]
    // argument, or `null` if source span tracking is disabled. This
    // stores [AstNode]s rather than [FileSpan]s so it can avoid
    // calling [AstNode.span] if the span isn't required, since
    // some nodes need to do real work to manufacture a source span.
    // sass::vector<Ast_Node_Obj> positionalNodes;

    // Arguments passed by name.
    // A list implementation might be more efficient
    // I dont expect any function to have many arguments
    // Normally the tradeoff is around 8 items in the list
    ADD_REF(KeywordMap<ValueObj>, named);

    // The [AstNode]s that hold the spans for each [named] argument,
    // or `null` if source span tracking is disabled. This stores
    // [AstNode]s rather than [FileSpan]s so it can avoid calling
    // [AstNode.span] if the span isn't required, since some nodes
    // need to do real work to manufacture a source span.
    // KeywordMap<Ast_Node_Obj> namedNodes;

    // The separator used for the rest argument list, if any.
    ADD_REF(Sass_Separator, separator);

  public:

    ArgumentResults2() {};

    ArgumentResults2(
      const ArgumentResults2& other);

    ArgumentResults2(
      ArgumentResults2&& other);

    ArgumentResults2& operator=(
      ArgumentResults2&& other);

    ArgumentResults2(
      const sass::vector<ValueObj>& positional,
      const KeywordMap<ValueObj>& named,
      Sass_Separator separator);

    ArgumentResults2(
      sass::vector<ValueObj> && positional,
      KeywordMap<ValueObj>&& named,
      Sass_Separator separator);

  };


  class ArgumentInvocation : public SassNode {

    // The arguments passed by position.
    ADD_REF(sass::vector<ExpressionObj>, positional);
  public:
    // 
    ArgumentResults2 evaluated;

    // The arguments passed by name.
    ADD_CONSTREF(KeywordMap<ExpressionObj>, named);

    // The first rest argument (as in `$args...`).
    ADD_PROPERTY(ExpressionObj, restArg);

    // The second rest argument, which is expected to only contain a keyword map.
    ADD_PROPERTY(ExpressionObj, kwdRest);

  public:

    ArgumentInvocation(const SourceSpan& pstate,
      const sass::vector<ExpressionObj>& positional,
      const KeywordMap<ExpressionObj>& named,
      Expression* restArgs = nullptr,
      Expression* kwdRest = nullptr);

    // Returns whether this invocation passes no arguments.
    bool isEmpty() const;

    sass::string toString() const;

    ATTACH_CRTP_PERFORM_METHODS();

  };

  //////////////////////////////////////////////////////////////////////
  // Abstract base class for expressions. This side of the AST hierarchy
  // represents elements in value contexts, which exist primarily to be
  // evaluated and returned.
  //////////////////////////////////////////////////////////////////////
  class Expression : public SassNode {
  public:
    enum Type {
      NONE,
      BOOLEAN,
      NUMBER,
      COLOR,
      STRING,
      LIST,
      MAP,
      NULL_VAL,
      FUNCTION_VAL,
      C_WARNING,
      C_ERROR,
      FUNCTION,
      VARIABLE,
      PARENT,
      NUM_TYPES
    };
  private:
    // expressions in some contexts shouldn't be evaluated
    ADD_PROPERTY(Type, concrete_type)
  public:
    Expression(SourceSpan&& pstate, bool d = false, bool e = false, bool i = false, Type ct = NONE);
    Expression(const SourceSpan& pstate, bool d = false, bool e = false, bool i = false, Type ct = NONE);
    virtual operator bool() { return true; }
    virtual ~Expression() { }
    virtual bool is_invisible() const { return false; }

    virtual const sass::string& type() const {
      throw std::runtime_error("Invalid reflection");
    }

    virtual Expression* withoutSlash() {
      return this;
    }

    virtual Expression* removeSlash() {
      return this;
    }

    virtual bool is_false() { return false; }
    // virtual bool is_true() { return !is_false(); }
    virtual bool operator== (const Expression& rhs) const { return false; }
    inline bool operator!=(const Expression& rhs) const { return !(rhs == *this); }
    ATTACH_VIRTUAL_COPY_OPERATIONS(Expression);
    size_t hash() const override { return 0; }
  };

}

/////////////////////////////////////////////////////////////////////////////////////
// Hash method specializations for std::unordered_map to work with Sass::Expression
/////////////////////////////////////////////////////////////////////////////////////

namespace std {
  template<>
  struct hash<Sass::Expression_Obj>
  {
    size_t operator()(Sass::Expression_Obj s) const
    {
      return s->hash();
    }
  };
  template<>
  struct equal_to<Sass::Expression_Obj>
  {
    bool operator()( Sass::Expression_Obj lhs,  Sass::Expression_Obj rhs) const
    {
      return lhs->hash() == rhs->hash();
    }
  };
}

namespace Sass {

  /////////////////////////////////////////////////////////////////////////////
  // Mixin class for AST nodes that should behave like vectors. Uses the
  // "Template Method" design pattern to allow subclasses to adjust their flags
  // when certain objects are pushed.
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class Vectorized {
    sass::vector<T> elements_;
  protected:
    mutable size_t hash_;
    void reset_hash() { hash_ = 0; }
  public:
    Vectorized(size_t s = 0) : hash_(0)
    { elements_.reserve(s); }
    Vectorized(const Vectorized<T>* vec) :
      elements_(vec->elements_),
      hash_(0)
    {}
    Vectorized(sass::vector<T> vec) :
      elements_(std::move(vec)),
      hash_(0)
    {}
    ~Vectorized() {};
    size_t length() const   { return elements_.size(); }
    bool empty() const      { return elements_.empty(); }
    void clear()            { return elements_.clear(); }
    T& last()               { return elements_.back(); }
    T& first()              { return elements_.front(); }
    const T& last() const   { return elements_.back(); }
    const T& first() const  { return elements_.front(); }

    bool operator== (const Vectorized<T>& rhs) const {
      // Abort early if sizes do not match
      if (length() != rhs.length()) return false;
      // Otherwise test each node for object equalicy in order
      return std::equal(begin(), end(), rhs.begin(), ObjEqualityFn<T>);
    }

    bool operator!= (const Vectorized<T>& rhs) const {
      return !(*this == rhs);
    }

    T& operator[](size_t i) { return elements_[i]; }
    virtual const T& at(size_t i) const { return elements_.at(i); }
    virtual T& at(size_t i) { return elements_.at(i); }
    const T& get(size_t i) const { return elements_[i]; }
    // ToDo: might insert am item (update ordered list)
    const T& operator[](size_t i) const { return elements_[i]; }

    // Implicitly get the std::vector from our object
    // Makes the Vector directly assignable to std::vector
    // You are responsible to make a copy if needed
    // Note: since this returns the real object, we can't
    // Note: guarantee that the hash will not get out of sync
    operator sass::vector<T>&() { return elements_; }
    operator const sass::vector<T>&() const { return elements_; }

    // Explicitly request all elements as a real std::vector
    // You are responsible to make a copy if needed
    // Note: since this returns the real object, we can't
    // Note: guarantee that the hash will not get out of sync
    sass::vector<T>& elements() { return elements_; }
    const sass::vector<T>& elements() const { return elements_; }

    // Insert all items from compatible vector
    void concat(const sass::vector<T>& v)
    {
      if (!v.empty()) reset_hash();
      elements().insert(end(), v.begin(), v.end());
    }

    // Insert all items from compatible vector
    void concat(sass::vector<T>&& v)
    {
      if (!v.empty()) reset_hash();
      std::move(v.begin(), v.end(),
        std::back_inserter(elements_));
      // elements().insert(end(), v.begin(), v.end());
    }


    // Syntatic sugar for pointers
    void concat(const Vectorized<T>* v)
    {
      if (v != nullptr) {
        return concat(*v);
      }
    }

    // Insert one item on the front
    void unshift(T element)
    {
      reset_hash();
      elements_.insert(begin(), element);
    }

    // Remove and return item on the front
    // ToDo: handle empty vectors
    T shift() {
      reset_hash();
      T first = get(0);
      elements_.erase(begin());
      return first;
    }

    // Insert one item on the back
    // ToDo: rename this to push
    void append(T element)
    {
      if (!element) {
        std::cerr << "APPEND NULL PTR\n";
      }
      reset_hash();
      elements_.emplace_back(element);
    }

    // Check if an item already exists
    // Uses underlying object `operator==`
    // E.g. compares the actual objects
    bool contains(const T& el) const {
      for (const T& rhs : elements_) {
        // Test the underlying objects for equality
        // A std::find checks for pointer equality
        if (ObjEqualityFn(el, rhs)) {
          return true;
        }
      }
      return false;
    }

    // This might be better implemented as `operator=`?
    void elements(sass::vector<T> e) {
      reset_hash();
      elements_ = std::move(e);
    }

    virtual size_t hash() const
    {
      if (hash_ == 0) {
        for (const T& el : elements_) {
          hash_combine(hash_, el->hash());
        }
      }
      return hash_;
    }

    template <typename P, typename V>
    typename sass::vector<T>::iterator insert(P position, const V& val) {
      reset_hash();
      return elements_.insert(position, val);
    }

    typename sass::vector<T>::iterator end() { return elements_.end(); }
    typename sass::vector<T>::iterator begin() { return elements_.begin(); }
    typename sass::vector<T>::const_iterator end() const { return elements_.end(); }
    typename sass::vector<T>::const_iterator begin() const { return elements_.begin(); }
    typename sass::vector<T>::iterator erase(typename sass::vector<T>::iterator el) { reset_hash(); return elements_.erase(el); }
    typename sass::vector<T>::const_iterator erase(typename sass::vector<T>::const_iterator el) { reset_hash(); return elements_.erase(el); }

  };

  /////////////////////////////////////////////////////////////////////////////
  // Mixin class for AST nodes that should behave like a hash table. Uses an
  // extra <std::vector> internally to maintain insertion order for interation.
  /////////////////////////////////////////////////////////////////////////////
  template <typename K, typename T>
  class Hashed {
    

    using ordered_map_type = typename tsl::ordered_map<
      K, T, ObjHash, ObjEquality,
      SassAllocator<std::pair<K, T>>,
      sass::vector<std::pair<K, T>>
    >;

  protected:

    ordered_map_type elements_;

    mutable size_t hash_;

    void reset_hash() { hash_ = 0; }

  public:

    Hashed()
    : elements_(),
      hash_(0)
    {
      // elements_.reserve(s);
    }

    // Copy constructor
    Hashed(const Hashed<K, T>& copy) :
      elements_(), hash_(0)
    {
      // this seems to expensive!?
      // elements_.reserve(copy.size());
      elements_ = copy.elements_;
    };

    // Move constructor
    Hashed(Hashed<K, T>&& move) :
      elements_(std::move(move.elements_)),
      hash_(move.hash_) {};

    virtual ~Hashed();

    size_t size() const { return elements_.size(); }
    bool empty() const { return elements_.empty(); }

    bool has(K k) const          {
      return elements_.count(k) == 1;
    }

    void reserve(size_t size)
    {
      elements_.reserve(size);
    }

    T at(K k) const {
      auto it = elements_.find(k);
      if (it == elements_.end()) return {};
      else return it->second;
    }

    bool erase(K key)
    {
      reset_hash();
      return elements_.erase(key);
    }

    void set(std::pair<K, T>& kv)
    {
      reset_hash();
      elements_[kv.first] = kv.second;
    }

    void insert(K key, T val)
    {
      reset_hash();
      elements_[key] = val;
    }

    void concat(Hashed<K, T> arr)
    {
      reset_hash();
      for (const auto& kv : arr) {
        elements_[kv.first] = kv.second;
      }
      // elements_.append(arr.elements());
    }

    // Return unmodifiable reference
    const ordered_map_type& elements() const {
      return elements_;
    }

    const sass::vector<K> keys() const {
      sass::vector<T> list;
      for (auto kv : elements_) {
        list.emplace_back(kv.first);
      }
      return list;
    }
    const sass::vector<T> values() const {
      sass::vector<T> list;
      for (auto kv : elements_) {
        list.emplace_back(kv.second);
      }
      return list;
    }
    
    typename ordered_map_type::iterator end() { return elements_.end(); }
    typename ordered_map_type::iterator begin() { return elements_.begin(); }
    typename ordered_map_type::const_iterator end() const { return elements_.end(); }
    typename ordered_map_type::const_iterator begin() const { return elements_.begin(); }

  };

  template <typename K, typename T>
  inline Hashed<K, T>::~Hashed() { }

  /////////////////////////////////////////////////////////////////////////
  // Abstract base class for statements. This side of the AST hierarchy
  // represents elements in expansion contexts, which exist primarily to be
  // rewritten and macro-expanded.
  /////////////////////////////////////////////////////////////////////////
  class Statement : public AST_Node {
  private:
    ADD_PROPERTY(size_t, tabs)
    ADD_PROPERTY(bool, group_end)
  public:
    Statement(SourceSpan&& pstate, size_t t = 0);
    Statement(const SourceSpan& pstate, size_t t = 0);
    virtual ~Statement() = 0; // virtual destructor
    // needed for rearranging nested rulesets during CSS emission
    virtual bool bubbles() const;
    virtual bool has_content();
    virtual bool is_invisible() const;
    ATTACH_VIRTUAL_COPY_OPERATIONS(Statement)
  };
  inline Statement::~Statement() { }

  ////////////////////////
  // Blocks of statements.
  ////////////////////////
  class Block final : public Statement, public Vectorized<Statement_Obj> {
    ADD_POINTER(IDXS*, idxs);
    ADD_PROPERTY(bool, is_root);
    // needed for properly formatted CSS emission
  public:
    Block(const SourceSpan& pstate, size_t s = 0, bool r = false);
    Block(const SourceSpan& pstate, const sass::vector<StatementObj>& vec, bool r = false);
    Block(const SourceSpan& pstate, sass::vector<StatementObj>&& vec, bool r = false);
    bool isInvisible() const;
    bool is_invisible() const override {
      return isInvisible();
    }
    bool has_content() override;
    // ATTACH_CLONE_OPERATIONS(Block)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ////////////////////////////////////////////////////////////////////////
  // Abstract base class for statements that contain blocks of statements.
  ////////////////////////////////////////////////////////////////////////

  // [X] AtRootRule
  // [X] AtRule
  // [X] CallableDeclaration
  // [X] Declaration
  // [X] EachRule
  // [X] ForRule
  // [X] MediaRule
  // [X] StyleRule
  // [ ] Stylesheet
  // [X] SupportsRule
  // [X] WhileRule
  class ParentStatement : public Statement {
    ADD_PROPERTY(Block_Obj, block)
  public:
    void concat(const sass::vector<StatementObj>& vec) {
      if (block_.ptr() == nullptr) {
        block_ = SASS_MEMORY_NEW(Block, pstate_);
      }
      block_->concat(vec);
    }
    void concat(sass::vector<StatementObj>&& vec) {
      if (block_.ptr() == nullptr) {
        block_ = SASS_MEMORY_NEW(Block, pstate_);
      }
      block_->concat(std::move(vec));
    }
    ParentStatement(SourceSpan&& pstate, Block_Obj b);
    ParentStatement(const SourceSpan& pstate, Block_Obj b);
    ParentStatement(const ParentStatement* ptr); // copy constructor
    virtual ~ParentStatement() = 0; // virtual destructor
    virtual bool has_content() override;
  };
  inline ParentStatement::~ParentStatement() { }

  /////////////////////////////////////////////////////////////////////////////
  // A style rule. This applies style declarations to elements 
  // that match a given selector. Formerly known as `Ruleset`.
  /////////////////////////////////////////////////////////////////////////////
  class StyleRule final : public ParentStatement {
    // The selector to which the declaration will be applied.
    // This is only parsed after the interpolation has been resolved.
    ADD_PROPERTY(InterpolationObj, interpolation);
    ADD_POINTER(IDXS*, idxs);
  public:
    StyleRule(SourceSpan&& pstate, Interpolation* s, Block_Obj b = {});
    bool empty() const { return block().isNull() || block()->empty(); }
    // ATTACH_CLONE_OPERATIONS(StyleRule)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  // ToDo: ParentStatement
  ///////////////////////////////////////////////////////////////////////
  // At-rules -- arbitrary directives beginning with "@" that may have an
  // optional statement block.
  ///////////////////////////////////////////////////////////////////////
  class AtRule final : public ParentStatement {
    ADD_PROPERTY(InterpolationObj, name);
    ADD_PROPERTY(InterpolationObj, value);
  public:
    AtRule(const SourceSpan& pstate,
      InterpolationObj name,
      ExpressionObj value,
      Block_Obj b = {});
    ATTACH_CLONE_OPERATIONS(AtRule);
    ATTACH_CRTP_PERFORM_METHODS();
  };

  /////////////////
  // Bubble.
  /////////////////
  class Bubble final : public Statement {
    ADD_PROPERTY(Statement_Obj, node)
    ADD_PROPERTY(bool, group_end)
  public:
    Bubble(const SourceSpan& pstate, Statement_Obj n, Statement_Obj g = {}, size_t t = 0);
    bool bubbles() const override final;
    // ATTACH_CLONE_OPERATIONS(Bubble)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  /////////////////
  // Trace.
  /////////////////
  class Trace final : public ParentStatement {
    ADD_CONSTREF(char, type)
    ADD_CONSTREF(sass::string, name)
  public:
    Trace(const SourceSpan& pstate, const sass::string& name, Block_Obj b = {}, char type = 'm');
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  // An expression that directly embeds a [Value]. This is never
  // constructed by the parser. It's only used when ASTs are
  // constructed dynamically, as for the `call()` function.
  class ValueExpression : public Expression {
    ADD_PROPERTY(ValueObj, value);
  public:
    ValueExpression(
      const SourceSpan& pstate,
      ValueObj value);
    ATTACH_CRTP_PERFORM_METHODS();
  };

  class ListExpression : public Expression {
    ADD_CONSTREF(sass::vector<ExpressionObj>, contents);
    ADD_PROPERTY(Sass_Separator, separator);
    ADD_PROPERTY(bool, hasBrackets);
  public:
    ListExpression(const SourceSpan& pstate, Sass_Separator separator = SASS_UNDEF);
    void concat(sass::vector<ExpressionObj>& expressions) {
      std::copy(
        expressions.begin(), expressions.end(),
        std::back_inserter(contents_)
      );
    }
    size_t size() const {
      return contents_.size();
    }
    Expression* get(size_t i) {
      return contents_[i];
    }
    void append(Expression* expression) {
      contents_.emplace_back(expression);
    }
    sass::string toString() {
      // var buffer = StringBuffer();
      // if (hasBrackets) buffer.writeCharCode($lbracket);
      // buffer.write(contents
      //   .map((element) = >
      //     _elementNeedsParens(element) ? "($element)" : element.toString())
      //   .join(separator == ListSeparator.comma ? ", " : " "));
      // if (hasBrackets) buffer.writeCharCode($rbracket);
      // return buffer.toString();
      return "ListExpression";
    }
    // Returns whether [expression], contained in [this],
    // needs parentheses when printed as Sass source.
    bool _elementNeedsParens(Expression* expression) {
      /*
      if (expression is ListExpression) {
        if (expression.contents.length < 2) return false;
        if (expression.hasBrackets) return false;
        return separator == ListSeparator.comma
            ? separator == ListSeparator.comma
            : separator != ListSeparator.undecided;
      }

      if (separator != ListSeparator.space) return false;

      if (expression is UnaryOperationExpression) {
        return expression.operator == UnaryOperator.plus ||
            expression.operator == UnaryOperator.minus;
      }

      */
      return false;
    }
    ATTACH_CRTP_PERFORM_METHODS();
  };

  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  class MapExpression final : public Expression {
    ADD_CONSTREF(sass::vector<ExpressionObj>, kvlist);
  public:
    void append(Expression* kv) {
      kvlist_.emplace_back(kv);
    }
    size_t size() const {
      return kvlist_.size();
    }
    Expression* get(size_t i) {
      return kvlist_[i];
    }
    MapExpression(const SourceSpan& pstate);
    ATTACH_CRTP_PERFORM_METHODS();
  };


  ////////////////////////////////////////////////////////////////////////
  // Declarations -- style rules consisting of a property name and values.
  ////////////////////////////////////////////////////////////////////////
  class Declaration final : public ParentStatement {
    ADD_PROPERTY(InterpolationObj, name);
    ADD_PROPERTY(ExpressionObj, value);
    ADD_PROPERTY(bool, is_custom_property);
  public:
    Declaration(const SourceSpan& pstate, InterpolationObj name, ExpressionObj value = {}, bool c = false, Block_Obj b = {});
    bool is_invisible() const override;
    // ATTACH_CLONE_OPERATIONS(Declaration)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  /////////////////////////////////////
  // Assignments -- variable and value.
  /////////////////////////////////////
  class Assignment final : public Statement {
    ADD_CONSTREF(EnvString, variable);
    ADD_PROPERTY(ExpressionObj, value);
    ADD_PROPERTY(IdxRef, vidx);
    ADD_PROPERTY(bool, is_default);
    ADD_PROPERTY(bool, is_global);
  public:
    Assignment(const SourceSpan& pstate, const sass::string& var, IdxRef vidx, Expression_Obj val, bool is_default = false, bool is_global = false);
    // ATTACH_CLONE_OPERATIONS(Assignment)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  class ImportBase : public Statement {
  public:
    ImportBase(const SourceSpan& pstate);
    ATTACH_VIRTUAL_COPY_OPERATIONS(ImportBase);
  };

  class StaticImport final : public ImportBase {
    ADD_PROPERTY(InterpolationObj, url);
    ADD_PROPERTY(CssStringObj, url2);
    ADD_PROPERTY(SupportsCondition_Obj, supports);
    ADD_PROPERTY(InterpolationObj, media);
    ADD_PROPERTY(bool, outOfOrder);
  public:
    StaticImport(const SourceSpan& pstate, InterpolationObj url, SupportsCondition_Obj supports = {}, InterpolationObj media = {});
    // ATTACH_CLONE_OPERATIONS(StaticImport);
    ATTACH_CRTP_PERFORM_METHODS();
  };

  class DynamicImport final : public ImportBase {
    ADD_CONSTREF(sass::string, url);
  public:
    DynamicImport(const SourceSpan& pstate, const sass::string& url);
    // ATTACH_CLONE_OPERATIONS(DynamicImport);
    ATTACH_CRTP_PERFORM_METHODS();
  };


  class ImportRule final : public Statement, public Vectorized<ImportBaseObj> {
  public:
    ImportRule(const SourceSpan& pstate);
    // ATTACH_CLONE_OPERATIONS(ImportRule);
    ATTACH_CRTP_PERFORM_METHODS();
  };

  ////////////////////////////////////////////////////////////////////////////
  // Import directives. CSS and Sass import lists can be intermingled, so it's
  // necessary to store a list of each in an Import node.
  ////////////////////////////////////////////////////////////////////////////
  class Import final : public ImportBase {
    sass::vector<Expression_Obj> urls_;
    sass::vector<Include>        incs_;
    // sass::vector<ImportBaseObj> imports_;
    ADD_CONSTREF(sass::vector<ExpressionObj>, import_queries);
    ADD_CONSTREF(sass::vector<CssMediaQueryObj>, queries);
  public:
    Import(const SourceSpan& pstate);
    sass::vector<Include>& incs();
    sass::vector<ExpressionObj>& urls();
    // sass::vector<ImportBaseObj>& imports();
    sass::vector<ExpressionObj>& queries2();
    bool is_invisible() const override;
    // ATTACH_CLONE_OPERATIONS(Import)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  // not yet resolved single import
  // so far we only know requested name
  class Import_Stub final : public ImportBase {
    Include resource_;
    // Sass_Import_Entry import_;
  public:
    Import_Stub(const SourceSpan& pstate, Include res/*,
      Sass_Import_Entry import*/);
    Include resource();
    // Sass_Import_Entry import();
    sass::string imp_path();
    sass::string abs_path();
    // ATTACH_CLONE_OPERATIONS(Import_Stub)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  //////////////////////////////
  // The Sass `@warn` directive.
  //////////////////////////////
  class WarnRule final : public Statement {
    ADD_PROPERTY(ExpressionObj, expression);
  public:
    WarnRule(const SourceSpan& pstate,
      ExpressionObj expression);
    // String toString() => "@warn $expression;";
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ///////////////////////////////
  // The Sass `@error` directive.
  ///////////////////////////////
  class ErrorRule final : public Statement {
    ADD_PROPERTY(ExpressionObj, expression);
  public:
    ErrorRule(const SourceSpan& pstate,
      ExpressionObj expression);
    // String toString() => "@error $expression;";
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ///////////////////////////////
  // The Sass `@debug` directive.
  ///////////////////////////////
  class DebugRule final : public Statement {
    ADD_PROPERTY(ExpressionObj, expression);
  public:
    DebugRule(const SourceSpan& pstate,
      ExpressionObj expression);
    // String toString() => "@debug $expression;";
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ///////////////////////////////////////////
  // CSS comments. These may be interpolated.
  ///////////////////////////////////////////
  class LoudComment final : public Statement {
    // The interpolated text of this comment, including comment characters.
    ADD_PROPERTY(InterpolationObj, text)
  public:
    LoudComment(const SourceSpan& pstate, InterpolationObj itpl);
    // ATTACH_CLONE_OPERATIONS(LoudComment)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  class SilentComment final : public Statement {
    // The text of this comment, including comment characters.
    ADD_CONSTREF(sass::string, text)
  public:
    SilentComment(const SourceSpan& pstate, const sass::string& text);
    // not used in dart sass beside tests!?
    // sass::string getDocComment() const;
    // ATTACH_CLONE_OPERATIONS(SilentComment)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ////////////////////////////////////
  // The Sass `@if` control directive.
  ////////////////////////////////////
  class If final : public ParentStatement {
    ADD_POINTER(IDXS*, idxs);
    ADD_PROPERTY(Expression_Obj, predicate);
    ADD_PROPERTY(Block_Obj, alternative);
  public:
    If(const SourceSpan& pstate, Expression_Obj pred, Block_Obj con, Block_Obj alt = {});
    virtual bool has_content() override;
    // ATTACH_CLONE_OPERATIONS(If)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  /////////////////////////////////////
  // The Sass `@for` control directive.
  /////////////////////////////////////
  class For final : public ParentStatement {
    ADD_CONSTREF(EnvString, variable);
    ADD_PROPERTY(Expression_Obj, lower_bound);
    ADD_PROPERTY(Expression_Obj, upper_bound);
    ADD_POINTER(IDXS*, idxs);
    ADD_PROPERTY(bool, is_inclusive);
  public:
    For(const SourceSpan& pstate, const EnvString& var, Expression_Obj lo, Expression_Obj hi, bool inc = false, Block_Obj b = {});
    // ATTACH_CLONE_OPERATIONS(For)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  //////////////////////////////////////
  // The Sass `@each` control directive.
  //////////////////////////////////////
  class Each final : public ParentStatement {
    ADD_CONSTREF(sass::vector<EnvString>, variables);
    ADD_POINTER(IDXS*, idxs);
    ADD_PROPERTY(Expression_Obj, list);
  public:
    Each(const SourceSpan& pstate, const sass::vector<EnvString>& vars, Expression_Obj lst, Block_Obj b = {});
    // ATTACH_CLONE_OPERATIONS(Each)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ///////////////////////////////////////
  // The Sass `@while` control directive.
  ///////////////////////////////////////
  class WhileRule final : public ParentStatement {
    ADD_PROPERTY(ExpressionObj, condition);
    ADD_POINTER(IDXS*, idxs);
  public:
    WhileRule(const SourceSpan& pstate,
      ExpressionObj condition,
      Block_Obj b = {});
    // String toString() = > "@while $condition {${children.join(" ")}}";
    ATTACH_CRTP_PERFORM_METHODS()
  };

  /////////////////////////////////////////////////////////////
  // The @return directive for use inside SassScript functions.
  /////////////////////////////////////////////////////////////
  class Return final : public Statement {
    ADD_PROPERTY(Expression_Obj, value);
  public:
    Return(const SourceSpan& pstate, Expression_Obj val);
    // ATTACH_CLONE_OPERATIONS(Return)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  class InvocationExpression :
    public Expression,
    public CallableInvocation {
  public:
    InvocationExpression(const SourceSpan& pstate,
      ArgumentInvocation* arguments) :
      Expression(pstate),
      CallableInvocation(arguments)
    {
    }
  };

  class InvocationStatement :
    public Statement,
    public CallableInvocation {
  public:
    InvocationStatement(const SourceSpan& pstate,
      ArgumentInvocation* arguments) :
      Statement(pstate),
      CallableInvocation(arguments)
    {
    }
  };

  /// A function invocation.
  ///
  /// This may be a plain CSS function or a Sass function.
  class IfExpression : public InvocationExpression {

  public:
    IfExpression(const SourceSpan& pstate,
      ArgumentInvocation* arguments) :
      InvocationExpression(pstate, arguments)
    {
    }

    sass::string toString() const {
      return "if" + arguments_->toString();
    }

    ATTACH_CRTP_PERFORM_METHODS();
  };

  /// A function invocation.
  ///
  /// This may be a plain CSS function or a Sass function.
  class FunctionExpression : public InvocationExpression {

    // The namespace of the function being invoked,
    // or `null` if it's invoked without a namespace.
    ADD_CONSTREF(sass::string, ns);

    ADD_PROPERTY(IdxRef, fidx);

    // The name of the function being invoked. If this is
    // interpolated, the function will be interpreted as plain
    // CSS, even if it has the same name as a Sass function.
    ADD_PROPERTY(InterpolationObj, name);

  public:
    FunctionExpression(const SourceSpan& pstate,
      Interpolation* name,
      ArgumentInvocation* arguments,
      const sass::string& ns = "") :
      InvocationExpression(pstate, arguments),
      ns_(ns), name_(name)
    {

    }

    ATTACH_CRTP_PERFORM_METHODS();
  };

  /////////////////////////////////////////////////////////////////////////////
  // Definitions for both mixins and functions. The two cases are distinguished
  // by a type tag.
  /////////////////////////////////////////////////////////////////////////////
  class CallableDeclaration : public ParentStatement {
    // The name of this callable.
    // This may be empty for callables without names.
    ADD_CONSTREF(EnvString, name);
    ADD_POINTER(IDXS*, idxs);
    ADD_PROPERTY(IdxRef, fidx);
    ADD_PROPERTY(IdxRef, cidx);

    // The comment immediately preceding this declaration.
    ADD_PROPERTY(SilentCommentObj, comment);
    // The declared arguments this callable accepts.
    ADD_PROPERTY(ArgumentDeclarationObj, arguments);
  public:
    CallableDeclaration(
      const SourceSpan& pstate,
      const EnvString& name,
      ArgumentDeclaration* arguments,
      SilentComment* comment = nullptr,
      Block* block = nullptr);

    // Stringify declarations etc. (dart)
    virtual sass::string toString1() const = 0;

    ATTACH_ABSTRACT_CRTP_PERFORM_METHODS();
  };

  class ContentBlock :
    public CallableDeclaration {
  public:
    ContentBlock(
      const SourceSpan& pstate,
      ArgumentDeclaration* arguments = nullptr,
      const sass::vector<StatementObj>& children = {});
    sass::string toString1() const override final;
    ATTACH_CRTP_PERFORM_METHODS();
  };

  class FunctionRule final :
    public CallableDeclaration {
  public:
    FunctionRule(
      const SourceSpan& pstate,
      const EnvString& name,
      ArgumentDeclaration* arguments,
      SilentComment* comment = nullptr,
      Block* block = nullptr);
    sass::string toString1() const override final;
    ATTACH_CRTP_PERFORM_METHODS();
  };

  class MixinRule final :
    public CallableDeclaration {
    ADD_CONSTREF(IdxRef, cidx);
  public:
    MixinRule(
      const SourceSpan& pstate,
      const sass::string& name,
      ArgumentDeclaration* arguments,
      SilentComment* comment = nullptr,
      Block* block = nullptr);
    sass::string toString1() const override final;
    ATTACH_CRTP_PERFORM_METHODS();
  };

  class IncludeRule final : public InvocationStatement {

    // The namespace of the mixin being invoked, or
    // `null` if it's invoked without a namespace.
    ADD_CONSTREF(sass::string, ns);

    // The name of the mixin being invoked.
    ADD_CONSTREF(EnvString, name);

    // The block that will be invoked for [ContentRule]s in the mixin
    // being invoked, or `null` if this doesn't pass a content block.
    ADD_PROPERTY(ContentBlockObj, content);

    ADD_CONSTREF(IdxRef, midx);

  public:

    IncludeRule(
      const SourceSpan& pstate,
      const EnvString& name,
      ArgumentInvocation* arguments,
      const sass::string& ns = "",
      ContentBlock* content = nullptr,
      Block* block = nullptr);

    bool has_content() override final;

    ATTACH_CRTP_PERFORM_METHODS();
  };

  ///////////////////////////////////////////////////
  // The @content directive for mixin content blocks.
  ///////////////////////////////////////////////////
  class ContentRule final : public Statement {
    ADD_PROPERTY(ArgumentInvocationObj, arguments);
  public:
    ContentRule(const SourceSpan& pstate,
      ArgumentInvocation* arguments);
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ////////////////////////////////////////////////////////////////////////////
  class ParenthesizedExpression final : public Expression {
    ADD_PROPERTY(ExpressionObj, expression)
  public:
    ParenthesizedExpression(const SourceSpan& pstate, Expression* expression);
    // ATTACH_CLONE_OPERATIONS(ParenthesizedExpression);
    ATTACH_CRTP_PERFORM_METHODS();
  };
  ////////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////////
  // Arithmetic negation (logical negation is just an ordinary function call).
  ////////////////////////////////////////////////////////////////////////////
  class Unary_Expression final : public Expression {
  public:
    enum Type { PLUS, MINUS, NOT, SLASH };
  private:
    ADD_PROPERTY(Type, optype)
    ADD_PROPERTY(Expression_Obj, operand)
  public:
    Unary_Expression(const SourceSpan& pstate, Type t, Expression_Obj o);
    // ATTACH_CLONE_OPERATIONS(Unary_Expression)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  // A Media Ruleset before it has been evaluated
  // Could be already final or an interpolation
  class MediaRule final : public ParentStatement {
    ADD_PROPERTY(InterpolationObj, query)
  public:
    // The query that determines on which platforms the styles will be in effect.
    // This is only parsed after the interpolation has been resolved.
    MediaRule(const SourceSpan& pstate, InterpolationObj query, Block_Obj block = {});

    bool bubbles() const override final { return true; };
    bool is_invisible() const override { return false; };
    // ATTACH_CLONE_OPERATIONS(MediaRule)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  /////////////////////////////////////////////////
  // A query for the `@at-root` rule.
  /////////////////////////////////////////////////
  class AtRootQuery final : public AST_Node {
  private:
    // The names of the rules included or excluded by this query. There are 
    // two special names. "all" indicates that all rules are included or
    // excluded, and "rule" indicates style rules are included or excluded.
    ADD_PROPERTY(StringSet, names);
    // Whether the query includes or excludes rules with the specified names.
    ADD_PROPERTY(bool, include);

  public:

    AtRootQuery(
      const SourceSpan& pstate,
      const StringSet& names,
      bool include);

    // Whether this includes or excludes *all* rules.
    bool all() const;

    // Whether this includes or excludes style rules.
    bool rule() const;

    // Whether this includes or excludes media rules.
    bool media() const;

    // Returns the at-rule name for [node], or `null` if it's not an at-rule.
    sass::string _nameFor(Statement* node) const;

    // Returns whether [this] excludes a node with the given [name].
    bool excludesName(const sass::string& name) const;

    // Returns whether [this] excludes [node].
    bool excludes(Statement* node) const;

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
      const sass::string& contents, Context& ctx);

    // The default at-root query, which excludes only style rules.
    // ToDo: check out how to make this static
    static AtRootQuery* defaultQuery(const SourceSpan& pstate);

    // Only for debug purposes
    sass::string toString() const;

    ATTACH_CRTP_PERFORM_METHODS()
  };

  ///////////
  // At-root.
  ///////////
  class AtRootRule final : public ParentStatement {
    ADD_PROPERTY(InterpolationObj, query);
    ADD_POINTER(IDXS*, idxs);
  public:
    AtRootRule(SourceSpan&& pstate, InterpolationObj query = {}, Block_Obj b = {});
    // ATTACH_CLONE_OPERATIONS(CssAtRootRule)
    ATTACH_CRTP_PERFORM_METHODS()
  };

  ////////////////////////////////////////////////////////////
  // Individual argument objects for mixin and function calls.
  ////////////////////////////////////////////////////////////
  class Argument final : public Expression {
    HASH_PROPERTY(Expression_Obj, value);
    HASH_CONSTREF(EnvString, name);
    ADD_PROPERTY(bool, is_rest_argument);
    ADD_PROPERTY(bool, is_keyword_argument);
    mutable size_t hash_;
  public:
    Argument(const SourceSpan& pstate, Expression_Obj val,
      const EnvString& n, bool rest = false, bool keyword = false);
    size_t hash() const override;
    ATTACH_CLONE_OPERATIONS(Argument)
      ATTACH_CRTP_PERFORM_METHODS()
  };

  /////////////////////////////////////////////////////////////////////////
  // Parameter lists -- in their own class to facilitate context-sensitive
  // error checking (e.g., ensuring that all optional parameters follow all
  // required parameters).
  /////////////////////////////////////////////////////////////////////////
  typedef Value* (*SassFnSig)(FN_PROTOTYPE2);
  typedef std::pair<ArgumentDeclarationObj, SassFnSig> SassFnPair;
  typedef sass::vector<SassFnPair> SassFnPairs;

  class Callable : public SassNode {
  public:
    Callable(const SourceSpan& pstate);
    virtual Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate) = 0;
    virtual bool operator== (const Callable& rhs) const = 0;
    ATTACH_CRTP_PERFORM_METHODS()
  };

  class UserDefinedCallable : public Callable {
    // Name of this callable (used for reporting)
    ADD_CONSTREF(EnvString, name);
    // The declaration (parameters this function takes).
    ADD_PROPERTY(CallableDeclarationObj, declaration);
    // The environment in which this callable was declared.
    ADD_POINTER(EnvSnapshot*, snapshot);
  public:
    UserDefinedCallable(
      const SourceSpan& pstate,
      const EnvString& name,
      CallableDeclarationObj declaration,
      EnvSnapshot* snapshot);
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate) override final;
    bool operator== (const Callable& rhs) const override final;
    ATTACH_CRTP_PERFORM_METHODS()
  };

  class PlainCssCallable : public Callable {
    ADD_CONSTREF(sass::string, name);
  public:
    PlainCssCallable(const SourceSpan& pstate, const sass::string& name);
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate) override final;
    bool operator== (const Callable& rhs) const override final;
    ATTACH_CRTP_PERFORM_METHODS()
  };

  class BuiltInCallable : public Callable {

    // The function name
    ADD_CONSTREF(EnvString, name);

    ADD_PROPERTY(ArgumentDeclarationObj, parameters);

    ADD_CONSTREF(SassFnPair, function);

  public:

    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate) override final;

    // Value* execute(ArgumentInvocation* arguments) {
    //   // return callback(arguments);
    //   return nullptr;
    // }

    // Creates a callable with a single [arguments] declaration
    // and a single [callback]. The argument declaration is parsed
    // from [arguments], which should not include parentheses.
    // Throws a [SassFormatException] if parsing fails.
    BuiltInCallable(
      const EnvString& name,
      ArgumentDeclaration* parameters,
      const SassFnSig& callback);

    virtual const SassFnPair& callbackFor(
      size_t positional,
      const KeywordMap<ValueObj>& names);

    bool operator== (const Callable& rhs) const override final;

    ATTACH_CRTP_PERFORM_METHODS()
  };

  class BuiltInCallables : public Callable {

    // The function name
    ADD_CONSTREF(EnvString, name);

    // The overloads declared for this callable.
    ADD_PROPERTY(SassFnPairs, overloads);

  public:

    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate) override final;

    // Value* execute(ArgumentInvocation* arguments) {
    //   // return callback(arguments);
    //   return nullptr;
    // }

    // Creates a callable with multiple implementations. Each
    // key/value pair in [overloads] defines the argument declaration
    // for the overload (which should not include parentheses), and
    // the callback to execute if that argument declaration matches.
    // Throws a [SassFormatException] if parsing fails.
    BuiltInCallables(
      const EnvString& name,
      const SassFnPairs& overloads);

    const SassFnPair& callbackFor(
      size_t positional,
      const KeywordMap<ValueObj>& names); // override final;

    bool operator== (const Callable& rhs) const override final;

    ATTACH_CRTP_PERFORM_METHODS()
  };

  class ExternalCallable : public Callable {

    // The function name
    ADD_CONSTREF(sass::string, name);

    ADD_PROPERTY(ArgumentDeclarationObj, declaration);

    ADD_PROPERTY(Sass_Function_Entry, function);

    ADD_POINTER(IDXS*, idxs);

  public:

    ExternalCallable(
      const sass::string& name,
      ArgumentDeclaration* parameters,
      Sass_Function_Entry function);
    Value* execute(Eval& eval, ArgumentInvocation* arguments, const SourceSpan& pstate) override final;
    bool operator== (const Callable& rhs) const override final;

    ATTACH_CRTP_PERFORM_METHODS()
  };


}

#include "ast_css.hpp"
#include "ast_values.hpp"
#include "ast_supports.hpp"
#include "ast_selectors.hpp"

#ifdef __clang__

// #pragma clang diagnostic pop
// #pragma clang diagnostic push

#endif

#endif
