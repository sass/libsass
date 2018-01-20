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

  /*#########################################################################*/
  /*#########################################################################*/

  // it's a superselector if every selector of the right side
  // list is a superselector of the given left side selector
  bool Selector_List::is_superselector_of(Selector_List_Obj rhs, std::string wrapping)
  {
    // Check every rhs selector against left hand list
    for(size_t i = 0, L = rhs->length(); i < L; ++i) {
      if (!is_superselector_of(rhs->get(i), wrapping)) return false;
    }
    return true;
  }

  // it's a superselector if every selector on the right side
  // is a superselector of any one of the left side selectors
  bool Selector_List::is_superselector_of(Complex_Selector_Obj rhs, std::string wrapping)
  {
    // Check every lhs selector against right hand
    for(size_t i = 0, L = length(); i < L; ++i) {
      if (get(i)->is_superselector_of(rhs)) return true;
    }
    return false;
  }

  // it's a superselector if every selector on the right side
  // is a superselector of any one of the left side selectors
  bool Selector_List::is_superselector_of(Compound_Selector_Obj rhs, std::string wrapping)
  {
    // Check every lhs selector against right hand
    for(size_t i = 0, L = length(); i < L; ++i) {
      if (get(i)->is_superselector_of(rhs, wrapping)) return true;
    }
    return false;
  }

  /*#########################################################################*/
  /*#########################################################################*/

  // it's a superselector if every selector of the right side
  // list is a superselector of the given left side selector
  bool Complex_Selector::is_superselector_of(Selector_List_Obj rhs, std::string wrapping)
  {
    // Check every rhs selector against left hand list
    for(size_t i = 0, L = rhs->length(); i < L; ++i) {
      if (!is_superselector_of(rhs->get(i), wrapping)) return false;
    }
    return true;
  }

  bool Complex_Selector::is_superselector_of(Compound_Selector_Obj rhs, std::string wrapping)
  {
    return last()->head() && last()->head()->is_superselector_of(rhs, wrapping);
  }

  bool Compound_Selector::is_superselector_of(Selector_List_Obj rhs, std::string wrapped)
  {
    for(size_t i = 0, L = rhs->length(); i < L; ++i) {
      if (is_superselector_of(rhs->get(i), wrapped)) return true;
    }
    return false;
  }

  bool Compound_Selector::is_superselector_of(Complex_Selector_Obj rhs, std::string wrapped)
  {
    if (!rhs->head()) return false;
    return is_superselector_of(rhs->head(), wrapped);
  }

  /*#########################################################################*/
  /*#########################################################################*/

  bool Complex_Selector::is_superselector_of(Complex_Selector_Obj rhs, std::string wrapping)
  {
    Complex_Selector_Ptr lhs = this;
    // check for selectors with leading or trailing combinators
    if (!lhs->head() || !rhs->head())
    { return false; }
    Complex_Selector_Obj l_innermost = lhs->innermost();
    if (l_innermost->combinator() != Complex_Selector::ANCESTOR_OF)
    { return false; }
    Complex_Selector_Obj r_innermost = rhs->innermost();
    if (r_innermost->combinator() != Complex_Selector::ANCESTOR_OF)
    { return false; }
    // more complex (i.e., longer) selectors are always more specific
    size_t l_len = lhs->length(), r_len = rhs->length();
    if (l_len > r_len)
    { return false; }

    if (l_len == 1)
    { return lhs->head()->is_superselector_of(rhs->last()->head(), wrapping); }

    // we have to look one tail deeper, since we cary the
    // combinator around for it (which is important here)
    if (rhs->tail() && lhs->tail() && combinator() != Complex_Selector::ANCESTOR_OF) {
      Complex_Selector_Obj lhs_tail = lhs->tail();
      Complex_Selector_Obj rhs_tail = rhs->tail();
      if (lhs_tail->combinator() != rhs_tail->combinator()) return false;
      if (lhs_tail->head() && !rhs_tail->head()) return false;
      if (!lhs_tail->head() && rhs_tail->head()) return false;
      if (lhs_tail->head() && rhs_tail->head()) {
        if (!lhs_tail->head()->is_superselector_of(rhs_tail->head())) return false;
      }
    }

    bool found = false;
    Complex_Selector_Obj marker = rhs;
    for (size_t i = 0, L = rhs->length(); i < L; ++i) {
      if (i == L-1)
      { return false; }
      if (lhs->head() && marker->head() && lhs->head()->is_superselector_of(marker->head(), wrapping))
      { found = true; break; }
      marker = marker->tail();
    }
    if (!found)
    { return false; }

    /*
      Hmm, I hope I have the logic right:

      if lhs has a combinator:
        if !(marker has a combinator) return false
        if !(lhs.combinator == '~' ? marker.combinator != '>' : lhs.combinator == marker.combinator) return false
        return lhs.tail-without-innermost.is_superselector_of(marker.tail-without-innermost)
      else if marker has a combinator:
        if !(marker.combinator == ">") return false
        return lhs.tail.is_superselector_of(marker.tail)
      else
        return lhs.tail.is_superselector_of(marker.tail)
    */
    if (lhs->combinator() != Complex_Selector::ANCESTOR_OF)
    {
      if (marker->combinator() == Complex_Selector::ANCESTOR_OF)
      { return false; }
      if (!(lhs->combinator() == Complex_Selector::PRECEDES ? marker->combinator() != Complex_Selector::PARENT_OF : lhs->combinator() == marker->combinator()))
      { return false; }
      return lhs->tail()->is_superselector_of(marker->tail());
    }
    else if (marker->combinator() != Complex_Selector::ANCESTOR_OF)
    {
      if (marker->combinator() != Complex_Selector::PARENT_OF)
      { return false; }
      return lhs->tail()->is_superselector_of(marker->tail());
    }
    return lhs->tail()->is_superselector_of(marker->tail());
  }

  bool Compound_Selector::is_superselector_of(Compound_Selector_Obj rhs, std::string wrapping)
  {
    Compound_Selector_Ptr lhs = this;
    Simple_Selector_Ptr lbase = lhs->base();
    Simple_Selector_Ptr rbase = rhs->base();

    // Check if pseudo-elements are the same between the selectors

    std::set<Simple_Selector*> lpsuedoset, rpsuedoset;
    for (size_t i = 0, L = length(); i < L; ++i)
    {
      if (get(i)->is_pseudo_element()) {
        lpsuedoset.insert(get(i));
      }
    }
    for (size_t i = 0, L = rhs->length(); i < L; ++i)
    {
      if (rhs->get(i)->is_pseudo_element()) {
        rpsuedoset.insert(rhs->get(i));
      }
    }
    if (lpsuedoset != rpsuedoset) {
      return false;
    }

    // replaced compare without stringification
    // https://github.com/sass/sass/issues/2229
    SelectorSet lset, rset;

    if (lbase && rbase)
    {
      if (*lbase == *rbase) {
        // create ordered sets for includes query
        lset.insert(this->begin(), this->end());
        rset.insert(rhs->begin(), rhs->end());
        return includes(rset.begin(), rset.end(), lset.begin(), lset.end());
      }
      return false;
    }

    for (size_t i = 0, iL = length(); i < iL; ++i)
    {
      Selector_Ptr wlhs = (*this)[i];
      // very special case for wrapped matches selector
      if (Wrapped_Selector_Ptr wrapped = Cast<Wrapped_Selector>(wlhs)) {
        if (wrapped->name() == ":not") {
          if (Selector_List_Ptr not_list = Cast<Selector_List>(wrapped->selector())) {
            if (not_list->is_superselector_of(rhs, wrapped->name())) return false;
          } else {
            throw std::runtime_error("wrapped not selector is not a list");
          }
        }
        if (wrapped->name() == ":matches" || (wrapped->name()[0] == ':' && ends_with(wrapped->name(), "-any"))) {
          wlhs = wrapped->selector();
          if (Selector_List_Ptr list = Cast<Selector_List>(wrapped->selector())) {
            if (Compound_Selector_Ptr comp = Cast<Compound_Selector>(rhs)) {
              if (!wrapping.empty() && wrapping != wrapped->name()) return false;
              if (wrapping.empty() || wrapping != wrapped->name()) {;
                if (list->is_superselector_of(comp, wrapped->name())) return true;
              }
            }
          }
        }
        Simple_Selector_Ptr rhs_sel = NULL;
        if (rhs->elements().size() > i) rhs_sel = (*rhs)[i];
        if (Wrapped_Selector_Ptr wrapped_r = Cast<Wrapped_Selector>(rhs_sel)) {
          if (wrapped->name() == wrapped_r->name()) {
          if (wrapped->is_superselector_of(wrapped_r)) {
             continue;
          }}
        }
      }
      // match from here on as strings
      lset.insert(wlhs);
    }

    for (size_t n = 0, nL = rhs->length(); n < nL; ++n)
    {
      Selector_Ptr r = (*rhs)[n];
      if (Wrapped_Selector_Ptr wrapped = Cast<Wrapped_Selector>(r)) {
        if (wrapped->name() == ":not") {
          if (Selector_List_Ptr ls = Cast<Selector_List>(wrapped->selector())) {
            ls->remove_parent_selectors();
            if (is_superselector_of(ls, wrapped->name())) return false;
          }
        }
        if (wrapped->name() == ":matches" || (wrapped->name()[0] == ':' && ends_with(wrapped->name(), "-any"))) {
          if (!wrapping.empty()) {
            if (wrapping != wrapped->name()) return false;
          }
          if (Selector_List_Ptr ls = Cast<Selector_List>(wrapped->selector())) {
            ls->remove_parent_selectors();
            return (is_superselector_of(ls, wrapped->name()));
          }
        }
      }
      rset.insert(r);
    }

    //for (auto l : lset) { cerr << "l: " << l << endl; }
    //for (auto r : rset) { cerr << "r: " << r << endl; }

    if (lset.empty()) return true;
    // return true if rset contains all the elements of lset
    return includes(rset.begin(), rset.end(), lset.begin(), lset.end());

  }

  bool Wrapped_Selector::is_superselector_of(Wrapped_Selector_Obj rhs)
  {
    if (this->name() == ":current") return false;
    if (this->name() != rhs->name()) return false;
    return selector()->is_superselector_of(rhs->selector());
  }


  /*#########################################################################*/
  /*#########################################################################*/

  Selector_List_Ptr Complex_Selector::unify_with(Complex_Selector_Ptr other)
  {

    // get last tails (on the right side)
    Complex_Selector_Obj l_last = this->last();
    Complex_Selector_Obj r_last = other->last();

    // check valid pointers (assertion)
    SASS_ASSERT(l_last, "lhs is null");
    SASS_ASSERT(r_last, "rhs is null");

    // Not sure about this check, but closest way I could check
    // was to see if this is a ruby 'SimpleSequence' equivalent.
    // It seems to do the job correctly as some specs react to this
    if (l_last->combinator() != Combinator::ANCESTOR_OF) return 0;
    if (r_last->combinator() != Combinator::ANCESTOR_OF ) return 0;

    // get the headers for the last tails
    Compound_Selector_Obj l_last_head = l_last->head();
    Compound_Selector_Obj r_last_head = r_last->head();

    // check valid head pointers (assertion)
    SASS_ASSERT(l_last_head, "lhs head is null");
    SASS_ASSERT(r_last_head, "rhs head is null");

    // get the unification of the last compound selectors
    Compound_Selector_Obj unified = r_last_head->unify_with(l_last_head);

    // abort if we could not unify heads
    if (unified == 0) return 0;

    // check for universal (star: `*`) selector
    bool is_universal = l_last_head->is_universal() ||
                        r_last_head->is_universal();

    if (is_universal)
    {
      // move the head
      l_last->head(0);
      r_last->head(unified);
    }

    // create nodes from both selectors
    Node lhsNode = complexSelectorToNode(this);
    Node rhsNode = complexSelectorToNode(other);

    // overwrite universal base
    if (!is_universal)
    {
      // create some temporaries to convert to node
      Complex_Selector_Obj fake = unified->to_complex();
      Node unified_node = complexSelectorToNode(fake);
      // add to permutate the list?
      rhsNode.plus(unified_node);
    }

    // do some magic we inherit from node and extend
    Node node = subweave(lhsNode, rhsNode);
    Selector_List_Obj result = SASS_MEMORY_NEW(Selector_List, pstate());
    NodeDequePtr col = node.collection(); // move from collection to list
    for (NodeDeque::iterator it = col->begin(), end = col->end(); it != end; it++)
    { result->append(nodeToComplexSelector(Node::naiveTrim(*it))); }

    // only return if list has some entries
    return result->length() ? result.detach() : 0;

  }

  Selector_List_Ptr Selector_List::unify_with(Selector_List_Ptr rhs) {
    std::vector<Complex_Selector_Obj> unified_complex_selectors;
    // Unify all of children with RHS's children, storing the results in `unified_complex_selectors`
    for (size_t lhs_i = 0, lhs_L = length(); lhs_i < lhs_L; ++lhs_i) {
      Complex_Selector_Obj seq1 = (*this)[lhs_i];
      for(size_t rhs_i = 0, rhs_L = rhs->length(); rhs_i < rhs_L; ++rhs_i) {
        Complex_Selector_Ptr seq2 = rhs->at(rhs_i);

        Selector_List_Obj result = seq1->unify_with(seq2);
        if( result ) {
          for(size_t i = 0, L = result->length(); i < L; ++i) {
            unified_complex_selectors.push_back( (*result)[i] );
          }
        }
      }
    }

    // Creates the final Selector_List by combining all the complex selectors
    Selector_List_Ptr final_result = SASS_MEMORY_NEW(Selector_List, pstate());
    for (auto itr = unified_complex_selectors.begin(); itr != unified_complex_selectors.end(); ++itr) {
      final_result->append(*itr);
    }
    return final_result;
  }

  Compound_Selector_Ptr Compound_Selector::unify_with(Compound_Selector_Ptr rhs)
  {
    if (empty()) return rhs;
    Compound_Selector_Obj unified = SASS_MEMORY_COPY(rhs);
    for (size_t i = 0, L = length(); i < L; ++i)
    {
      if (unified.isNull()) break;
      unified = at(i)->unify_with(unified);
    }
    return unified.detach();
  }




  Compound_Selector_Ptr Simple_Selector::unify_with(Compound_Selector_Ptr rhs)
  {
    for (size_t i = 0, L = rhs->length(); i < L; ++i)
    { if (*this == *rhs->at(i)) return rhs; }

    // check for pseudo elements because they are always last
    size_t i, L;
    bool found = false;
    if (typeid(*this) == typeid(Pseudo_Selector) || typeid(*this) == typeid(Wrapped_Selector) || typeid(*this) == typeid(Attribute_Selector))
    {
      for (i = 0, L = rhs->length(); i < L; ++i)
      {
        if ((Cast<Pseudo_Selector>((*rhs)[i]) || Cast<Wrapped_Selector>((*rhs)[i]) || Cast<Attribute_Selector>((*rhs)[i])) && (*rhs)[L-1]->is_pseudo_element())
        { found = true; break; }
      }
    }
    else
    {
      for (i = 0, L = rhs->length(); i < L; ++i)
      {
        if (Cast<Pseudo_Selector>((*rhs)[i]) || Cast<Wrapped_Selector>((*rhs)[i]) || Cast<Attribute_Selector>((*rhs)[i]))
        { found = true; break; }
      }
    }
    if (!found)
    {
      rhs->append(this);
    } else {
      rhs->elements().insert(rhs->elements().begin() + i, this);
    }
    return rhs;
  }

  Simple_Selector_Ptr Element_Selector::unify_with(Simple_Selector_Ptr rhs)
  {
    // check if ns can be extended
    // true for no ns or universal
    if (has_universal_ns())
    {
      // but dont extend with universal
      // true for valid ns and universal
      if (!rhs->is_universal_ns())
      {
        // overwrite the name if star is given as name
        if (this->name() == "*") { this->name(rhs->name()); }
        // now overwrite the namespace name and flag
        this->ns(rhs->ns()); this->has_ns(rhs->has_ns());
        // return copy
        return this;
      }
    }
    // namespace may changed, check the name now
    // overwrite star (but not with another star)
    if (name() == "*" && rhs->name() != "*")
    {
      // simply set the new name
      this->name(rhs->name());
      // return copy
      return this;
    }
    // return original
    return this;
  }

  Compound_Selector_Ptr Element_Selector::unify_with(Compound_Selector_Ptr rhs)
  {
    // TODO: handle namespaces

    // if the rhs is empty, just return a copy of this
    if (rhs->length() == 0) {
      rhs->append(this);
      return rhs;
    }

    Simple_Selector_Ptr rhs_0 = rhs->at(0);
    // otherwise, this is a tag name
    if (name() == "*")
    {
      if (typeid(*rhs_0) == typeid(Element_Selector))
      {
        // if rhs is universal, just return this tagname + rhs's qualifiers
        Element_Selector_Ptr ts = Cast<Element_Selector>(rhs_0);
        rhs->at(0) = this->unify_with(ts);
        return rhs;
      }
      else if (Cast<Class_Selector>(rhs_0) || Cast<Id_Selector>(rhs_0)) {
        // qualifier is `.class`, so we can prefix with `ns|*.class`
        if (has_ns() && !rhs_0->has_ns()) {
          if (ns() != "*") rhs->elements().insert(rhs->begin(), this);
        }
        return rhs;
      }


      return rhs;
    }

    if (typeid(*rhs_0) == typeid(Element_Selector))
    {
      // if rhs is universal, just return this tagname + rhs's qualifiers
      if (rhs_0->name() != "*" && rhs_0->ns() != "*" && rhs_0->name() != name()) return 0;
      // otherwise create new compound and unify first simple selector
      rhs->at(0) = this->unify_with(rhs_0);
      return rhs;

    }
    // else it's a tag name and a bunch of qualifiers -- just append them
    if (name() != "*") rhs->elements().insert(rhs->begin(), this);
    return rhs;
  }

  Compound_Selector_Ptr Class_Selector::unify_with(Compound_Selector_Ptr rhs)
  {
    rhs->has_line_break(has_line_break());
    return Simple_Selector::unify_with(rhs);
  }

  Compound_Selector_Ptr Id_Selector::unify_with(Compound_Selector_Ptr rhs)
  {
    for (size_t i = 0, L = rhs->length(); i < L; ++i)
    {
      if (Id_Selector_Ptr sel = Cast<Id_Selector>(rhs->at(i))) {
        if (sel->name() != name()) return 0;
      }
    }
    rhs->has_line_break(has_line_break());
    return Simple_Selector::unify_with(rhs);
  }

  Compound_Selector_Ptr Pseudo_Selector::unify_with(Compound_Selector_Ptr rhs)
  {
    if (is_pseudo_element())
    {
      for (size_t i = 0, L = rhs->length(); i < L; ++i)
      {
        if (Pseudo_Selector_Ptr sel = Cast<Pseudo_Selector>(rhs->at(i))) {
          if (sel->is_pseudo_element() && sel->name() != name()) return 0;
        }
      }
    }
    return Simple_Selector::unify_with(rhs);
  }

  /*#########################################################################*/
  /*#########################################################################*/

}