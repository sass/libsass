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


  Expression* Listize::operator()(Binary_Expression* b)
  {
    string sep("/");
    To_String to_string(&ctx);
    string lstr(b->left()->perform(&to_string));
    string rstr(b->right()->perform(&to_string));
    return new (ctx.mem) String_Quoted(b->pstate(), lstr + sep + rstr);
  }

  Expression* Listize::operator()(Argument* arg)
  {
    return arg->value()->perform(this);
  }

*/


  Expression* Listize::operator()(Selector_List* sel)
  {
    List* l = new (ctx.mem) List(sel->pstate(), sel->length(), SASS_COMMA);
    for (size_t i = 0, L = sel->length(); i < L; ++i) {
      if (!(*sel)[i]) continue;
      *l << (*sel)[i]->perform(this);
    }
    return l;
  }

  Expression* Listize::operator()(Compound_Selector* sel)
  {
    To_String to_string;
    string str;
    for (size_t i = 0, L = sel->length(); i < L; ++i) {
      Expression* e = (*sel)[i]->perform(this);
      if (e) str += e->perform(&to_string);
    }
    return new (ctx.mem) String_Quoted(sel->pstate(), str);
  }

  Expression* Listize::operator()(Complex_Selector* sel)
  {
    List* l = new (ctx.mem) List(sel->pstate(), 2);

    Compound_Selector* head = sel->head();
    if (head && !head->is_empty_reference())
    {
      Expression* hh = head->perform(this);
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
      Expression* tt = tail->perform(this);
      if (tt && tt->concrete_type() == Expression::LIST)
      { *l += dynamic_cast<List*>(tt); }
      else if (tt) *l << dynamic_cast<List*>(tt);
    }
    if (l->length() == 0) return 0;
    return l;
  }

/*
  Expression* Listize::operator()(Parent_Selector* sel)
  {
    return 0;
  }
  Expression* Listize::operator()(Expression* val)
  {
    return val;
  }
*/

  Expression* Listize::operator()(Type_Selector* sel)
  {
    return new (ctx.mem) String_Quoted(sel->pstate(), sel->ns_name());
  }

  Expression* Listize::operator()(Selector_Qualifier* sel)
  {
    return new (ctx.mem) String_Quoted(sel->pstate(), sel->ns_name());
  }

  Expression* Listize::fallback_impl(AST_Node* n)
  {
    Expression* val = dynamic_cast<Expression*>(n);
    if (!val) debug_ast(n);
    return val;
  }

}
