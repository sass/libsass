#ifndef SASS_AST_BLOCKS_H
#define SASS_AST_BLOCKS_H

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
#include "values.hpp"
#include "containers.hpp"
#include "expressions.hpp"
#include "statements.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////////
  // Rulesets (i.e., sets of styles headed by a selector and containing a block
  // of style declarations.
  /////////////////////////////////////////////////////////////////////////////
  class Ruleset : public Has_Block {
    ADD_PROPERTY(Selector*, selector)
    ADD_PROPERTY(bool, at_root);
    ADD_PROPERTY(bool, is_root);
  public:
    Ruleset(ParserState pstate, Selector* s = 0, Block* b = 0)
    : Has_Block(pstate, b), selector_(s), at_root_(false), is_root_(false)
    { statement_type(RULESET); }
    bool is_invisible() const;
    // nested rulesets need to be hoisted out of their enclosing blocks
    bool is_hoistable() { return true; }
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////////////////////////
  // Nested declaration sets (i.e., namespaced properties).
  /////////////////////////////////////////////////////////
  class Propset : public Has_Block {
    ADD_PROPERTY(String*, property_fragment)
  public:
    Propset(ParserState pstate, String* pf, Block* b = 0)
    : Has_Block(pstate, b), property_fragment_(pf)
    { }
    ATTACH_OPERATIONS()
  };


  /////////////////
  // Media queries.
  /////////////////
  class Media_Block : public Has_Block {
    ADD_PROPERTY(List*, media_queries)
  public:
    Media_Block(ParserState pstate, List* mqs, Block* b)
    : Has_Block(pstate, b), media_queries_(mqs)
    { statement_type(MEDIA); }
    Media_Block(ParserState pstate, List* mqs, Block* b, Selector* s)
    : Has_Block(pstate, b), media_queries_(mqs)
    { statement_type(MEDIA); }
    bool bubbles() { return true; }
    bool is_hoistable() { return true; }
    bool is_invisible() const;
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////////////////////////////////////
  // At-rules -- arbitrary directives beginning with "@" that may have an
  // optional statement block.
  ///////////////////////////////////////////////////////////////////////
  class Directive : public Has_Block {
    ADD_PROPERTY(std::string, keyword)
    ADD_PROPERTY(Selector*, selector)
    ADD_PROPERTY(Expression*, value)
  public:
    Directive(ParserState pstate, std::string kwd, Selector* sel = 0, Block* b = 0, Expression* val = 0)
    : Has_Block(pstate, b), keyword_(kwd), selector_(sel), value_(val) // set value manually if needed
    { statement_type(DIRECTIVE); }
    bool bubbles() { return is_keyframes() || is_media(); }
    bool is_media() {
      return keyword_.compare("@-webkit-media") == 0 ||
             keyword_.compare("@-moz-media") == 0 ||
             keyword_.compare("@-o-media") == 0 ||
             keyword_.compare("@media") == 0;
    }
    bool is_keyframes() {
      return keyword_.compare("@-webkit-keyframes") == 0 ||
             keyword_.compare("@-moz-keyframes") == 0 ||
             keyword_.compare("@-o-keyframes") == 0 ||
             keyword_.compare("@keyframes") == 0;
    }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////////////////////////////////////
  // Keyframe-rules -- the child blocks of "@keyframes" nodes.
  ///////////////////////////////////////////////////////////////////////
  class Keyframe_Rule : public Has_Block {
    ADD_PROPERTY(Selector*, selector)
  public:
    Keyframe_Rule(ParserState pstate, Block* b)
    : Has_Block(pstate, b), selector_(0)
    { statement_type(KEYFRAMERULE); }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////
  // The Sass `@if` control directive.
  ////////////////////////////////////
  class If : public Has_Block {
    ADD_PROPERTY(Expression*, predicate)
    ADD_PROPERTY(Block*, alternative)
  public:
    If(ParserState pstate, Expression* pred, Block* con, Block* alt = 0)
    : Has_Block(pstate, con), predicate_(pred), alternative_(alt)
    { statement_type(IF); }
    virtual bool has_content()
    {
      return Has_Block::has_content() || (alternative_ && alternative_->has_content());
    }
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////
  // The Sass `@for` control directive.
  /////////////////////////////////////
  class For : public Has_Block {
    ADD_PROPERTY(std::string, variable)
    ADD_PROPERTY(Expression*, lower_bound)
    ADD_PROPERTY(Expression*, upper_bound)
    ADD_PROPERTY(bool, is_inclusive)
  public:
    For(ParserState pstate,
        std::string var, Expression* lo, Expression* hi, Block* b, bool inc)
    : Has_Block(pstate, b),
      variable_(var), lower_bound_(lo), upper_bound_(hi), is_inclusive_(inc)
    { statement_type(FOR); }
    ATTACH_OPERATIONS()
  };

  //////////////////////////////////////
  // The Sass `@each` control directive.
  //////////////////////////////////////
  class Each : public Has_Block {
    ADD_PROPERTY(std::vector<std::string>, variables)
    ADD_PROPERTY(Expression*, list)
  public:
    Each(ParserState pstate, std::vector<std::string> vars, Expression* lst, Block* b)
    : Has_Block(pstate, b), variables_(vars), list_(lst)
    { statement_type(EACH); }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////
  // The Sass `@while` control directive.
  ///////////////////////////////////////
  class While : public Has_Block {
    ADD_PROPERTY(Expression*, predicate)
  public:
    While(ParserState pstate, Expression* pred, Block* b)
    : Has_Block(pstate, b), predicate_(pred)
    { statement_type(WHILE); }
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////////////////////////////////////////////
  // Definitions for both mixins and functions. The two cases are distinguished
  // by a type tag.
  /////////////////////////////////////////////////////////////////////////////
  struct Backtrace;
  typedef Environment<AST_Node*> Env;
  typedef const char* Signature;
  typedef Expression* (*Native_Function)(Env&, Env&, Context&, Signature, ParserState, Backtrace*);
  typedef const char* Signature;
  class Definition : public Has_Block {
  public:
    enum Type { MIXIN, FUNCTION };
    ADD_PROPERTY(std::string, name)
    ADD_PROPERTY(Parameters*, parameters)
    ADD_PROPERTY(Env*, environment)
    ADD_PROPERTY(Type, type)
    ADD_PROPERTY(Native_Function, native_function)
    ADD_PROPERTY(Sass_Function_Entry, c_function)
    ADD_PROPERTY(void*, cookie)
    ADD_PROPERTY(bool, is_overload_stub)
    ADD_PROPERTY(Signature, signature)
  public:
    Definition(ParserState pstate,
               std::string n,
               Parameters* params,
               Block* b,
               Type t)
    : Has_Block(pstate, b),
      name_(n),
      parameters_(params),
      environment_(0),
      type_(t),
      native_function_(0),
      c_function_(0),
      cookie_(0),
      is_overload_stub_(false),
      signature_(0)
    { }
    Definition(ParserState pstate,
               Signature sig,
               std::string n,
               Parameters* params,
               Native_Function func_ptr,
               bool overload_stub = false)
    : Has_Block(pstate, 0),
      name_(n),
      parameters_(params),
      environment_(0),
      type_(FUNCTION),
      native_function_(func_ptr),
      c_function_(0),
      cookie_(0),
      is_overload_stub_(overload_stub),
      signature_(sig)
    { }
    Definition(ParserState pstate,
               Signature sig,
               std::string n,
               Parameters* params,
               Sass_Function_Entry c_func,
               bool whatever,
               bool whatever2)
    : Has_Block(pstate, 0),
      name_(n),
      parameters_(params),
      environment_(0),
      type_(FUNCTION),
      native_function_(0),
      c_function_(c_func),
      cookie_(sass_function_get_cookie(c_func)),
      is_overload_stub_(false),
      signature_(sig)
    { }
    ATTACH_OPERATIONS()
  };

  //////////////////////////////////////
  // Mixin calls (i.e., `@include ...`).
  //////////////////////////////////////
  class Mixin_Call : public Has_Block {
    ADD_PROPERTY(std::string, name)
    ADD_PROPERTY(Arguments*, arguments)
  public:
    Mixin_Call(ParserState pstate, std::string n, Arguments* args, Block* b = 0)
    : Has_Block(pstate, b), name_(n), arguments_(args)
    { }
    ATTACH_OPERATIONS()
  };

  ////////////////////
  // `@supports` rule.
  ////////////////////
  class Supports_Block : public Has_Block {
    ADD_PROPERTY(Supports_Condition*, condition)
  public:
    Supports_Block(ParserState pstate, Supports_Condition* condition, Block* block = 0)
    : Has_Block(pstate, block), condition_(condition)
    { statement_type(SUPPORTS); }
    bool is_hoistable() { return true; }
    bool bubbles() { return true; }
    ATTACH_OPERATIONS()
  };


  ///////////
  // At-root.
  ///////////
  class At_Root_Block : public Has_Block {
    ADD_PROPERTY(At_Root_Query*, expression)
  public:
    At_Root_Block(ParserState pstate, Block* b = 0, At_Root_Query* e = 0)
    : Has_Block(pstate, b), expression_(e)
    { statement_type(ATROOT); }
    bool is_hoistable() { return true; }
    bool bubbles() { return true; }
    bool exclude_node(Statement* s) {
      if (s->statement_type() == Statement::DIRECTIVE)
      {
        return expression()->exclude(static_cast<Directive*>(s)->keyword().erase(0, 1));
      }
      if (s->statement_type() == Statement::MEDIA)
      {
        return expression()->exclude("media");
      }
      if (s->statement_type() == Statement::RULESET)
      {
        return expression()->exclude("rule");
      }
      if (s->statement_type() == Statement::SUPPORTS)
      {
        return expression()->exclude("supports");
      }
      if (static_cast<Directive*>(s)->is_keyframes())
      {
        return expression()->exclude("keyframes");
      }
      return false;
    }
    ATTACH_OPERATIONS()
  };

}

#endif
