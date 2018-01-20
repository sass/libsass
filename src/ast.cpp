#include "sass.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "node.hpp"
#include "eval.hpp"
#include "extend.hpp"
#include "debugger.hpp"
#include "emitter.hpp"
#include "color_maps.hpp"
#include "ast_fwd_decl.hpp"
#include <set>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace Sass {

  static Null sass_null(ParserState("null"));

  bool Wrapped_Selector::find ( bool (*f)(AST_Node_Obj) )
  {
    // check children first
    if (selector_) {
      if (selector_->find(f)) return true;
    }
    // execute last
    return f(this);
  }

  bool Selector_List::find ( bool (*f)(AST_Node_Obj) )
  {
    // check children first
    for (Complex_Selector_Obj sel : elements()) {
      if (sel->find(f)) return true;
    }
    // execute last
    return f(this);
  }

  bool Compound_Selector::find ( bool (*f)(AST_Node_Obj) )
  {
    // check children first
    for (Simple_Selector_Obj sel : elements()) {
      if (sel->find(f)) return true;
    }
    // execute last
    return f(this);
  }

  bool Complex_Selector::find ( bool (*f)(AST_Node_Obj) )
  {
    // check children first
    if (head_ && head_->find(f)) return true;
    if (tail_ && tail_->find(f)) return true;
    // execute last
    return f(this);
  }

  bool Supports_Operator::needs_parens(Supports_Condition_Obj cond) const {
    if (Supports_Operator_Obj op = Cast<Supports_Operator>(cond)) {
      return op->operand() != operand();
    }
    return Cast<Supports_Negation>(cond) != NULL;
  }

  bool Supports_Negation::needs_parens(Supports_Condition_Obj cond) const {
    return Cast<Supports_Negation>(cond) ||
           Cast<Supports_Operator>(cond);
  }

  void str_rtrim(std::string& str, const std::string& delimiters = " \f\n\r\t\v")
  {
    str.erase( str.find_last_not_of( delimiters ) + 1 );
  }

  void String_Constant::rtrim()
  {
    str_rtrim(value_);
  }

  void String_Schema::rtrim()
  {
    if (!empty()) {
      if (String_Ptr str = Cast<String>(last())) str->rtrim();
    }
  }

  void Argument::set_delayed(bool delayed)
  {
    if (value_) value_->set_delayed(delayed);
    is_delayed(delayed);
  }

  void Arguments::set_delayed(bool delayed)
  {
    for (Argument_Obj arg : elements()) {
      if (arg) arg->set_delayed(delayed);
    }
    is_delayed(delayed);
  }


  bool At_Root_Query::exclude(std::string str)
  {
    bool with = feature() && unquote(feature()->to_string()).compare("with") == 0;
    List_Ptr l = static_cast<List_Ptr>(value().ptr());
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

  void AST_Node::update_pstate(const ParserState& pstate)
  {
    pstate_.offset += pstate - pstate_ + pstate.offset;
  }

  bool Simple_Selector::is_ns_eq(const Simple_Selector& r) const
  {
    // https://github.com/sass/sass/issues/2229
    if ((has_ns_ == r.has_ns_) ||
        (has_ns_ && ns_.empty()) ||
        (r.has_ns_ && r.ns_.empty())
    ) {
      if (ns_.empty() && r.ns() == "*") return false;
      else if (r.ns().empty() && ns() == "*") return false;
      else return ns() == r.ns();
    }
    return false;
  }

  bool Compound_Selector::has_parent_ref() const
  {
    for (Simple_Selector_Obj s : *this) {
      if (s && s->has_parent_ref()) return true;
    }
    return false;
  }

  bool Compound_Selector::has_real_parent_ref() const
  {
    for (Simple_Selector_Obj s : *this) {
      if (s && s->has_real_parent_ref()) return true;
    }
    return false;
  }

  bool Complex_Selector::has_parent_ref() const
  {
    return (head() && head()->has_parent_ref()) ||
           (tail() && tail()->has_parent_ref());
  }

  bool Complex_Selector::has_real_parent_ref() const
  {
    return (head() && head()->has_real_parent_ref()) ||
           (tail() && tail()->has_real_parent_ref());
  }



  // create complex selector (ancestor of) from compound selector
  Complex_Selector_Obj Compound_Selector::to_complex()
  {
    // create an intermediate complex selector
    return SASS_MEMORY_NEW(Complex_Selector,
                           pstate(),
                           Complex_Selector::ANCESTOR_OF,
                           this,
                           0);
  }



  size_t Complex_Selector::length() const
  {
    // TODO: make this iterative
    if (!tail()) return 1;
    return 1 + tail()->length();
  }

  // append another complex selector at the end
  // check if we need to append some headers
  // then we need to check for the combinator
  // only then we can safely set the new tail
  void Complex_Selector::append(Complex_Selector_Obj ss)
  {

    Complex_Selector_Obj t = ss->tail();
    Combinator c = ss->combinator();
    String_Obj r = ss->reference();
    Compound_Selector_Obj h = ss->head();

    if (ss->has_line_feed()) has_line_feed(true);
    if (ss->has_line_break()) has_line_break(true);

    // append old headers
    if (h && h->length()) {
      if (last()->combinator() != ANCESTOR_OF && c != ANCESTOR_OF) {
        error("Invalid parent selector", pstate_);
      } else if (last()->head_ && last()->head_->length()) {
        Compound_Selector_Obj rh = last()->head();
        size_t i;
        size_t L = h->length();
        if (Cast<Element_Selector>(h->first())) {
          if (Class_Selector_Ptr cs = Cast<Class_Selector>(rh->last())) {
            Class_Selector_Ptr sqs = SASS_MEMORY_COPY(cs);
            sqs->name(sqs->name() + (*h)[0]->name());
            sqs->pstate((*h)[0]->pstate());
            (*rh)[rh->length()-1] = sqs;
            rh->pstate(h->pstate());
            for (i = 1; i < L; ++i) rh->append((*h)[i]);
          } else if (Id_Selector_Ptr is = Cast<Id_Selector>(rh->last())) {
            Id_Selector_Ptr sqs = SASS_MEMORY_COPY(is);
            sqs->name(sqs->name() + (*h)[0]->name());
            sqs->pstate((*h)[0]->pstate());
            (*rh)[rh->length()-1] = sqs;
            rh->pstate(h->pstate());
            for (i = 1; i < L; ++i) rh->append((*h)[i]);
          } else if (Element_Selector_Ptr ts = Cast<Element_Selector>(rh->last())) {
            Element_Selector_Ptr tss = SASS_MEMORY_COPY(ts);
            tss->name(tss->name() + (*h)[0]->name());
            tss->pstate((*h)[0]->pstate());
            (*rh)[rh->length()-1] = tss;
            rh->pstate(h->pstate());
            for (i = 1; i < L; ++i) rh->append((*h)[i]);
          } else if (Placeholder_Selector_Ptr ps = Cast<Placeholder_Selector>(rh->last())) {
            Placeholder_Selector_Ptr pss = SASS_MEMORY_COPY(ps);
            pss->name(pss->name() + (*h)[0]->name());
            pss->pstate((*h)[0]->pstate());
            (*rh)[rh->length()-1] = pss;
            rh->pstate(h->pstate());
            for (i = 1; i < L; ++i) rh->append((*h)[i]);
          } else {
            last()->head_->concat(h);
          }
        } else {
          last()->head_->concat(h);
        }
      } else if (last()->head_) {
        last()->head_->concat(h);
      }
    } else {
      // std::cerr << "has no or empty head\n";
    }

    if (last()) {
      if (last()->combinator() != ANCESTOR_OF && c != ANCESTOR_OF) {
        Complex_Selector_Ptr inter = SASS_MEMORY_NEW(Complex_Selector, pstate());
        inter->reference(r);
        inter->combinator(c);
        inter->tail(t);
        last()->tail(inter);
      } else {
        if (last()->combinator() == ANCESTOR_OF) {
          last()->combinator(c);
          last()->reference(r);
        }
        last()->tail(t);
      }
    }

  }

  Selector_List_Obj Selector_List::eval(Eval& eval)
  {
    Selector_List_Obj list = schema() ?
      eval(schema()) : eval(this);
    list->schema(schema());
    return list;
  }

  Selector_List_Ptr Selector_List::resolve_parent_refs(std::vector<Selector_List_Obj>& pstack, bool implicit_parent)
  {
    if (!this->has_parent_ref()) return this;
    Selector_List_Ptr ss = SASS_MEMORY_NEW(Selector_List, pstate());
    Selector_List_Ptr ps = pstack.back();
    for (size_t pi = 0, pL = ps->length(); pi < pL; ++pi) {
      for (size_t si = 0, sL = this->length(); si < sL; ++si) {
        Selector_List_Obj rv = at(si)->resolve_parent_refs(pstack, implicit_parent);
        ss->concat(rv);
      }
    }
    return ss;
  }

  Selector_List_Ptr Complex_Selector::resolve_parent_refs(std::vector<Selector_List_Obj>& pstack, bool implicit_parent)
  {
    Complex_Selector_Obj tail = this->tail();
    Compound_Selector_Obj head = this->head();
    Selector_List_Ptr parents = pstack.back();

    if (!this->has_real_parent_ref() && !implicit_parent) {
      Selector_List_Ptr retval = SASS_MEMORY_NEW(Selector_List, pstate());
      retval->append(this);
      return retval;
    }

    // first resolve_parent_refs the tail (which may return an expanded list)
    Selector_List_Obj tails = tail ? tail->resolve_parent_refs(pstack, implicit_parent) : 0;

    if (head && head->length() > 0) {

      Selector_List_Obj retval;
      // we have a parent selector in a simple compound list
      // mix parent complex selector into the compound list
      if (Cast<Parent_Selector>((*head)[0])) {
        retval = SASS_MEMORY_NEW(Selector_List, pstate());

        // it turns out that real parent references reach
        // across @at-root rules, which comes unexpected
        if (parents == NULL && head->has_real_parent_ref()) {
          int i = pstack.size() - 1;
          while (!parents && i > -1) {
            parents = pstack.at(i--);
          }
        }

        if (parents && parents->length()) {
          if (tails && tails->length() > 0) {
            for (size_t n = 0, nL = tails->length(); n < nL; ++n) {
              for (size_t i = 0, iL = parents->length(); i < iL; ++i) {
                Complex_Selector_Obj t = (*tails)[n];
                Complex_Selector_Obj parent = (*parents)[i];
                Complex_Selector_Obj s = SASS_MEMORY_CLONE(parent);
                Complex_Selector_Obj ss = SASS_MEMORY_CLONE(this);
                ss->tail(t ? SASS_MEMORY_CLONE(t) : NULL);
                Compound_Selector_Obj h = SASS_MEMORY_COPY(head_);
                // remove parent selector from sequence
                if (h->length()) {
                  h->erase(h->begin());
                  ss->head(h);
                } else {
                  ss->head(NULL);
                }
                // adjust for parent selector (1 char)
                // if (h->length()) {
                //   ParserState state(h->at(0)->pstate());
                //   state.offset.column += 1;
                //   state.column -= 1;
                //   (*h)[0]->pstate(state);
                // }
                // keep old parser state
                s->pstate(pstate());
                // append new tail
                s->append(ss);
                retval->append(s);
              }
            }
          }
          // have no tails but parents
          // loop above is inside out
          else {
            for (size_t i = 0, iL = parents->length(); i < iL; ++i) {
              Complex_Selector_Obj parent = (*parents)[i];
              Complex_Selector_Obj s = SASS_MEMORY_CLONE(parent);
              Complex_Selector_Obj ss = SASS_MEMORY_CLONE(this);
              // this is only if valid if the parent has no trailing op
              // otherwise we cannot append more simple selectors to head
              if (parent->last()->combinator() != ANCESTOR_OF) {
                throw Exception::InvalidParent(parent, ss);
              }
              ss->tail(tail ? SASS_MEMORY_CLONE(tail) : NULL);
              Compound_Selector_Obj h = SASS_MEMORY_COPY(head_);
              // remove parent selector from sequence
              if (h->length()) {
                h->erase(h->begin());
                ss->head(h);
              } else {
                ss->head(NULL);
              }
              // \/ IMO ruby sass bug \/
              ss->has_line_feed(false);
              // adjust for parent selector (1 char)
              // if (h->length()) {
              //   ParserState state(h->at(0)->pstate());
              //   state.offset.column += 1;
              //   state.column -= 1;
              //   (*h)[0]->pstate(state);
              // }
              // keep old parser state
              s->pstate(pstate());
              // append new tail
              s->append(ss);
              retval->append(s);
            }
          }
        }
        // have no parent but some tails
        else {
          if (tails && tails->length() > 0) {
            for (size_t n = 0, nL = tails->length(); n < nL; ++n) {
              Complex_Selector_Obj cpy = SASS_MEMORY_CLONE(this);
              cpy->tail(SASS_MEMORY_CLONE(tails->at(n)));
              cpy->head(SASS_MEMORY_NEW(Compound_Selector, head->pstate()));
              for (size_t i = 1, L = this->head()->length(); i < L; ++i)
                cpy->head()->append((*this->head())[i]);
              if (!cpy->head()->length()) cpy->head(0);
              retval->append(cpy->skip_empty_reference());
            }
          }
          // have no parent nor tails
          else {
            Complex_Selector_Obj cpy = SASS_MEMORY_CLONE(this);
            cpy->head(SASS_MEMORY_NEW(Compound_Selector, head->pstate()));
            for (size_t i = 1, L = this->head()->length(); i < L; ++i)
              cpy->head()->append((*this->head())[i]);
            if (!cpy->head()->length()) cpy->head(0);
            retval->append(cpy->skip_empty_reference());
          }
        }
      }
      // no parent selector in head
      else {
        retval = this->tails(tails);
      }

      for (Simple_Selector_Obj ss : head->elements()) {
        if (Wrapped_Selector_Ptr ws = Cast<Wrapped_Selector>(ss)) {
          if (Selector_List_Ptr sl = Cast<Selector_List>(ws->selector())) {
            if (parents) ws->selector(sl->resolve_parent_refs(pstack, implicit_parent));
          }
        }
      }

      return retval.detach();

    }
    // has no head
    return this->tails(tails);
  }

  Selector_List_Ptr Complex_Selector::tails(Selector_List_Ptr tails)
  {
    Selector_List_Ptr rv = SASS_MEMORY_NEW(Selector_List, pstate_);
    if (tails && tails->length()) {
      for (size_t i = 0, iL = tails->length(); i < iL; ++i) {
        Complex_Selector_Obj pr = SASS_MEMORY_CLONE(this);
        pr->tail(tails->at(i));
        rv->append(pr);
      }
    }
    else {
      rv->append(this);
    }
    return rv;
  }

  // return the last tail that is defined
  Complex_Selector_Obj Complex_Selector::first()
  {
    // declare variables used in loop
    Complex_Selector_Obj cur = this;
    Compound_Selector_Obj head;
    // processing loop
    while (cur)
    {
      // get the head
      head = cur->head_;
      // abort (and return) if it is not a parent selector
      if (!head || head->length() != 1 || !Cast<Parent_Selector>((*head)[0])) {
        break;
      }
      // advance to next
      cur = cur->tail_;
    }
    // result
    return cur;
  }

  // return the last tail that is defined
  Complex_Selector_Obj Complex_Selector::last()
  {
    Complex_Selector_Ptr cur = this;
    Complex_Selector_Ptr nxt = cur;
    // loop until last
    while (nxt) {
      cur = nxt;
      nxt = cur->tail();
    }
    return cur;
  }

  Complex_Selector::Combinator Complex_Selector::clear_innermost()
  {
    Combinator c;
    if (!tail() || tail()->tail() == 0)
    { c = combinator(); combinator(ANCESTOR_OF); tail(0); }
    else
    { c = tail()->clear_innermost(); }
    return c;
  }

  void Complex_Selector::set_innermost(Complex_Selector_Obj val, Combinator c)
  {
    if (!tail())
    { tail(val); combinator(c); }
    else
    { tail()->set_innermost(val, c); }
  }

  void Complex_Selector::cloneChildren()
  {
    if (head()) head(SASS_MEMORY_CLONE(head()));
    if (tail()) tail(SASS_MEMORY_CLONE(tail()));
  }

  void Compound_Selector::cloneChildren()
  {
    for (size_t i = 0, l = length(); i < l; i++) {
      at(i) = SASS_MEMORY_CLONE(at(i));
    }
  }

  void Selector_List::cloneChildren()
  {
    for (size_t i = 0, l = length(); i < l; i++) {
      at(i) = SASS_MEMORY_CLONE(at(i));
    }
  }

  void Wrapped_Selector::cloneChildren()
  {
    selector(SASS_MEMORY_CLONE(selector()));
  }

  // remove parent selector references
  // basically unwraps parsed selectors
  void Selector_List::remove_parent_selectors()
  {
    // Check every rhs selector against left hand list
    for(size_t i = 0, L = length(); i < L; ++i) {
      if (!(*this)[i]->head()) continue;
      if ((*this)[i]->head()->is_empty_reference()) {
        // simply move to the next tail if we have "no" combinator
        if ((*this)[i]->combinator() == Complex_Selector::ANCESTOR_OF) {
          if ((*this)[i]->tail()) {
            if ((*this)[i]->has_line_feed()) {
              (*this)[i]->tail()->has_line_feed(true);
            }
            (*this)[i] = (*this)[i]->tail();
          }
        }
        // otherwise remove the first item from head
        else {
          (*this)[i]->head()->erase((*this)[i]->head()->begin());
        }
      }
    }
  }

  size_t Wrapped_Selector::hash()
  {
    if (hash_ == 0) {
      hash_combine(hash_, Simple_Selector::hash());
      if (selector_) hash_combine(hash_, selector_->hash());
    }
    return hash_;
  }
  bool Wrapped_Selector::has_parent_ref() const {
    // if (has_reference()) return true;
    if (!selector()) return false;
    return selector()->has_parent_ref();
  }
  bool Wrapped_Selector::has_real_parent_ref() const {
    // if (has_reference()) return true;
    if (!selector()) return false;
    return selector()->has_real_parent_ref();
  }
  unsigned long Wrapped_Selector::specificity() const
  {
    return selector_ ? selector_->specificity() : 0;
  }


  bool Selector_List::has_parent_ref() const
  {
    for (Complex_Selector_Obj s : elements()) {
      if (s && s->has_parent_ref()) return true;
    }
    return false;
  }

  bool Selector_List::has_real_parent_ref() const
  {
    for (Complex_Selector_Obj s : elements()) {
      if (s && s->has_real_parent_ref()) return true;
    }
    return false;
  }

  bool Selector_Schema::has_parent_ref() const
  {
    if (String_Schema_Obj schema = Cast<String_Schema>(contents())) {
      return schema->length() > 0 && Cast<Parent_Selector>(schema->at(0)) != NULL;
    }
    return false;
  }

  bool Selector_Schema::has_real_parent_ref() const
  {
    if (String_Schema_Obj schema = Cast<String_Schema>(contents())) {
      Parent_Selector_Obj p = Cast<Parent_Selector>(schema->at(0));
      return schema->length() > 0 && p && p->is_real_parent_ref();
    }
    return false;
  }

  void Selector_List::adjust_after_pushing(Complex_Selector_Obj c)
  {
    // if (c->has_reference())   has_reference(true);
  }


  void Selector_List::populate_extends(Selector_List_Obj extendee, Subset_Map& extends)
  {

    Selector_List_Ptr extender = this;
    for (auto complex_sel : extendee->elements()) {
      Complex_Selector_Obj c = complex_sel;


      // Ignore any parent selectors, until we find the first non Selectorerence head
      Compound_Selector_Obj compound_sel = c->head();
      Complex_Selector_Obj pIter = complex_sel;
      while (pIter) {
        Compound_Selector_Obj pHead = pIter->head();
        if (pHead && Cast<Parent_Selector>(pHead->elements()[0]) == NULL) {
          compound_sel = pHead;
          break;
        }

        pIter = pIter->tail();
      }

      if (!pIter->head() || pIter->tail()) {
        error("nested selectors may not be extended", c->pstate());
      }

      compound_sel->is_optional(extendee->is_optional());

      for (size_t i = 0, L = extender->length(); i < L; ++i) {
        extends.put(compound_sel, std::make_pair((*extender)[i], compound_sel));
      }
    }
  };

  void Compound_Selector::append(Simple_Selector_Ptr element)
  {
    Vectorized<Simple_Selector_Obj>::append(element);
    pstate_.offset += element->pstate().offset;
  }

  Compound_Selector_Ptr Compound_Selector::minus(Compound_Selector_Ptr rhs)
  {
    Compound_Selector_Ptr result = SASS_MEMORY_NEW(Compound_Selector, pstate());
    // result->has_parent_reference(has_parent_reference());

    // not very efficient because it needs to preserve order
    for (size_t i = 0, L = length(); i < L; ++i)
    {
      bool found = false;
      for (size_t j = 0, M = rhs->length(); j < M; ++j)
      {
        if (*(*this)[i] == *(*rhs)[j])
        {
          found = true;
          break;
        }
      }
      if (!found) result->append((*this)[i]);
    }

    return result;
  }

  void Compound_Selector::mergeSources(ComplexSelectorSet& sources)
  {
    for (ComplexSelectorSet::iterator iterator = sources.begin(), endIterator = sources.end(); iterator != endIterator; ++iterator) {
      this->sources_.insert(SASS_MEMORY_CLONE(*iterator));
    }
  }

  Argument_Obj Arguments::get_rest_argument()
  {
    if (this->has_rest_argument()) {
      for (Argument_Obj arg : this->elements()) {
        if (arg->is_rest_argument()) {
          return arg;
        }
      }
    }
    return NULL;
  }

  Argument_Obj Arguments::get_keyword_argument()
  {
    if (this->has_keyword_argument()) {
      for (Argument_Obj arg : this->elements()) {
        if (arg->is_keyword_argument()) {
          return arg;
        }
      }
    }
    return NULL;
  }

  void Arguments::adjust_after_pushing(Argument_Obj a)
  {
    if (!a->name().empty()) {
      if (has_keyword_argument()) {
        error("named arguments must precede variable-length argument", a->pstate());
      }
      has_named_arguments(true);
    }
    else if (a->is_rest_argument()) {
      if (has_rest_argument()) {
        error("functions and mixins may only be called with one variable-length argument", a->pstate());
      }
      if (has_keyword_argument_) {
        error("only keyword arguments may follow variable arguments", a->pstate());
      }
      has_rest_argument(true);
    }
    else if (a->is_keyword_argument()) {
      if (has_keyword_argument()) {
        error("functions and mixins may only be called with one keyword argument", a->pstate());
      }
      has_keyword_argument(true);
    }
    else {
      if (has_rest_argument()) {
        error("ordinal arguments must precede variable-length arguments", a->pstate());
      }
      if (has_named_arguments()) {
        error("ordinal arguments must precede named arguments", a->pstate());
      }
    }
  }

  bool Ruleset::is_invisible() const {
    if (Selector_List_Ptr sl = Cast<Selector_List>(selector())) {
      for (size_t i = 0, L = sl->length(); i < L; ++i)
        if (!(*sl)[i]->has_placeholder()) return false;
    }
    return true;
  }

  bool Media_Block::is_invisible() const {
    for (size_t i = 0, L = block()->length(); i < L; ++i) {
      Statement_Obj stm = block()->at(i);
      if (!stm->is_invisible()) return false;
    }
    return true;
  }

  Number::Number(ParserState pstate, double val, std::string u, bool zero)
  : Value(pstate),
    Units(),
    value_(val),
    zero_(zero),
    hash_(0)
  {
    size_t l = 0;
    size_t r;
    if (!u.empty()) {
      bool nominator = true;
      while (true) {
        r = u.find_first_of("*/", l);
        std::string unit(u.substr(l, r == std::string::npos ? r : r - l));
        if (!unit.empty()) {
          if (nominator) numerators.push_back(unit);
          else denominators.push_back(unit);
        }
        if (r == std::string::npos) break;
        // ToDo: should error for multiple slashes
        // if (!nominator && u[r] == '/') error(...)
        if (u[r] == '/')
          nominator = false;
        // strange math parsing?
        // else if (u[r] == '*')
        //  nominator = true;
        l = r + 1;
      }
    }
    concrete_type(NUMBER);
  }

  // cancel out unnecessary units
  void Number::reduce()
  {
    // apply conversion factor
    value_ *= this->Units::reduce();
  }

  void Number::normalize()
  {
    // apply conversion factor
    value_ *= this->Units::normalize();
  }

  bool Custom_Warning::operator== (const Expression& rhs) const
  {
    if (Custom_Warning_Ptr_Const r = Cast<Custom_Warning>(&rhs)) {
      return message() == r->message();
    }
    return false;
  }

  bool Custom_Error::operator== (const Expression& rhs) const
  {
    if (Custom_Error_Ptr_Const r = Cast<Custom_Error>(&rhs)) {
      return message() == r->message();
    }
    return false;
  }

  bool Number::operator== (const Expression& rhs) const
  {
    if (auto rhsnr = Cast<Number>(&rhs)) {
      return *this == *rhsnr;
    }
    return false;
  }

  bool Number::operator== (const Number& rhs) const
  {
    Number l(*this), r(rhs); l.reduce(); r.reduce();
    size_t lhs_units = l.numerators.size() + l.denominators.size();
    size_t rhs_units = r.numerators.size() + r.denominators.size();
    // unitless and only having one unit seems equivalent (will change in future)
    if (!lhs_units || !rhs_units) {
      return NEAR_EQUAL(l.value(), r.value());
    }
    l.normalize(); r.normalize();
    Units &lhs_unit = l, &rhs_unit = r;
    return lhs_unit == rhs_unit &&
      NEAR_EQUAL(l.value(), r.value());
  }

  bool Number::operator< (const Number& rhs) const
  {
    Number l(*this), r(rhs); l.reduce(); r.reduce();
    size_t lhs_units = l.numerators.size() + l.denominators.size();
    size_t rhs_units = r.numerators.size() + r.denominators.size();
    // unitless and only having one unit seems equivalent (will change in future)
    if (!lhs_units || !rhs_units) {
      return l.value() < r.value();
    }
    l.normalize(); r.normalize();
    Units &lhs_unit = l, &rhs_unit = r;
    if (!(lhs_unit == rhs_unit)) {
      throw Exception::IncompatibleUnits(rhs, *this);
    }
    return lhs_unit < rhs_unit ||
           l.value() < r.value();
  }

  bool String_Quoted::operator== (const Expression& rhs) const
  {
    if (String_Quoted_Ptr_Const qstr = Cast<String_Quoted>(&rhs)) {
      return (value() == qstr->value());
    } else if (String_Constant_Ptr_Const cstr = Cast<String_Constant>(&rhs)) {
      return (value() == cstr->value());
    }
    return false;
  }

  bool String_Constant::is_invisible() const {
    return value_.empty() && quote_mark_ == 0;
  }

  bool String_Constant::operator== (const Expression& rhs) const
  {
    if (String_Quoted_Ptr_Const qstr = Cast<String_Quoted>(&rhs)) {
      return (value() == qstr->value());
    } else if (String_Constant_Ptr_Const cstr = Cast<String_Constant>(&rhs)) {
      return (value() == cstr->value());
    }
    return false;
  }

  bool String_Schema::is_left_interpolant(void) const
  {
    return length() && first()->is_left_interpolant();
  }
  bool String_Schema::is_right_interpolant(void) const
  {
    return length() && last()->is_right_interpolant();
  }

  bool String_Schema::operator== (const Expression& rhs) const
  {
    if (String_Schema_Ptr_Const r = Cast<String_Schema>(&rhs)) {
      if (length() != r->length()) return false;
      for (size_t i = 0, L = length(); i < L; ++i) {
        Expression_Obj rv = (*r)[i];
        Expression_Obj lv = (*this)[i];
        if (!lv || !rv) return false;
        if (!(*lv == *rv)) return false;
      }
      return true;
    }
    return false;
  }

  bool Boolean::operator== (const Expression& rhs) const
  {
    if (Boolean_Ptr_Const r = Cast<Boolean>(&rhs)) {
      return (value() == r->value());
    }
    return false;
  }

  bool Color::operator== (const Expression& rhs) const
  {
    if (Color_Ptr_Const r = Cast<Color>(&rhs)) {
      return r_ == r->r() &&
             g_ == r->g() &&
             b_ == r->b() &&
             a_ == r->a();
    }
    return false;
  }

  bool List::operator== (const Expression& rhs) const
  {
    if (List_Ptr_Const r = Cast<List>(&rhs)) {
      if (length() != r->length()) return false;
      if (separator() != r->separator()) return false;
      if (is_bracketed() != r->is_bracketed()) return false;
      for (size_t i = 0, L = length(); i < L; ++i) {
        Expression_Obj rv = r->at(i);
        Expression_Obj lv = this->at(i);
        if (!lv || !rv) return false;
        if (!(*lv == *rv)) return false;
      }
      return true;
    }
    return false;
  }

  bool Map::operator== (const Expression& rhs) const
  {
    if (Map_Ptr_Const r = Cast<Map>(&rhs)) {
      if (length() != r->length()) return false;
      for (auto key : keys()) {
        Expression_Obj lv = at(key);
        Expression_Obj rv = r->at(key);
        if (!rv || !lv) return false;
        if (!(*lv == *rv)) return false;
      }
      return true;
    }
    return false;
  }

  bool Null::operator== (const Expression& rhs) const
  {
    return rhs.concrete_type() == NULL_VAL;
  }

  bool Function::operator== (const Expression& rhs) const
  {
    if (Function_Ptr_Const r = Cast<Function>(&rhs)) {
      Definition_Ptr_Const d1 = Cast<Definition>(definition());
      Definition_Ptr_Const d2 = Cast<Definition>(r->definition());
      return d1 && d2 && d1 == d2 && is_css() == r->is_css();
    }
    return false;
  }

  size_t List::size() const {
    if (!is_arglist_) return length();
    // arglist expects a list of arguments
    // so we need to break before keywords
    for (size_t i = 0, L = length(); i < L; ++i) {
      Expression_Obj obj = this->at(i);
      if (Argument_Ptr arg = Cast<Argument>(obj)) {
        if (!arg->name().empty()) return i;
      }
    }
    return length();
  }

  Expression_Obj Hashed::at(Expression_Obj k) const
  {
    if (elements_.count(k))
    { return elements_.at(k); }
    else { return NULL; }
  }

  bool Binary_Expression::is_left_interpolant(void) const
  {
    return is_interpolant() || (left() && left()->is_left_interpolant());
  }
  bool Binary_Expression::is_right_interpolant(void) const
  {
    return is_interpolant() || (right() && right()->is_right_interpolant());
  }

  const std::string AST_Node::to_string(Sass_Inspect_Options opt) const
  {
    Sass_Output_Options out(opt);
    Emitter emitter(out);
    Inspect i(emitter);
    i.in_declaration = true;
    // ToDo: inspect should be const
    const_cast<AST_Node_Ptr>(this)->perform(&i);
    return i.get_buffer();
  }

  const std::string AST_Node::to_string() const
  {
    return to_string({ NESTED, 5 });
  }

  std::string String_Quoted::inspect() const
  {
    return quote(value_, '*');
  }

  std::string String_Constant::inspect() const
  {
    return quote(value_, '*');
  }

  bool Declaration::is_invisible() const
  {
    if (is_custom_property()) return false;

    return !(value_ && value_->concrete_type() != Expression::NULL_VAL);
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Additional method on Lists to retrieve values directly or from an encompassed Argument.
  //////////////////////////////////////////////////////////////////////////////////////////
  Expression_Obj List::value_at_index(size_t i) {
    Expression_Obj obj = this->at(i);
    if (is_arglist_) {
      if (Argument_Ptr arg = Cast<Argument>(obj)) {
        return arg->value();
      } else {
        return obj;
      }
    } else {
      return obj;
    }
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Convert map to (key, value) list.
  //////////////////////////////////////////////////////////////////////////////////////////
  List_Obj Map::to_list(ParserState& pstate) {
    List_Obj ret = SASS_MEMORY_NEW(List, pstate, length(), SASS_COMMA);

    for (auto key : keys()) {
      List_Obj l = SASS_MEMORY_NEW(List, pstate, 2);
      l->append(key);
      l->append(at(key));
      ret->append(l);
    }

    return ret;
  }

  //////////////////////////////////////////////////////////////////////////////////////////
  // Copy implementations
  //////////////////////////////////////////////////////////////////////////////////////////

  #ifdef DEBUG_SHARED_PTR

  #define IMPLEMENT_AST_OPERATORS(klass) \
    klass##_Ptr klass::copy(std::string file, size_t line) const { \
      klass##_Ptr cpy = new klass(this); \
      cpy->trace(file, line); \
      return cpy; \
    } \
    klass##_Ptr klass::clone(std::string file, size_t line) const { \
      klass##_Ptr cpy = copy(file, line); \
      cpy->cloneChildren(); \
      return cpy; \
    } \

  #else

  #define IMPLEMENT_AST_OPERATORS(klass) \
    klass##_Ptr klass::copy() const { \
      return new klass(this); \
    } \
    klass##_Ptr klass::clone() const { \
      klass##_Ptr cpy = copy(); \
      cpy->cloneChildren(); \
      return cpy; \
    } \

  #endif

  IMPLEMENT_AST_OPERATORS(Supports_Operator);
  IMPLEMENT_AST_OPERATORS(Supports_Negation);
  IMPLEMENT_AST_OPERATORS(Compound_Selector);
  IMPLEMENT_AST_OPERATORS(Complex_Selector);
  IMPLEMENT_AST_OPERATORS(Element_Selector);
  IMPLEMENT_AST_OPERATORS(Class_Selector);
  IMPLEMENT_AST_OPERATORS(Id_Selector);
  IMPLEMENT_AST_OPERATORS(Pseudo_Selector);
  IMPLEMENT_AST_OPERATORS(Wrapped_Selector);
  IMPLEMENT_AST_OPERATORS(Selector_List);
  IMPLEMENT_AST_OPERATORS(Ruleset);
  IMPLEMENT_AST_OPERATORS(Media_Block);
  IMPLEMENT_AST_OPERATORS(Custom_Warning);
  IMPLEMENT_AST_OPERATORS(Custom_Error);
  IMPLEMENT_AST_OPERATORS(List);
  IMPLEMENT_AST_OPERATORS(Map);
  IMPLEMENT_AST_OPERATORS(Function);
  IMPLEMENT_AST_OPERATORS(Number);
  IMPLEMENT_AST_OPERATORS(Binary_Expression);
  IMPLEMENT_AST_OPERATORS(String_Schema);
  IMPLEMENT_AST_OPERATORS(String_Constant);
  IMPLEMENT_AST_OPERATORS(String_Quoted);
  IMPLEMENT_AST_OPERATORS(Boolean);
  IMPLEMENT_AST_OPERATORS(Color);
  IMPLEMENT_AST_OPERATORS(Null);
  IMPLEMENT_AST_OPERATORS(Parent_Selector);
  IMPLEMENT_AST_OPERATORS(Import);
  IMPLEMENT_AST_OPERATORS(Import_Stub);
  IMPLEMENT_AST_OPERATORS(Function_Call);
  IMPLEMENT_AST_OPERATORS(Directive);
  IMPLEMENT_AST_OPERATORS(At_Root_Block);
  IMPLEMENT_AST_OPERATORS(Supports_Block);
  IMPLEMENT_AST_OPERATORS(While);
  IMPLEMENT_AST_OPERATORS(Each);
  IMPLEMENT_AST_OPERATORS(For);
  IMPLEMENT_AST_OPERATORS(If);
  IMPLEMENT_AST_OPERATORS(Mixin_Call);
  IMPLEMENT_AST_OPERATORS(Extension);
  IMPLEMENT_AST_OPERATORS(Media_Query);
  IMPLEMENT_AST_OPERATORS(Media_Query_Expression);
  IMPLEMENT_AST_OPERATORS(Debug);
  IMPLEMENT_AST_OPERATORS(Error);
  IMPLEMENT_AST_OPERATORS(Warning);
  IMPLEMENT_AST_OPERATORS(Assignment);
  IMPLEMENT_AST_OPERATORS(Return);
  IMPLEMENT_AST_OPERATORS(At_Root_Query);
  IMPLEMENT_AST_OPERATORS(Variable);
  IMPLEMENT_AST_OPERATORS(Comment);
  IMPLEMENT_AST_OPERATORS(Attribute_Selector);
  IMPLEMENT_AST_OPERATORS(Supports_Interpolation);
  IMPLEMENT_AST_OPERATORS(Supports_Declaration);
  IMPLEMENT_AST_OPERATORS(Supports_Condition);
  IMPLEMENT_AST_OPERATORS(Parameters);
  IMPLEMENT_AST_OPERATORS(Parameter);
  IMPLEMENT_AST_OPERATORS(Arguments);
  IMPLEMENT_AST_OPERATORS(Argument);
  IMPLEMENT_AST_OPERATORS(Unary_Expression);
  IMPLEMENT_AST_OPERATORS(Function_Call_Schema);
  IMPLEMENT_AST_OPERATORS(Block);
  IMPLEMENT_AST_OPERATORS(Content);
  IMPLEMENT_AST_OPERATORS(Trace);
  IMPLEMENT_AST_OPERATORS(Keyframe_Rule);
  IMPLEMENT_AST_OPERATORS(Bubble);
  IMPLEMENT_AST_OPERATORS(Selector_Schema);
  IMPLEMENT_AST_OPERATORS(Placeholder_Selector);
  IMPLEMENT_AST_OPERATORS(Definition);
  IMPLEMENT_AST_OPERATORS(Declaration);
}
