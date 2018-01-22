#include "sass.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "node.hpp"
#include "eval.hpp"
#include "debugger.hpp"
#include "extend.hpp"
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
    // replaced compare without stringification
    // https://github.com/sass/sass/issues/2229
    SimpleSelectorSet lpset, rpset;

    // Check if pseudo-elements are the same
    // otherwise this is not a superselector
    for (size_t i = 0, L = length(); i < L; ++i)
    {
      if (get(i)->is_pseudo_element()) {
        lpset.insert(get(i));
      }
    }
    for (size_t i = 0, L = rhs->length(); i < L; ++i)
    {
      if (rhs->get(i)->is_pseudo_element()) {
        rpset.insert(rhs->get(i));
      }
    }
    if (!SetsAreEqual(rpset, lpset)) return false;

    // replaced compare without stringification
    // https://github.com/sass/sass/issues/2229
    SimpleSelectorSet lset, rset;

    Simple_Selector_Ptr lbase = base();
    Simple_Selector_Ptr rbase = rhs->base();

    if (lbase && rbase)
    {
      if (*lbase == *rbase) {
        // create ordered sets for contains query
        lset.insert(this->begin(), this->end());
        rset.insert(rhs->begin(), rhs->end());
        return SetContains(rset, lset);
      }
      return false;
    }

    for (size_t i = 0, iL = length(); i < iL; ++i)
    {
      Simple_Selector_Ptr wlhs = (*this)[i];
      // very special case for wrapped matches selector
      if (Wrapped_Selector_Ptr wrapped = Cast<Wrapped_Selector>(wlhs)) {
        if (wrapped->name() == ":not") {
          if (wrapped->selector()->is_superselector_of(rhs, wrapped->name())) return false;
        }
        if (wrapped->name() == ":matches" || (wrapped->name()[0] == ':' && ends_with(wrapped->name(), "-any"))) {
          // wlhs = wrapped->selector();
          if (Selector_List_Ptr list = wrapped->selector()) {
            if (!wrapping.empty() && wrapping != wrapped->name()) return false;
            if (wrapping.empty() || wrapping != wrapped->name()) {;
              if (list->is_superselector_of(rhs, wrapped->name())) return true;
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
      Simple_Selector_Ptr r = (*rhs)[n];
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

    if (lset.empty()) return true;
    if (rset.empty()) return false;
    // return true if rset contains all the elements of lset
    return SetContains(rset, lset);

  }

  bool Wrapped_Selector::is_superselector_of(Wrapped_Selector_Obj rhs)
  {
    if (this->name() == ":current") return false;
    if (this->name() != rhs->name()) return false;
    return selector()->is_superselector_of(rhs->selector());
  }


  /*#########################################################################*/
  /*#########################################################################*/

  Selector_List_Ptr Complex_Selector::unify_with(Complex_Selector_Ptr rhs)
  {


    // get last tails (on the right side)
    Complex_Selector_Obj l_last = this->last();
    Complex_Selector_Obj r_last = rhs->last();

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
    Node rhsNode = complexSelectorToNode(rhs);

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

  Selector_List_Ptr Selector_List::unify_with(Selector_List_Ptr rhs)
  {

    std::vector<Complex_Selector_Obj> unified_complex_selectors;

    // Unify all of children with RHS's children, storing the results in `unified_complex_selectors`
    for (size_t lhs_i = 0, lhs_L = length(); lhs_i < lhs_L; ++lhs_i) {
      Complex_Selector_Obj seq1 = this->get(lhs_i);
      for(size_t rhs_i = 0, rhs_L = rhs->length(); rhs_i < rhs_L; ++rhs_i) {
        Complex_Selector_Ptr seq2 = rhs->get(rhs_i);
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
        // is_pseudo_element check is needed to preserve the correct order!??
        if ((Cast<Pseudo_Selector>((*rhs)[i])) && (*rhs)[L-1]->is_pseudo_element())
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
    return Simple_Selector::unify_with(rhs);
  }

  /*#########################################################################*/
  /*#########################################################################*/

  Selector_List_Ptr Compound_Selector::weaver(Selector_List_Ptr rhs)
  {
    throw std::runtime_error("not yet implemented");
  }

  // https://www.geeksforgeeks.org/longest-common-subsequence/
  // https://www.geeksforgeeks.org/printing-longest-common-subsequence/
  std::vector<Selector_Group_Obj> lcs(Selector_Groups& X, Selector_Groups& Y)
  {

    size_t m = X.length() - 1;
    size_t n = Y.length() - 1;

    #define L(i,j) l[ (i) * m + j ]
    size_t* l = new size_t[(m+1)*(n+1)];

    /* Following steps build L[m+1][n+1] in bottom up fashion. Note
      that L[i][j] contains length of LCS of X[0..i-1] and Y[0..j-1] */
    for (size_t i = 0; i <= m; i++)
    {
      for (size_t j = 0; j <= n; j++)
      {
        if (i == 0 || j == 0)
          L(i, j) = 0;
        else if (*X[i-1] == *Y[j-1])
          L(i, j) = L(i-1, j-1) + 1;
        else
          L(i, j) = std::max(L(i-1, j), L(i, j-1));
      }
    }

    // Following code is used to print LCS
    size_t index = L(m, n);

    // Create an array to store the lcs groups
    std::vector<Selector_Group_Obj> lcs(index);

    // Start from the right-most-bottom-most corner and
    // one by one store objects in lcs[]
    size_t i = m, j = n;
    while (i > 0 && j > 0)
    {

      // If current objects in X[] and Y are same, then
      // current object is part of LCS
      if (*X[i-1] == *Y[j-1])
      {
          lcs[index-1] = X[i-1]; // Put current object in result
          i--; j--; index--;     // reduce values of i, j and index
      }

      // If not same, then find the larger of two and
      // go in the direction of larger value
      else if (L(i-1, j) > L(i, j-1))
          i--;
      else
          j--;
    }

    delete[] l;
    return lcs;

  }

  // make static obj to always hold on to the pointer
  Complex_Selector_Obj base = (SASS_MEMORY_NEW(Placeholder_Selector,
    ParserState("[TMP]"), "<temp>"))->toComplexSelector();

  bool parent_superselector(Selector_Group_Obj lhs, Selector_Group_Obj rhs)
  {
    lhs = lhs->copy(); lhs->append(base);
    rhs = rhs->copy(); rhs->append(base);
    Complex_Selector_Obj lhcs = lhs->toComplexSelector();
    Complex_Selector_Obj rhcs = rhs->toComplexSelector();
    return lhcs->is_superselector_of(rhcs);
  }

  /*
  def chunks(seq1, seq2)
    chunk1 = []
    chunk1 << seq1.shift until yield seq1
    chunk2 = []
    chunk2 << seq2.shift until yield seq2
    return [] if chunk1.empty? && chunk2.empty?
    return [chunk2] if chunk1.empty?
    return [chunk1] if chunk2.empty?
    [chunk1 + chunk2, chunk2 + chunk1]
  end
  */

  Selector_Groups_Obj chunks(Selector_Groups_Ptr seq1, Selector_Groups_Ptr seq2, std::vector<Selector_Group_Obj> lcs)
  {
    auto chunks = SASS_MEMORY_NEW(Selector_Groups, ParserState("[TMP]"));
    auto chunks1 = SASS_MEMORY_NEW(Selector_Group, ParserState("[TMP]"));
    auto chunks2 = SASS_MEMORY_NEW(Selector_Group, ParserState("[TMP]"));

    while (!seq1->empty()) {
      Selector_Group_Obj s1 = seq1->first();
      if (parent_superselector(s1, lcs.front())) break;
      seq1->erase(seq1->begin());
      chunks1->concat(s1);
    }
    while (!seq2->empty()) {
      Selector_Group_Obj s2 = seq2->first();
      if (parent_superselector(s2, lcs.front())) break;
      seq2->erase(seq2->begin());
      chunks2->concat(s2);
    }

    if (chunks1->empty() && chunks2->empty()) { return chunks; }
    else if (chunks2->empty() && !chunks1->empty()) { chunks->append(chunks1); }
    else if (chunks1->empty() && !chunks2->empty()) { chunks->append(chunks2); }
    else if (!chunks1->empty() && !chunks2->empty()) {
      auto lhs = chunks1->copy();
      auto rhs = chunks2->copy();
      lhs->concat(chunks2);
      rhs->concat(chunks1);
      chunks->append(lhs);
      chunks->append(rhs);
    }

    return chunks;
  }

  /*
  def group_selectors(seq)
    newseq = []
    tail = seq.dup
    until tail.empty?
      head = []
      begin
        head << tail.shift
      end while !tail.empty? && head.last.is_a?(String) || tail.first.is_a?(String)
      newseq << head
    end
    newseq
  end
  */

  Selector_Groups_Obj group_selectors(Complex_Selector_Ptr s)
  {
    Complex_Selector_Ptr current = s->first();
    auto groups = SASS_MEMORY_NEW(Selector_Groups, s->pstate());
    if (current->empty()) return groups;
    auto sg = SASS_MEMORY_NEW(Selector_Group, s->pstate());
    while (current) {
      sg->append(current);
      if (current->is_ancestor()) {
        groups->append(sg);
        sg = SASS_MEMORY_NEW(Selector_Group, s->pstate());
      }
      current = current->tail();
    }
    groups->append(sg);
    return groups;
  }

  /*
  def paths(arrs)
    arrs.inject([[]]) do |paths, arr|
      arr.map {|e| paths.map {|path| path + [e]}}.flatten(1)
    end
  end
  */

  Selector_Groups_Ptr paths(const std::vector<Selector_Groups_Obj>& arrs)
  {

    ParserState pstate("[NA]");
    Selector_Groups_Obj paths =
      SASS_MEMORY_NEW(Selector_Groups, pstate);

    // declare loop variables
    size_t parts = arrs.size();
    size_t* idx = new size_t[parts]();
    size_t* lens = new size_t[parts]();

    // loop iteration variables
    size_t perms = 1, perm = 0;

    // prepare some loop variables
    for (size_t i = 0; i < parts; i++) {
      // store length for easy access
      lens[i] = arrs[i]->length();
      // calculate permutations
      perms *= lens[i];
    }

    // create all permutations
    while (perm ++ < perms) {

      // create current permutation
      Selector_Group_Obj path =
        SASS_MEMORY_NEW(Selector_Group, pstate);
      for (size_t i = 0; i < parts; i++) {
        path->concat(arrs[i]->get(idx[i]));
      }
      paths->append(path);

      // increment for all permutations
      for (size_t i = 0; i < parts; i ++) {
        if (idx[i] + 1 == lens[i]) {
          idx[i] = 0;
        } else {
          idx[i] ++;
          break;
        }
      }

    }

    delete[] idx;
    delete[] lens;
    return paths.detach();

  }

  /*
  def merge_initial_ops(seq1, seq2)
    ops1, ops2 = [], []
    ops1 << seq1.shift while seq1.first.is_a?(String)
    ops2 << seq2.shift while seq2.first.is_a?(String)
    STDERR.puts "SEQ #{ops1}"
    newline = false
    newline ||= !!ops1.shift if ops1.first == "\n"
    newline ||= !!ops2.shift if ops2.first == "\n"

    # If neither sequence is a subsequence of the other, they cannot be
    # merged successfully
    lcs = Sass::Util.lcs(ops1, ops2)
    return unless lcs == ops1 || lcs == ops2
    (newline ? ["\n"] : []) + (ops1.size > ops2.size ? ops1 : ops2)
  end
  */

//    Selector_Groups_Obj init = SASS_MEMORY_NEW(Selector_Groups, rh.pstate());

  Selector_Group_Ptr merge_initial_ops(Complex_Selector_Ptr* lhsptr, Complex_Selector_Ptr* rhsptr)
  {

    Complex_Selector_Ptr lhs = *lhsptr, rhs = *rhsptr;

    Selector_Group_Obj ops1 = SASS_MEMORY_NEW(Selector_Group, lhs->pstate());
    Selector_Group_Obj ops2 = SASS_MEMORY_NEW(Selector_Group, rhs->pstate());

    // extract all bare combinators (no heads)
    while (lhs && lhs->is_bare_combinator()) {
      ops1->append(lhs);
      lhs = lhs->tail();
    }
    while (rhs && rhs->is_bare_combinator()) {
      ops2->append(rhs);
      rhs = rhs->tail();
    }

    size_t min = std::min(ops1->length(), ops2->length());
    for (size_t i = 0; i < min; i ++) {
      auto lh = ops1->get(i), rh = ops2->get(i);
      if (lh->combinator() != rh->combinator()) {
        return NULL; // sequences do not match
      }
    }

    *lhsptr = lhs, *rhsptr = rhs;

    // return the bigger sequence
    return ops1->length() > ops2->length()
      ? ops1.detach() : ops2.detach();

  }

  /*
  def subweave(seq1, seq2)
    return [seq2] if seq1.empty?
    return [seq1] if seq2.empty?

    seq1, seq2 = seq1.dup, seq2.dup
    return unless (init = merge_initial_ops(seq1, seq2))
    return unless (fin = merge_final_ops(seq1, seq2))


    # Make sure there's only one root selector in the output.
    root1 = has_root?(seq1.first) && seq1.shift
    root2 = has_root?(seq2.first) && seq2.shift
    if root1 && root2
      return unless (root = root1.unify(root2))
      seq1.unshift root
      seq2.unshift root
    elsif root1
      seq2.unshift root1
    elsif root2
      seq1.unshift root2
    end

    seq1 = group_selectors(seq1)
    seq2 = group_selectors(seq2)
    lcs = Sass::Util.lcs(seq2, seq1) do |s1, s2|
      next s1 if s1 == s2
      next unless s1.first.is_a?(SimpleSequence) && s2.first.is_a?(SimpleSequence)
      next s2 if parent_superselector?(s1, s2)
      next s1 if parent_superselector?(s2, s1)
      next unless must_unify?(s1, s2)
      next unless (unified = Sequence.new(s1).unify(Sequence.new(s2)))
      unified.members.first.members if unified.members.length == 1
    end

    diff = [[init]]

    until lcs.empty?
      diff << chunks(seq1, seq2) {|s| parent_superselector?(s.first, lcs.first)} << [lcs.shift]
      seq1.shift
      seq2.shift
    end
    diff << chunks(seq1, seq2) {|s| s.empty?}
    diff += fin.map {|sel| sel.is_a?(Array) ? sel : [sel]}
    diff.reject! {|c| c.empty?}
    # diff = diff.map {|sel| sel.map {|sel| sel.is_a?(Array) ? sel.flatten : sel} }
    Sass::Util.paths(diff).map {|p| p.flatten}.reject {|p| path_has_two_subjects?(p)}
  end
  */

  Selector_List_Ptr Complex_Selector::subweaver(Complex_Selector_Ptr rhs)
  {

    if (empty()) { return rhs->toSelectorList(); }
    if (rhs->empty()) { return toSelectorList(); }

    // first returns an object
    // need to keep it therefore
    // ToDO: make it more elegant
    Complex_Selector_Obj olhs;
    Complex_Selector_Obj orhs;
    Complex_Selector_Ptr lhs;
    olhs = this->first();
    orhs = rhs->first();
    lhs = olhs, rhs = orhs;

    auto init = merge_initial_ops(&lhs, &rhs);
    if (init == NULL) return NULL;

    // this one will be tricky, as we probably
    // need to create copies to cut them off
    // auto fin = merge_final_ops(lh, rh);
    // if (fin == NULL) return NULL;

    // return unless (init = merge_initial_ops(seq1, seq2))
    // return unless (fin = merge_final_ops(seq1, seq2))

    // root1 = has_root?(seq1.first) && seq1.shift
    // root2 = has_root?(seq2.first) && seq2.shift
    // ...........................................

    auto seq1 = group_selectors(lhs);
    auto seq2 = group_selectors(rhs);

    // so far only equality test is done!
    // ruby sass has additional checks!
    auto LCS = lcs(*seq1, *seq2);

    std::vector<Selector_Groups_Obj> diff;

    if (!init->empty()) diff.push_back(init->toSelectorGroups());

    while (!LCS.empty()) {
      auto chks = chunks(seq1, seq2, LCS);
      if (!chks->empty()) diff.push_back(chks);
      auto lcsfirst = LCS.front();
      auto lcg = SASS_MEMORY_NEW(Selector_Groups, /* rh. */pstate());
      lcg->append(lcsfirst);
      diff.push_back(lcg);
      seq2->erase(seq2->begin());
      seq1->erase(seq1->begin());
      LCS.erase(LCS.begin());
    }

    Selector_Groups_Obj path = paths(diff);
    return path->toSelectorList();

  }

}