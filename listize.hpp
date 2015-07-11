#ifndef SASS_LISTIZE_H
#define SASS_LISTIZE_H

#include <vector>
#include <iostream>

#include "ast.hpp"
#include "context.hpp"
#include "operation.hpp"
#include "environment.hpp"

namespace Sass {
  using namespace std;

  typedef Environment<AST_Node*> Env;
  struct Backtrace;

  class Listize : public Operation_CRTP<Value*, Listize> {

    Context&            ctx;

    Value* fallback_impl(AST_Node* n);

  public:
    Listize(Context&);
    virtual ~Listize() { }

    using Operation<Value*>::operator();

    Value* operator()(Selector_List*);
    Value* operator()(Complex_Selector*);
    Value* operator()(Compound_Selector*);
    //Value* operator()(Parent_Selector*);
    Value* operator()(Type_Selector*);
    Value* operator()(Selector_Qualifier*);

    // Value* operator()(Binary_Expression*);

    template <typename U>
    Value* fallback(U x) { return fallback_impl(x); }
  };

}

#endif
