#include <iostream>
#include <typeinfo>

#include "listize.hpp"
#include "to_string.hpp"
#include "context.hpp"
#include "backtrace.hpp"
#include "error_handling.hpp"
#include "debugger.hpp"

namespace Sass {

  Listize::Listize(Context& ctx)
  : ctx(ctx)
  {  }
/*


  Value* Listize::operator()(Binary_Expression* b)
  {
    string sep("/");
    To_String to_string(&ctx);
    string lstr(b->left()->perform(&to_string));
    string rstr(b->right()->perform(&to_string));
    return new (ctx.mem) String_Quoted(b->pstate(), lstr + sep + rstr);
  }

  Value* Listize::operator()(Argument* arg)
  {
    return arg->value()->perform(this);
  }

*/


  Value* Listize::operator()(Selector_List* sel)
  {
    List* l = new (ctx.mem) List(sel->pstate(), sel->length(), List::COMMA);
    for (size_t i = 0, L = sel->length(); i < L; ++i) {
      if (!(*sel)[i]) continue;
      *l << (*sel)[i]->perform(this);
    }
    return l;
  }

  Value* Listize::operator()(Compound_Selector* sel)
  {
    To_String to_string;
    string str;
    for (size_t i = 0, L = sel->length(); i < L; ++i) {
      Expression* e = (*sel)[i]->perform(this);
      if (e) str += e->perform(&to_string);
    }
    return new (ctx.mem) String_Quoted(sel->pstate(), str);
  }

  Value* Listize::operator()(Complex_Selector* sel)
  {
    List* l = new (ctx.mem) List(sel->pstate(), 2);

    Compound_Selector* head = sel->head();
    if (head && !head->is_empty_reference())
    {
      Value* hh = head->perform(this);
      if (hh) *l << hh;
    }

    To_String to_string;
    string reference = ! sel->reference() ? ""
      : sel->reference()->perform(&to_string);
    switch(sel->combinator())
    {
      case Complex_Selector::PARENT_OF:
        *l << new (ctx.mem) String_Quoted(sel->pstate(), ">");
      break;
      case Complex_Selector::ADJACENT_TO:
        *l << new (ctx.mem) String_Quoted(sel->pstate(), "+");
      break;
      case Complex_Selector::REFERENCE:
        *l << new (ctx.mem) String_Quoted(sel->pstate(), "/" + reference + "/");
      break;
      case Complex_Selector::PRECEDES:
        *l << new (ctx.mem) String_Quoted(sel->pstate(), "~");
      break;
      case Complex_Selector::ANCESTOR_OF:
      break;
    }

    Complex_Selector* tail = sel->tail();
    if (tail)
    {
      Value* tt = tail->perform(this);
      if (tt && tt->concrete_type() == Expression::LIST)
      { *l += static_cast<List*>(tt); }
      else if (tt) *l << static_cast<List*>(tt);
    }
    if (l->length() == 0) return 0;
    return l;
  }

/*
  Value* Listize::operator()(Parent_Selector* sel)
  {
    return 0;
  }
  Value* Listize::operator()(Value* val)
  {
    return val;
  }
*/

  Value* Listize::operator()(Type_Selector* sel)
  {
    return new (ctx.mem) String_Quoted(sel->pstate(), sel->ns_name());
  }

  Value* Listize::operator()(Selector_Qualifier* sel)
  {
    return new (ctx.mem) String_Quoted(sel->pstate(), sel->ns_name());
  }

  Value* Listize::fallback_impl(AST_Node* n)
  {
    Value* val = static_cast<Value*>(n);
    if (!val) debug_ast(n);
    return val;
  }

}
