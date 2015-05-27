#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "ast.hpp"
#include "inspect.hpp"
#include "context.hpp"
#include "to_string.hpp"

namespace Sass {
  using namespace std;

  To_String::To_String(Context* ctx, bool in_declaration)
  : ctx(ctx), in_declaration(in_declaration) { }
  To_String::~To_String() { }

  inline string To_String::fallback_impl(AST_Node* n)
  {
    Emitter emitter(ctx);
    Inspect i(emitter);
    i.in_declaration = in_declaration;
    n->perform(&i);
    return i.get_buffer();
  }

  inline string To_String::operator()(String_Constant* s)
  {
    return s->value();
  }

  inline string To_String::operator()(Null* n)
  { return ""; }

  inline string To_String::operator()(List* list)
  {
    string str = "";
    string sep(list->separator() == List::SPACE ? " " : ",");
    if (list->empty()) return str;
    bool items_output = false;

    for (size_t i = 0, L = list->size(); i < L; ++i) {
      Expression* list_item = (*list)[i];
      if (list_item->is_invisible()) {
        continue;
      }
      if (items_output) str += sep;
      if (items_output && sep != " ") str += " ";
      if (list_item->concrete_type() == Expression::LIST) str += "(";
      str += list_item->perform(this);
      if (list_item->concrete_type() == Expression::LIST) str += ")";
      items_output = true;
    }
    return str;
  }
}
