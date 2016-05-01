#ifndef SASS_AST_STATEMENTS_H
#define SASS_AST_STATEMENTS_H

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

#include "common.hpp"
#include "nodes.hpp"
#include "containers.hpp"

namespace Sass {


  ////////////////////////
  // Blocks of statements.
  ////////////////////////
  class Block : public Statement, public Vectorized<Statement*> {
    ADD_PROPERTY(bool, is_root)
    ADD_PROPERTY(bool, is_at_root);
    // needed for properly formatted CSS emission
    ADD_PROPERTY(bool, has_hoistable)
    ADD_PROPERTY(bool, has_non_hoistable)
  protected:
    void adjust_after_pushing(Statement* s)
    {
      if (s->is_hoistable()) has_hoistable_     = true;
      else                   has_non_hoistable_ = true;
    }
  public:
    Block(ParserState pstate, size_t s = 0, bool r = false)
    : Statement(pstate),
      Vectorized<Statement*>(s),
      is_root_(r),
      is_at_root_(false),
      has_hoistable_(false),
      has_non_hoistable_(false)
    { }
    virtual bool has_content()
    {
      for (size_t i = 0, L = elements().size(); i < L; ++i) {
        if (elements()[i]->has_content()) return true;
      }
      return Statement::has_content();
    }
    Block* block() { return this; }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////////////////////
  // Abstract base class for statements that contain blocks of statements.
  ////////////////////////////////////////////////////////////////////////
  class Has_Block : public Statement {
    ADD_PROPERTY(Block*, block)
  public:
    Has_Block(ParserState pstate, Block* b)
    : Statement(pstate), block_(b)
    { }
    virtual bool has_content()
    {
      return (block_ && block_->has_content()) || Statement::has_content();
    }
    virtual ~Has_Block() = 0;
  };
  inline Has_Block::~Has_Block() { }


  /////////////////
  // Bubble.
  /////////////////
  class Bubble : public Statement {
    ADD_PROPERTY(Statement*, node)
    ADD_PROPERTY(bool, group_end)
  public:
    Bubble(ParserState pstate, Statement* n, Statement* g = 0, size_t t = 0)
    : Statement(pstate, Statement::BUBBLE, t), node_(n), group_end_(g == 0)
    { }
    bool bubbles() { return true; }
    ATTACH_OPERATIONS()
  };


  ////////////////////////////////////////////////////////////////////////
  // Declarations -- style rules consisting of a property name and values.
  ////////////////////////////////////////////////////////////////////////
  class Declaration : public Statement {
    ADD_PROPERTY(String*, property)
    ADD_PROPERTY(Expression*, value)
    ADD_PROPERTY(bool, is_important)
    ADD_PROPERTY(bool, is_indented)
  public:
    Declaration(ParserState pstate,
                String* prop, Expression* val, bool i = false)
    : Statement(pstate), property_(prop), value_(val), is_important_(i), is_indented_(false)
    { statement_type(DECLARATION); }
    ATTACH_OPERATIONS()
  };

  /////////////////////////////////////
  // Assignments -- variable and value.
  /////////////////////////////////////
  class Assignment : public Statement {
    ADD_PROPERTY(std::string, variable)
    ADD_PROPERTY(Expression*, value)
    ADD_PROPERTY(bool, is_default)
    ADD_PROPERTY(bool, is_global)
  public:
    Assignment(ParserState pstate,
               std::string var, Expression* val,
               bool is_default = false,
               bool is_global = false)
    : Statement(pstate), variable_(var), value_(val), is_default_(is_default), is_global_(is_global)
    { statement_type(ASSIGNMENT); }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////////////////////////////////////////////////
  // Import directives. CSS and Sass import lists can be intermingled, so it's
  // necessary to store a list of each in an Import node.
  ////////////////////////////////////////////////////////////////////////////
  class Import : public Statement {
    std::vector<Expression*> urls_;
    std::vector<Include>     incs_;
    ADD_PROPERTY(List*,      media_queries);
  public:
    Import(ParserState pstate)
    : Statement(pstate),
      urls_(std::vector<Expression*>()),
      incs_(std::vector<Include>()),
      media_queries_(0)
    { statement_type(IMPORT); }
    std::vector<Expression*>& urls() { return urls_; }
    std::vector<Include>& incs() { return incs_; }
    ATTACH_OPERATIONS()
  };

  // not yet resolved single import
  // so far we only know requested name
  class Import_Stub : public Statement {
    Include resource_;
  public:
    std::string abs_path() { return resource_.abs_path; };
    std::string imp_path() { return resource_.imp_path; };
    Include resource() { return resource_; };

    Import_Stub(ParserState pstate, Include res)
    : Statement(pstate), resource_(res)
    { statement_type(IMPORT_STUB); }
    ATTACH_OPERATIONS()
  };

  //////////////////////////////
  // The Sass `@warn` directive.
  //////////////////////////////
  class Warning : public Statement {
    ADD_PROPERTY(Expression*, message)
  public:
    Warning(ParserState pstate, Expression* msg)
    : Statement(pstate), message_(msg)
    { statement_type(WARNING); }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////
  // The Sass `@error` directive.
  ///////////////////////////////
  class Error : public Statement {
    ADD_PROPERTY(Expression*, message)
  public:
    Error(ParserState pstate, Expression* msg)
    : Statement(pstate), message_(msg)
    { statement_type(ERROR); }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////
  // The Sass `@debug` directive.
  ///////////////////////////////
  class Debug : public Statement {
    ADD_PROPERTY(Expression*, value)
  public:
    Debug(ParserState pstate, Expression* val)
    : Statement(pstate), value_(val)
    { statement_type(DEBUGSTMT); }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////////
  // CSS comments. These may be interpolated.
  ///////////////////////////////////////////
  class Comment : public Statement {
    ADD_PROPERTY(String*, text)
    ADD_PROPERTY(bool, is_important)
  public:
    Comment(ParserState pstate, String* txt, bool is_important)
    : Statement(pstate), text_(txt), is_important_(is_important)
    { statement_type(COMMENT); }
    virtual bool is_invisible() const
    { return /* is_important() == */ false; }
    ATTACH_OPERATIONS()
  };


  /////////////////////////////////////////////////////////////
  // The @return directive for use inside SassScript functions.
  /////////////////////////////////////////////////////////////
  class Return : public Statement {
    ADD_PROPERTY(Expression*, value)
  public:
    Return(ParserState pstate, Expression* val)
    : Statement(pstate), value_(val)
    { statement_type(RETURN); }
    ATTACH_OPERATIONS()
  };

  ////////////////////////////////
  // The Sass `@extend` directive.
  ////////////////////////////////
  class Extension : public Statement {
    ADD_PROPERTY(Selector*, selector)
  public:
    Extension(ParserState pstate, Selector* s)
    : Statement(pstate), selector_(s)
    { statement_type(EXTEND); }
    ATTACH_OPERATIONS()
  };

  ///////////////////////////////////////////////////
  // The @content directive for mixin content blocks.
  ///////////////////////////////////////////////////
  class Content : public Statement {
    ADD_PROPERTY(Media_Block*, media_block)
  public:
    Content(ParserState pstate) : Statement(pstate)
    { statement_type(CONTENT); }
    ATTACH_OPERATIONS()
  };

}

#endif
