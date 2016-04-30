#ifndef SASS_AST_EXPRESSIONS_H
#define SASS_AST_EXPRESSIONS_H

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

namespace Sass {

  typedef Environment<AST_Node*> Env;

  //////////////////////////////////////////////////////////////////////
  // Still just an expression, but with a to_string method
  //////////////////////////////////////////////////////////////////////
  class PreValue : public Expression {
  public:
    PreValue(ParserState pstate,
               bool d = false, bool e = false, bool i = false, Concrete_Type ct = NONE)
    : Expression(pstate, d, e, i, ct)
    { }
    virtual ~PreValue() { }
  };

  //////////////////////////////////////////////////////////////////////
  // base class for values that support operations
  //////////////////////////////////////////////////////////////////////
  class Value : public Expression {
  public:
    Value(ParserState pstate,
          bool d = false, bool e = false, bool i = false, Concrete_Type ct = NONE)
    : Expression(pstate, d, e, i, ct)
    { }
    virtual bool operator== (const Expression& rhs) const = 0;
  };



  ////////////////////////////////////////////////////////////////////////////
  // Arithmetic negation (logical negation is just an ordinary function call).
  ////////////////////////////////////////////////////////////////////////////
  class Unary_Expression : public Expression {
  public:
    enum Type { PLUS, MINUS, NOT };
  private:
    ADD_HASHED(Type, type)
    ADD_HASHED(Expression*, operand)
    size_t hash_;
  public:
    Unary_Expression(ParserState pstate, Type t, Expression* o)
    : Expression(pstate), type_(t), operand_(o), hash_(0)
    { }
    const std::string type_name() {
      switch (type_) {
        case PLUS: return "plus"; break;
        case MINUS: return "minus"; break;
        case NOT: return "not"; break;
        default: return "invalid"; break;
      }
    }
    virtual bool operator==(const Expression& rhs) const
    {
      try
      {
        const Unary_Expression* m = dynamic_cast<const Unary_Expression*>(&rhs);
        if (m == 0) return false;
        return type() == m->type() &&
               operand() == m->operand();
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
        hash_ = std::hash<size_t>()(type_);
        hash_combine(hash_, operand()->hash());
      };
      return hash_;
    }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////////
  // Individual argument objects for mixin and function calls.
  ////////////////////////////////////////////////////////////
  class Argument : public Expression {
    ADD_HASHED(Expression*, value)
    ADD_HASHED(std::string, name)
    ADD_PROPERTY(bool, is_rest_argument)
    ADD_PROPERTY(bool, is_keyword_argument)
    size_t hash_;
  public:
    Argument(ParserState pstate, Expression* val, std::string n = "", bool rest = false, bool keyword = false)
    : Expression(pstate), value_(val), name_(n), is_rest_argument_(rest), is_keyword_argument_(keyword), hash_(0)
    {
      if (!name_.empty() && is_rest_argument_) {
        error("variable-length argument may not be passed by name", pstate);
      }
    }

    virtual void set_delayed(bool delayed);
    virtual bool operator==(const Expression& rhs) const
    {
      try
      {
        const Argument* m = dynamic_cast<const Argument*>(&rhs);
        if (!(m && name() == m->name())) return false;
        return *value() == *m->value();
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
        hash_combine(hash_, value()->hash());
      }
      return hash_;
    }

    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////////////////////
  // Argument lists -- in their own class to facilitate context-sensitive
  // error checking (e.g., ensuring that all ordinal arguments precede all
  // named arguments).
  ////////////////////////////////////////////////////////////////////////
  class Arguments : public Expression, public Vectorized<Argument*> {
    ADD_PROPERTY(bool, has_named_arguments)
    ADD_PROPERTY(bool, has_rest_argument)
    ADD_PROPERTY(bool, has_keyword_argument)
  protected:
    void adjust_after_pushing(Argument* a);
  public:
    Arguments(ParserState pstate)
    : Expression(pstate),
      Vectorized<Argument*>(),
      has_named_arguments_(false),
      has_rest_argument_(false),
      has_keyword_argument_(false)
    { }

    virtual void set_delayed(bool delayed);

    Argument* get_rest_argument();
    Argument* get_keyword_argument();

    ATTACH_OPERATIONS()
  };


  /////////////////////////
  // Function call schemas.
  /////////////////////////
  class Function_Call_Schema : public Expression {
    ADD_PROPERTY(String*, name)
    ADD_PROPERTY(Arguments*, arguments)
  public:
    Function_Call_Schema(ParserState pstate, String* n, Arguments* args)
    : Expression(pstate), name_(n), arguments_(args)
    { concrete_type(STRING); }
    ATTACH_OPERATIONS()
  };


  ////////////////////////////////////////////////////////////////////////////
  // Textual (i.e., unevaluated) numeric data. Variants are distinguished with
  // a type tag.
  ////////////////////////////////////////////////////////////////////////////
  class Textual : public Expression {
  public:
    enum Type { NUMBER, PERCENTAGE, DIMENSION, HEX };
  private:
    ADD_HASHED(Type, type)
    ADD_HASHED(std::string, value)
    size_t hash_;
  public:
    Textual(ParserState pstate, Type t, std::string val)
    : Expression(pstate, DELAYED), type_(t), value_(val),
      hash_(0)
    { }

    virtual bool operator==(const Expression& rhs) const
    {
      try
      {
        const Textual* e = dynamic_cast<const Textual*>(&rhs);
        return e && value() == e->value() && type() == e->type();
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
        hash_ = std::hash<std::string>()(value_);
        hash_combine(hash_, std::hash<int>()(type_));
      }
      return hash_;
    }

    ATTACH_OPERATIONS()
  };



  /////////////////
  // Media queries.
  /////////////////
  class Media_Query : public Expression,
                      public Vectorized<Media_Query_Expression*> {
    ADD_PROPERTY(String*, media_type)
    ADD_PROPERTY(bool, is_negated)
    ADD_PROPERTY(bool, is_restricted)
  public:
    Media_Query(ParserState pstate,
                String* t = 0, size_t s = 0, bool n = false, bool r = false)
    : Expression(pstate), Vectorized<Media_Query_Expression*>(s),
      media_type_(t), is_negated_(n), is_restricted_(r)
    { }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////
  // Media expressions (for use inside media queries).
  ////////////////////////////////////////////////////
  class Media_Query_Expression : public Expression {
    ADD_PROPERTY(Expression*, feature)
    ADD_PROPERTY(Expression*, value)
    ADD_PROPERTY(bool, is_interpolated)
  public:
    Media_Query_Expression(ParserState pstate,
                           Expression* f, Expression* v, bool i = false)
    : Expression(pstate), feature_(f), value_(v), is_interpolated_(i)
    { }
    ATTACH_OPERATIONS()
  };

  //////////////////////////////////////////////////////
  // The abstract superclass of all Supports conditions.
  //////////////////////////////////////////////////////
  class Supports_Condition : public Expression {
  public:
    Supports_Condition(ParserState pstate)
    : Expression(pstate)
    { }
    virtual bool needs_parens(Supports_Condition* cond) const { return false; }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////////
  // An operator condition (e.g. `CONDITION1 and CONDITION2`).
  ////////////////////////////////////////////////////////////
  class Supports_Operator : public Supports_Condition {
  public:
    enum Operand { AND, OR };
  private:
    ADD_PROPERTY(Supports_Condition*, left);
    ADD_PROPERTY(Supports_Condition*, right);
    ADD_PROPERTY(Operand, operand);
  public:
    Supports_Operator(ParserState pstate, Supports_Condition* l, Supports_Condition* r, Operand o)
    : Supports_Condition(pstate), left_(l), right_(r), operand_(o)
    { }
    virtual bool needs_parens(Supports_Condition* cond) const;
    ATTACH_OPERATIONS()
  };

  //////////////////////////////////////////
  // A negation condition (`not CONDITION`).
  //////////////////////////////////////////
  class Supports_Negation : public Supports_Condition {
  private:
    ADD_PROPERTY(Supports_Condition*, condition);
  public:
    Supports_Negation(ParserState pstate, Supports_Condition* c)
    : Supports_Condition(pstate), condition_(c)
    { }
    virtual bool needs_parens(Supports_Condition* cond) const;
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////////////////////
  // A declaration condition (e.g. `(feature: value)`).
  /////////////////////////////////////////////////////
  class Supports_Declaration : public Supports_Condition {
  private:
    ADD_PROPERTY(Expression*, feature);
    ADD_PROPERTY(Expression*, value);
  public:
    Supports_Declaration(ParserState pstate, Expression* f, Expression* v)
    : Supports_Condition(pstate), feature_(f), value_(v)
    { }
    virtual bool needs_parens(Supports_Condition* cond) const { return false; }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////////////
  // An interpolation condition (e.g. `#{$var}`).
  ///////////////////////////////////////////////
  class Supports_Interpolation : public Supports_Condition {
  private:
    ADD_PROPERTY(Expression*, value);
  public:
    Supports_Interpolation(ParserState pstate, Expression* v)
    : Supports_Condition(pstate), value_(v)
    { }
    virtual bool needs_parens(Supports_Condition* cond) const { return false; }
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////////////////
  // At root expressions (for use inside @at-root).
  /////////////////////////////////////////////////
  class At_Root_Query : public Expression {
  private:
    ADD_PROPERTY(Expression*, feature)
    ADD_PROPERTY(Expression*, value)
  public:
    At_Root_Query(ParserState pstate, Expression* f = 0, Expression* v = 0, bool i = false)
    : Expression(pstate), feature_(f), value_(v)
    { }
    bool exclude(std::string str);
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////
  // Thunks for delayed evaluation.
  /////////////////////////////////
  class Thunk : public Expression {
    ADD_PROPERTY(Expression*, expression)
    ADD_PROPERTY(Env*, environment)
  public:
    Thunk(ParserState pstate, Expression* exp, Env* env = 0)
    : Expression(pstate), expression_(exp), environment_(env)
    { }
  };

}

#endif
