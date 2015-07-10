#include "ast.hpp"
#include "sass_values.h"
#include "to_value.hpp"
#include "to_string.hpp"
#include "debugger.hpp"

namespace Sass {
  using namespace std;

  Value* To_Value::fallback_impl(AST_Node* n)
  {
    To_String to_string(&ctx);
    return new (mem) String_Quoted(n->pstate(), n->perform(&to_string));
  }

  Value* To_Value::operator()(Boolean* b)
  {
    return b;
  }

  Value* To_Value::operator()(Number* n)
  {
    return n;
  }

  Value* To_Value::operator()(Color* c)
  {
    return c;
  }

  Value* To_Value::operator()(String_Constant* s)
  {
    return s;
  }

  Value* To_Value::operator()(String_Quoted* s)
  {
    return s;
  }

  Value* To_Value::operator()(List* l)
  {
    List* ll = new (mem) List(l->pstate(),
                              l->length(),
                              l->separator(),
                              l->is_arglist());
    for (size_t i = 0, L = l->length(); i < L; ++i) {
      *ll << (*l)[i]->perform(this);
    }
    return ll;
  }

  Value* To_Value::operator()(Map* m)
  {
    return m;
  }

  // not strictly necessary because of the fallback
  Value* To_Value::operator()(Null* n)
  {
    return n;
  }

};
