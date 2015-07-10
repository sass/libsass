#ifndef SASS_TO_VALUE_H
#define SASS_TO_VALUE_H

#include "operation.hpp"
#include "sass_values.h"

namespace Sass {
  using namespace std;

  class AST_Node;
  class Value;
  class Boolean;
  class Number;
  class Color;
  class String_Constant;
  class String_Quoted;
  class List;
  class Map;
  class Null;
  class Arguments;
  class Argument;

  class To_Value : public Operation_CRTP<Value*, To_Value> {

    Value* fallback_impl(AST_Node* n);

  private:

    Context& ctx;
    Memory_Manager<AST_Node>& mem;

  public:

    To_Value(Context& ctx, Memory_Manager<AST_Node>& mem)
    : ctx(ctx), mem(mem)
    { }
    virtual ~To_Value() { }
    using Operation<Value*>::operator();

    Value* operator()(Boolean*);
    Value* operator()(Number*);
    Value* operator()(Color*);
    Value* operator()(String_Constant*);
    Value* operator()(String_Quoted*);
    Value* operator()(List*);
    Value* operator()(Map*);
    Value* operator()(Null*);

    template <typename U>
    Value* fallback(U x) { return fallback_impl(x); }
  };

}

#endif
