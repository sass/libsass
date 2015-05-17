#ifndef SASS_EVAL_H
#define SASS_EVAL_H

#include <iostream>

#include "context.hpp"
#include "position.hpp"
#include "operation.hpp"
#include "environment.hpp"
#include "listize.hpp"
#include "sass_values.h"


namespace Sass {
  using namespace std;

  typedef Environment<AST_Node*> Env;
  struct Backtrace;
  class Listize;
  class Expand;

  class Eval : public Operation_CRTP<Expression*, Eval> {


    Expression* fallback_impl(AST_Node* n);

  public:
    Context&   ctx;
    Listize    listize;
    Expand*    exp;
    Eval(Eval* eval);
    Eval(Expand* exp, Context&, Env*, Backtrace*);
    virtual ~Eval();

    Env* environment();
    Context& context();
    Selector* selector();
    Backtrace* stacktrace();

    Eval* snapshot(); // for setting the env before eval'ing an expression
    using Operation<Expression*>::operator();

    // for evaluating function bodies
    Expression* operator()(Block*);
    Expression* operator()(Assignment*);
    Expression* operator()(If*);
    Expression* operator()(For*);
    Expression* operator()(Each*);
    Expression* operator()(While*);
    Expression* operator()(Return*);
    Expression* operator()(Warning*);
    Expression* operator()(Error*);
    Expression* operator()(Debug*);

    Expression* operator()(List*);
    Expression* operator()(Map*);
    Expression* operator()(Binary_Expression*);
    Expression* operator()(Unary_Expression*);
    Expression* operator()(Function_Call*);
    Expression* operator()(Function_Call_Schema*);
    Expression* operator()(Variable*);
    Expression* operator()(Textual*);
    Expression* operator()(Number*);
    Expression* operator()(Boolean*);
    Expression* operator()(String_Schema*);
    Expression* operator()(String_Constant*);
    Expression* operator()(Media_Query*);
    Expression* operator()(Media_Query_Expression*);
    Expression* operator()(At_Root_Expression*);
    Expression* operator()(Feature_Query*);
    Expression* operator()(Feature_Query_Condition*);
    Expression* operator()(Null*);
    Expression* operator()(Argument*);
    Expression* operator()(Arguments*);
    Expression* operator()(Comment*);

    // these should return selectors
    Expression* operator()(Selector_List*);
    Expression* operator()(Complex_Selector*);
    Expression* operator()(Compound_Selector*);
    Expression* operator()(Wrapped_Selector*);
    Expression* operator()(Pseudo_Selector*);
    Expression* operator()(Selector_Qualifier*);
    Expression* operator()(Type_Selector*);
    Expression* operator()(Selector_Placeholder*);
    Expression* operator()(Selector_Schema*);
    Expression* operator()(Parent_Selector*);
    Expression* operator()(Attribute_Selector*);


    template <typename U>
    Expression* fallback(U x) { return fallback_impl(x); }

  private:
    string interpolation(Expression* s);

  };

  Expression* cval_to_astnode(Sass_Value* v, Context& ctx, Backtrace* backtrace, ParserState pstate = ParserState("[AST]"));

  bool eq(Expression*, Expression*, Context&);
  bool lt(Expression*, Expression*, Context&);
}

#endif
