#ifndef SASS_LISTIZE_H
#define SASS_LISTIZE_H

#include <vector>
#include <iostream>

#include "ast.hpp"
#include "context.hpp"
#include "operation.hpp"
#include "environment.hpp"

namespace Sass {

  struct Backtrace;

  class Listize : public Operation_CRTP<Expression*, Listize> {

  public:
    Listize();
    ~Listize() { }

    Expression* operator()(Selector_List*);
    Expression* operator()(Complex_Selector*);
    Expression* operator()(Compound_Selector*);

    // generic fallback
    template <typename U>
    Expression* fallback(U x)
    { return Cast<Expression>(x); }
  };

}

#endif
