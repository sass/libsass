#ifndef SASS_AST_VALUES_H
#define SASS_AST_VALUES_H

#include <set>
#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <unordered_map>
#include "sass/base.h"


#include "../util.hpp"
#include "../units.hpp"
#include "../context.hpp"
#include "../position.hpp"
#include "../constants.hpp"
#include "../operation.hpp"
#include "../position.hpp"
#include "../inspect.hpp"
#include "../source_map.hpp"
#include "../environment.hpp"
#include "../error_handling.hpp"
#include "../ast_def_macros.hpp"
#include "../ast_fwd_decl.hpp"
#include "../source_map.hpp"

#include "sass.h"

#include "common.hpp"
#include "nodes.hpp"
#include "containers.hpp"
#include "expressions.hpp"

namespace Sass {


  ///////////////////////////////////////////////////////////////////////
  // Lists of values, both comma- and space-separated (distinguished by a
  // type-tag.) Also used to represent variable-length argument lists.
  ///////////////////////////////////////////////////////////////////////
  class List : public Value, public Vectorized<Expression*> {
    void adjust_after_pushing(Expression* e) { is_expanded(false); }
  private:
    ADD_PROPERTY(enum Sass_Separator, separator)
    ADD_PROPERTY(bool, is_arglist)
    ADD_PROPERTY(bool, from_selector)
  public:
    List(ParserState pstate,
         size_t size = 0, enum Sass_Separator sep = SASS_SPACE, bool argl = false)
    : Value(pstate),
      Vectorized<Expression*>(size),
      separator_(sep),
      is_arglist_(argl),
      from_selector_(false)
    { concrete_type(LIST); }
    std::string type() { return is_arglist_ ? "arglist" : "list"; }
    static std::string type_name() { return "list"; }
    const char* sep_string(bool compressed = false) const {
      return separator() == SASS_SPACE ?
        " " : (compressed ? "," : ", ");
    }
    bool is_invisible() const { return empty(); }
    Expression* value_at_index(size_t i);

    virtual size_t size() const;

    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<std::string>()(sep_string());
        for (size_t i = 0, L = length(); i < L; ++i)
          hash_combine(hash_, (elements()[i])->hash());
      }
      return hash_;
    }

    virtual void set_delayed(bool delayed)
    {
      is_delayed(delayed);
      // don't set children
    }

    virtual bool operator== (const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////////////////////////////////////
  // Key value paris.
  ///////////////////////////////////////////////////////////////////////
  class Map : public Value, public Hashed {
    void adjust_after_pushing(std::pair<Expression*, Expression*> p) { is_expanded(false); }
  public:
    Map(ParserState pstate,
         size_t size = 0)
    : Value(pstate),
      Hashed(size)
    { concrete_type(MAP); }
    std::string type() { return "map"; }
    static std::string type_name() { return "map"; }
    bool is_invisible() const { return empty(); }

    virtual size_t hash()
    {
      if (hash_ == 0) {
        for (auto key : keys()) {
          hash_combine(hash_, key->hash());
          hash_combine(hash_, at(key)->hash());
        }
      }

      return hash_;
    }

    virtual bool operator== (const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };

  inline static const std::string sass_op_to_name(enum Sass_OP op) {
    switch (op) {
      case AND: return "and"; break;
      case OR: return "or"; break;
      case EQ: return "eq"; break;
      case NEQ: return "neq"; break;
      case GT: return "gt"; break;
      case GTE: return "gte"; break;
      case LT: return "lt"; break;
      case LTE: return "lte"; break;
      case ADD: return "plus"; break;
      case SUB: return "sub"; break;
      case MUL: return "times"; break;
      case DIV: return "div"; break;
      case MOD: return "mod"; break;
      // this is only used internally!
      case NUM_OPS: return "[OPS]"; break;
      default: return "invalid"; break;
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // Binary expressions. Represents logical, relational, and arithmetic
  // operations. Templatized to avoid large switch statements and repetitive
  // subclassing.
  //////////////////////////////////////////////////////////////////////////
  class Binary_Expression : public PreValue {
  private:
    ADD_HASHED(Operand, op)
    ADD_HASHED(Expression*, left)
    ADD_HASHED(Expression*, right)
    size_t hash_;
  public:
    Binary_Expression(ParserState pstate,
                      Operand op, Expression* lhs, Expression* rhs)
    : PreValue(pstate), op_(op), left_(lhs), right_(rhs), hash_(0)
    { }
    const std::string type_name() {
      switch (type()) {
        case AND: return "and"; break;
        case OR: return "or"; break;
        case EQ: return "eq"; break;
        case NEQ: return "neq"; break;
        case GT: return "gt"; break;
        case GTE: return "gte"; break;
        case LT: return "lt"; break;
        case LTE: return "lte"; break;
        case ADD: return "add"; break;
        case SUB: return "sub"; break;
        case MUL: return "mul"; break;
        case DIV: return "div"; break;
        case MOD: return "mod"; break;
        // this is only used internally!
        case NUM_OPS: return "[OPS]"; break;
        default: return "invalid"; break;
      }
    }
    const std::string separator() {
      switch (type()) {
        case AND: return "&&"; break;
        case OR: return "||"; break;
        case EQ: return "=="; break;
        case NEQ: return "!="; break;
        case GT: return ">"; break;
        case GTE: return ">="; break;
        case LT: return "<"; break;
        case LTE: return "<="; break;
        case ADD: return "+"; break;
        case SUB: return "-"; break;
        case MUL: return "*"; break;
        case DIV: return "/"; break;
        case MOD: return "%"; break;
        // this is only used internally!
        case NUM_OPS: return "[OPS]"; break;
        default: return "invalid"; break;
      }
    }
    bool is_left_interpolant(void) const;
    bool is_right_interpolant(void) const;
    bool has_interpolant() const
    {
      return is_left_interpolant() ||
             is_right_interpolant();
    }
    virtual void set_delayed(bool delayed)
    {
      right()->set_delayed(delayed);
      left()->set_delayed(delayed);
      is_delayed(delayed);
    }
    virtual bool operator==(const Expression& rhs) const
    {
      try
      {
        const Binary_Expression* m = dynamic_cast<const Binary_Expression*>(&rhs);
        if (m == 0) return false;
        return type() == m->type() &&
               left() == m->left() &&
               right() == m->right();
      }
      catch (std::bad_cast&)
      {
        return false;
      }
      catch (...) { throw; }
    }
    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<size_t>()(type());
        hash_combine(hash_, left()->hash());
        hash_combine(hash_, right()->hash());
      }
      return hash_;
    }
    enum Sass_OP type() const { return op_.operand; }
    ATTACH_OPERATIONS()
  };


  //////////////////
  // Function calls.
  //////////////////
  class Function_Call : public PreValue {
    ADD_HASHED(std::string, name)
    ADD_HASHED(Arguments*, arguments)
    ADD_PROPERTY(void*, cookie)
    size_t hash_;
  public:
    Function_Call(ParserState pstate, std::string n, Arguments* args, void* cookie)
    : PreValue(pstate), name_(n), arguments_(args), cookie_(cookie), hash_(0)
    { concrete_type(STRING); }
    Function_Call(ParserState pstate, std::string n, Arguments* args)
    : PreValue(pstate), name_(n), arguments_(args), cookie_(0), hash_(0)
    { concrete_type(STRING); }

    virtual bool operator==(const Expression& rhs) const
    {
      try
      {
        const Function_Call* m = dynamic_cast<const Function_Call*>(&rhs);
        if (!(m && name() == m->name())) return false;
        if (!(m && arguments()->length() == m->arguments()->length())) return false;
        for (size_t i =0, L = arguments()->length(); i < L; ++i)
          if (!((*arguments())[i] == (*m->arguments())[i])) return false;
        return true;
      }
      catch (std::bad_cast&)
      {
        return false;
      }
      catch (...) { throw; }
    }

    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<std::string>()(name());
        for (auto argument : arguments()->elements())
          hash_combine(hash_, argument->hash());
      }
      return hash_;
    }

    ATTACH_OPERATIONS()
  };


  ///////////////////////
  // Variable references.
  ///////////////////////
  class Variable : public PreValue {
    ADD_PROPERTY(std::string, name)
  public:
    Variable(ParserState pstate, std::string n)
    : PreValue(pstate), name_(n)
    { }

    virtual bool operator==(const Expression& rhs) const
    {
      try
      {
        const Variable* e = dynamic_cast<const Variable*>(&rhs);
        return e && name() == e->name();
      }
      catch (std::bad_cast&)
      {
        return false;
      }
      catch (...) { throw; }
    }

    virtual size_t hash()
    {
      return std::hash<std::string>()(name());
    }

    ATTACH_OPERATIONS()
  };




  ////////////////////////////////////////////////
  // Numbers, percentages, dimensions, and colors.
  ////////////////////////////////////////////////
  class Number : public Value {
    ADD_HASHED(double, value)
    ADD_PROPERTY(bool, zero)
    std::vector<std::string> numerator_units_;
    std::vector<std::string> denominator_units_;
    size_t hash_;
  public:
    Number(ParserState pstate, double val, std::string u = "", bool zero = true);
    bool zero() { return zero_; }
    bool is_valid_css_unit() const;
    std::vector<std::string>& numerator_units()   { return numerator_units_; }
    std::vector<std::string>& denominator_units() { return denominator_units_; }
    const std::vector<std::string>& numerator_units() const   { return numerator_units_; }
    const std::vector<std::string>& denominator_units() const { return denominator_units_; }
    std::string type() { return "number"; }
    static std::string type_name() { return "number"; }
    std::string unit() const;

    bool is_unitless() const;
    double convert_factor(const Number&) const;
    bool convert(const std::string& unit = "", bool strict = false);
    void normalize(const std::string& unit = "", bool strict = false);
    // useful for making one number compatible with another
    std::string find_convertible_unit() const;

    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<double>()(value_);
        for (const auto numerator : numerator_units())
          hash_combine(hash_, std::hash<std::string>()(numerator));
        for (const auto denominator : denominator_units())
          hash_combine(hash_, std::hash<std::string>()(denominator));
      }
      return hash_;
    }

    virtual bool operator< (const Number& rhs) const;
    virtual bool operator== (const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };

  //////////
  // Colors.
  //////////
  class Color : public Value {
    ADD_HASHED(double, r)
    ADD_HASHED(double, g)
    ADD_HASHED(double, b)
    ADD_HASHED(double, a)
    ADD_PROPERTY(std::string, disp)
    size_t hash_;
  public:
    Color(ParserState pstate, double r, double g, double b, double a = 1, const std::string disp = "")
    : Value(pstate), r_(r), g_(g), b_(b), a_(a), disp_(disp),
      hash_(0)
    { concrete_type(COLOR); }
    std::string type() { return "color"; }
    static std::string type_name() { return "color"; }

    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<double>()(a_);
        hash_combine(hash_, std::hash<double>()(r_));
        hash_combine(hash_, std::hash<double>()(g_));
        hash_combine(hash_, std::hash<double>()(b_));
      }
      return hash_;
    }

    virtual bool operator== (const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };

  //////////////////////////////
  // Errors from Sass_Values.
  //////////////////////////////
  class Custom_Error : public Value {
    ADD_PROPERTY(std::string, message)
  public:
    Custom_Error(ParserState pstate, std::string msg)
    : Value(pstate), message_(msg)
    { concrete_type(C_ERROR); }
    virtual bool operator== (const Expression& rhs) const;
    ATTACH_OPERATIONS()
  };

  //////////////////////////////
  // Warnings from Sass_Values.
  //////////////////////////////
  class Custom_Warning : public Value {
    ADD_PROPERTY(std::string, message)
  public:
    Custom_Warning(ParserState pstate, std::string msg)
    : Value(pstate), message_(msg)
    { concrete_type(C_WARNING); }
    virtual bool operator== (const Expression& rhs) const;
    ATTACH_OPERATIONS()
  };

  ////////////
  // Booleans.
  ////////////
  class Boolean : public Value {
    ADD_HASHED(bool, value)
    size_t hash_;
  public:
    Boolean(ParserState pstate, bool val)
    : Value(pstate), value_(val),
      hash_(0)
    { concrete_type(BOOLEAN); }
    virtual operator bool() { return value_; }
    std::string type() { return "bool"; }
    static std::string type_name() { return "bool"; }
    virtual bool is_false() { return !value_; }

    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<bool>()(value_);
      }
      return hash_;
    }

    virtual bool operator== (const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };


  ////////////////////////////////////////////////////////////////////////
  // Abstract base class for Sass string values. Includes interpolated and
  // "flat" strings.
  ////////////////////////////////////////////////////////////////////////
  class String : public Value {
  public:
    String(ParserState pstate, bool delayed = false)
    : Value(pstate, delayed)
    { concrete_type(STRING); }
    static std::string type_name() { return "string"; }
    virtual ~String() = 0;
    virtual void rtrim() = 0;
    virtual void ltrim() = 0;
    virtual void trim() = 0;
    virtual bool operator==(const Expression& rhs) const = 0;
    ATTACH_OPERATIONS()
  };
  inline String::~String() { };

  ///////////////////////////////////////////////////////////////////////
  // Interpolated strings. Meant to be reduced to flat strings during the
  // evaluation phase.
  ///////////////////////////////////////////////////////////////////////
  class String_Schema : public String, public Vectorized<Expression*> {
    // ADD_PROPERTY(bool, has_interpolants)
    size_t hash_;
  public:
    String_Schema(ParserState pstate, size_t size = 0, bool has_interpolants = false)
    : String(pstate), Vectorized<Expression*>(size), hash_(0)
    { concrete_type(STRING); }
    std::string type() { return "string"; }
    static std::string type_name() { return "string"; }

    bool is_left_interpolant(void) const;
    bool is_right_interpolant(void) const;
    // void has_interpolants(bool tc) { }
    bool has_interpolants() {
      for (auto el : elements()) {
        if (el->is_interpolant()) return true;
      }
      return false;
    }
    virtual void rtrim();
    virtual void ltrim();
    virtual void trim();

    virtual size_t hash()
    {
      if (hash_ == 0) {
        for (auto string : elements())
          hash_combine(hash_, string->hash());
      }
      return hash_;
    }

    virtual void set_delayed(bool delayed) {
      is_delayed(delayed);
    }

    virtual bool operator==(const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////
  // Flat strings -- the lowest level of raw textual data.
  ////////////////////////////////////////////////////////
  class String_Constant : public String {
    ADD_PROPERTY(char, quote_mark)
    ADD_PROPERTY(bool, can_compress_whitespace)
    ADD_HASHED(std::string, value)
  protected:
    size_t hash_;
  public:
    String_Constant(ParserState pstate, std::string val)
    : String(pstate), quote_mark_(0), can_compress_whitespace_(false), value_(read_css_string(val)), hash_(0)
    { }
    String_Constant(ParserState pstate, const char* beg)
    : String(pstate), quote_mark_(0), can_compress_whitespace_(false), value_(read_css_string(std::string(beg))), hash_(0)
    { }
    String_Constant(ParserState pstate, const char* beg, const char* end)
    : String(pstate), quote_mark_(0), can_compress_whitespace_(false), value_(read_css_string(std::string(beg, end-beg))), hash_(0)
    { }
    String_Constant(ParserState pstate, const Token& tok)
    : String(pstate), quote_mark_(0), can_compress_whitespace_(false), value_(read_css_string(std::string(tok.begin, tok.end))), hash_(0)
    { }
    std::string type() { return "string"; }
    static std::string type_name() { return "string"; }
    virtual bool is_invisible() const;
    virtual void rtrim();
    virtual void ltrim();
    virtual void trim();

    virtual size_t hash()
    {
      if (hash_ == 0) {
        hash_ = std::hash<std::string>()(value_);
      }
      return hash_;
    }

    virtual bool operator==(const Expression& rhs) const;
    virtual std::string inspect() const; // quotes are forced on inspection

    // static char auto_quote() { return '*'; }
    static char double_quote() { return '"'; }
    static char single_quote() { return '\''; }

    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////
  // Possibly quoted string (unquote on instantiation)
  ////////////////////////////////////////////////////////
  class String_Quoted : public String_Constant {
  public:
    String_Quoted(ParserState pstate, std::string val, char q = 0, bool keep_utf8_escapes = false)
    : String_Constant(pstate, val)
    {
      value_ = unquote(value_, &quote_mark_, keep_utf8_escapes);
      if (q && quote_mark_) quote_mark_ = q;
    }
    virtual bool operator==(const Expression& rhs) const;
    virtual std::string inspect() const; // quotes are forced on inspection
    ATTACH_OPERATIONS()
  };


  //////////////////
  // The null value.
  //////////////////
  class Null : public Value {
  public:
    Null(ParserState pstate) : Value(pstate) { concrete_type(NULL_VAL); }
    std::string type() { return "null"; }
    static std::string type_name() { return "null"; }
    bool is_invisible() const { return true; }
    operator bool() { return false; }
    bool is_false() { return true; }

    virtual size_t hash()
    {
      return -1;
    }

    virtual bool operator== (const Expression& rhs) const;

    ATTACH_OPERATIONS()
  };

}

#endif
