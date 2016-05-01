#include "../sass.hpp"
#include "values.hpp"
#include "../ast.hpp"
#include "../context.hpp"
#include "../node.hpp"
#include "../extend.hpp"
#include "../emitter.hpp"
#include "../color_maps.hpp"
#include <set>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace Sass {

  bool Supports_Operator::needs_parens(Supports_Condition* cond) const {
    return dynamic_cast<Supports_Negation*>(cond) ||
          (dynamic_cast<Supports_Operator*>(cond) &&
           dynamic_cast<Supports_Operator*>(cond)->operand() != operand());
  }

  bool Supports_Negation::needs_parens(Supports_Condition* cond) const {
    return dynamic_cast<Supports_Negation*>(cond) ||
          dynamic_cast<Supports_Operator*>(cond);
  }


  bool At_Root_Query::exclude(std::string str)
  {
    bool with = feature() && unquote(feature()->to_string()).compare("with") == 0;
    List* l = static_cast<List*>(value());
    std::string v;

    if (with)
    {
      if (!l || l->length() == 0) return str.compare("rule") != 0;
      for (size_t i = 0, L = l->length(); i < L; ++i)
      {
        v = unquote((*l)[i]->to_string());
        if (v.compare("all") == 0 || v == str) return false;
      }
      return true;
    }
    else
    {
      if (!l || !l->length()) return str.compare("rule") == 0;
      for (size_t i = 0, L = l->length(); i < L; ++i)
      {
        v = unquote((*l)[i]->to_string());
        if (v.compare("all") == 0 || v == str) return true;
      }
      return false;
    }
  }


  void Argument::set_delayed(bool delayed)
  {
    if (value_) value_->set_delayed(delayed);
    is_delayed(delayed);
  }

  void Arguments::set_delayed(bool delayed)
  {
    for (Argument* arg : elements()) {
      if (arg) arg->set_delayed(delayed);
    }
    is_delayed(delayed);
  }



  Argument* Arguments::get_rest_argument()
  {
    Argument* arg = 0;
    if (this->has_rest_argument()) {
      for (auto a : this->elements()) {
        if (a->is_rest_argument()) {
          arg = a;
          break;
        }
      }
    }

    return arg;
  }

  Argument* Arguments::get_keyword_argument()
  {
    Argument* arg = 0;
    if (this->has_keyword_argument()) {
      for (auto a : this->elements()) {
        if (a->is_keyword_argument()) {
          arg = a;
          break;
        }
      }
    }

    return arg;
  }

  void Arguments::adjust_after_pushing(Argument* a)
  {
    if (!a->name().empty()) {
      if (/* has_rest_argument_ || */ has_keyword_argument_) {
        error("named arguments must precede variable-length argument", a->pstate());
      }
      has_named_arguments_ = true;
    }
    else if (a->is_rest_argument()) {
      if (has_rest_argument_) {
        error("functions and mixins may only be called with one variable-length argument", a->pstate());
      }
      if (has_keyword_argument_) {
        error("only keyword arguments may follow variable arguments", a->pstate());
      }
      has_rest_argument_ = true;
    }
    else if (a->is_keyword_argument()) {
      if (has_keyword_argument_) {
        error("functions and mixins may only be called with one keyword argument", a->pstate());
      }
      has_keyword_argument_ = true;
    }
    else {
      if (has_rest_argument_) {
        error("ordinal arguments must precede variable-length arguments", a->pstate());
      }
      if (has_named_arguments_) {
        error("ordinal arguments must precede named arguments", a->pstate());
      }
    }
  }

}