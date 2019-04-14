#include "sass.hpp"
#include "ast.hpp"
#include "context.hpp"
#include "node.hpp"
#include "eval.hpp"
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

#include "ast_selectors.hpp"

// #define DEBUG_UNIFY

namespace Sass {

  Compound_Selector* Compound_Selector::unify_with(Compound_Selector* rhs)
  {
    #ifdef DEBUG_UNIFY
    const std::string debug_call = "unify(Compound[" + this->to_string() + "], Compound[" + rhs->to_string() + "])";
    std::cerr << debug_call << std::endl;
    #endif

    if (empty()) return rhs;
    Compound_Selector_Obj unified = SASS_MEMORY_COPY(rhs);
    for (const Simple_Selector_Obj& sel : elements()) {
      unified = sel->unify_with(unified);
      if (unified.isNull()) break;
    }

    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " = Compound[" << unified->to_string() << "]" << std::endl;
    #endif
    return unified.detach();
  }

  Compound_Selector* Simple_Selector::unify_with(Compound_Selector* rhs)
  {
    #ifdef DEBUG_UNIFY
    const std::string debug_call = "unify(Simple[" + this->to_string() + "], Compound[" + rhs->to_string() + "])";
    std::cerr << debug_call << std::endl;
    #endif

    if (rhs->length() == 1) {
      if (rhs->at(0)->is_universal()) {
        Compound_Selector* this_compound = SASS_MEMORY_NEW(Compound_Selector, pstate(), 1);
        this_compound->append(SASS_MEMORY_COPY(this));
        Compound_Selector* unified = rhs->at(0)->unify_with(this_compound);
        if (unified == nullptr || unified != this_compound) delete this_compound;

        #ifdef DEBUG_UNIFY
        std::cerr << "> " << debug_call << " = " << "Compound[" << unified->to_string() << "]" << std::endl;
        #endif
        return unified;
      }
    }
    for (const Simple_Selector_Obj& sel : rhs->elements()) {
      if (*this == *sel) {
        #ifdef DEBUG_UNIFY
        std::cerr << "> " << debug_call << " = " << "Compound[" << rhs->to_string() << "]" << std::endl;
        #endif
        return rhs;
      }
    }
    const int lhs_order = this->unification_order();
    size_t i = rhs->length();
    while (i > 0 && lhs_order < rhs->at(i - 1)->unification_order()) --i;
    rhs->insert(rhs->begin() + i, this);
    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " = " << "Compound[" << rhs->to_string() << "]" << std::endl;
    #endif
    return rhs;
  }

  Simple_Selector* Type_Selector::unify_with(Simple_Selector* rhs)
  {
    #ifdef DEBUG_UNIFY
    const std::string debug_call = "unify(Type[" + this->to_string() + "], Simple[" + rhs->to_string() + "])";
    std::cerr << debug_call << std::endl;
    #endif

    bool rhs_ns = false;
    if (!(is_ns_eq(*rhs) || rhs->is_universal_ns())) {
      if (!is_universal_ns()) {
        #ifdef DEBUG_UNIFY
        std::cerr << "> " << debug_call << " = nullptr" << std::endl;
        #endif
        return nullptr;
      }
      rhs_ns = true;
    }
    bool rhs_name = false;
    if (!(name_ == rhs->name() || rhs->is_universal())) {
      if (!(is_universal())) {
        #ifdef DEBUG_UNIFY
        std::cerr << "> " << debug_call << " = nullptr" << std::endl;
        #endif
        return nullptr;
      }
      rhs_name = true;
    }
    if (rhs_ns) {
      ns(rhs->ns());
      has_ns(rhs->has_ns());
    }
    if (rhs_name) name(rhs->name());
    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " = Simple[" << this->to_string() << "]" << std::endl;
    #endif
    return this;
  }

  Compound_Selector* Type_Selector::unify_with(Compound_Selector* rhs)
  {
    #ifdef DEBUG_UNIFY
    const std::string debug_call = "unify(Type[" + this->to_string() + "], Compound[" + rhs->to_string() + "])";
    std::cerr << debug_call << std::endl;
    #endif

    if (rhs->empty()) {
      rhs->append(this);
      #ifdef DEBUG_UNIFY
      std::cerr << "> " << debug_call << " = Compound[" << rhs->to_string() << "]" << std::endl;
      #endif
      return rhs;
    }
    Type_Selector* rhs_0 = Cast<Type_Selector>(rhs->at(0));
    if (rhs_0 != nullptr) {
      Simple_Selector* unified = unify_with(rhs_0);
      if (unified == nullptr) {
        #ifdef DEBUG_UNIFY
        std::cerr << "> " << debug_call << " = nullptr" << std::endl;
        #endif
        return nullptr;
      }
      rhs->elements()[0] = unified;
    } else if (!is_universal() || (has_ns_ && ns_ != "*")) {
      rhs->insert(rhs->begin(), this);
    }
    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " = Compound[" << rhs->to_string() << "]" << std::endl;
    #endif
    return rhs;
  }

  Compound_Selector* Class_Selector::unify_with(Compound_Selector* rhs)
  {
    rhs->has_line_break(has_line_break());
    return Simple_Selector::unify_with(rhs);
  }

  Compound_Selector* Id_Selector::unify_with(Compound_Selector* rhs)
  {
    for (const Simple_Selector_Obj& sel : rhs->elements()) {
      if (Id_Selector* id_sel = Cast<Id_Selector>(sel)) {
        if (id_sel->name() != name()) return nullptr;
      }
    }
    rhs->has_line_break(has_line_break());
    return Simple_Selector::unify_with(rhs);
  }

  Compound_Selector* Pseudo_Selector::unify_with(Compound_Selector* rhs)
  {
    if (is_pseudo_element()) {
      for (const Simple_Selector_Obj& sel : rhs->elements()) {
        if (Pseudo_Selector* pseudo_sel = Cast<Pseudo_Selector>(sel)) {
          if (pseudo_sel->is_pseudo_element() && pseudo_sel->name() != name()) return nullptr;
        }
      }
    }
    return Simple_Selector::unify_with(rhs);
  }

  Selector_List* Complex_Selector::unify_with(Complex_Selector* rhs)
  {
    #ifdef DEBUG_UNIFY
    const std::string debug_call = "unify(Complex[" + this->to_string() + "], Complex[" + rhs->to_string() + "])";
    std::cerr << debug_call << std::endl;
    #endif

    // get last tails (on the right side)
    Complex_Selector* l_last = this->mutable_last();
    Complex_Selector* r_last = rhs->mutable_last();

    // check valid pointers (assertion)
    SASS_ASSERT(l_last, "lhs is null");
    SASS_ASSERT(r_last, "rhs is null");

    // Not sure about this check, but closest way I could check
    // was to see if this is a ruby 'SimpleSequence' equivalent.
    // It seems to do the job correctly as some specs react to this
    if (l_last->combinator() != Combinator::ANCESTOR_OF) return nullptr;
    if (r_last->combinator() != Combinator::ANCESTOR_OF) return nullptr;

    // get the headers for the last tails
    Compound_Selector* l_last_head = l_last->head();
    Compound_Selector* r_last_head = r_last->head();

    // check valid head pointers (assertion)
    SASS_ASSERT(l_last_head, "lhs head is null");
    SASS_ASSERT(r_last_head, "rhs head is null");

    // get the unification of the last compound selectors
    Compound_Selector_Obj unified = r_last_head->unify_with(l_last_head);

    // abort if we could not unify heads
    if (unified == nullptr) return nullptr;

    // move the head
    if (l_last_head->is_universal()) l_last->head({});
    r_last->head(unified);

    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " before weave: lhs=" << this->to_string() << " rhs=" << rhs->to_string() << std::endl;
    #endif

    // create nodes from both selectors
    Node lhsNode = complexSelectorToNode(this);
    Node rhsNode = complexSelectorToNode(rhs);

    // Complex_Selector_Obj fake = unified->to_complex();
    // Node unified_node = complexSelectorToNode(fake);
    // // add to permutate the list?
    // rhsNode.plus(unified_node);

    // do some magic we inherit from node and extend
    Node node = subweave(lhsNode, rhsNode);
    Selector_List_Obj result = SASS_MEMORY_NEW(Selector_List, pstate(), node.collection()->size());
    for (auto &item : *node.collection()) {
      result->append(nodeToComplexSelector(Node::naiveTrim(item)));
    }

    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " = " << result->to_string() << std::endl;
    #endif

    // only return if list has some entries
    return result->length() ? result.detach() : nullptr;
  }

  Selector_List* Selector_List::unify_with(Selector_List* rhs) {
    #ifdef DEBUG_UNIFY
    const std::string debug_call = "unify(List[" + this->to_string() + "], List[" + rhs->to_string() + "])";
    std::cerr << debug_call << std::endl;
    #endif

    std::vector<Complex_Selector_Obj> result;
    // Unify all of children with RHS's children, storing the results in `unified_complex_selectors`
    for (Complex_Selector_Obj& seq1 : elements()) {
      for (Complex_Selector_Obj& seq2 : rhs->elements()) {
        Complex_Selector_Obj seq1_copy = SASS_MEMORY_CLONE(seq1);
        Complex_Selector_Obj seq2_copy = SASS_MEMORY_CLONE(seq2);
        Selector_List_Obj unified = seq1_copy->unify_with(seq2_copy);
        if (unified) {
          result.reserve(result.size() + unified->length());
          std::copy(unified->begin(), unified->end(), std::back_inserter(result));
        }
      }
    }

    // Creates the final Selector_List by combining all the complex selectors
    Selector_List* final_result = SASS_MEMORY_NEW(Selector_List, pstate(), result.size());
    for (Complex_Selector_Obj& sel : result) {
      final_result->append(sel);
    }
    #ifdef DEBUG_UNIFY
    std::cerr << "> " << debug_call << " = " << final_result->to_string() << std::endl;
    #endif
    return final_result;
  }

  Selector_List* Compound_Selector::weaver(Selector_List* rhs) {
    throw std::runtime_error("not yet implemented");
  }

  // https://www.geeksforgeeks.org/longest-common-subsequence/
  // https://www.geeksforgeeks.org/printing-longest-common-subsequence/
  std::vector<Selector_Group_Obj> lcs(Selector_Groups& X, Selector_Groups& Y) {
    std::size_t m = X.length() - 1;
    std::size_t n = Y.length() - 1;

    std::vector<Selector_Group_Obj> lcs;
    if (m == std::string::npos) return lcs;
    if (n == std::string::npos) return lcs;

    #define L(i,j) l[ (i) * m + j ]
    std::size_t* l = new std::size_t[(m+1)*(n+1)];

    /* Following steps build L[m+1][n+1] in bottom up fashion. Note
      that L[i][j] contains length of LCS of X[0..i-1] and Y[0..j-1] */
    for (size_t i = 0; i <= m; i++) {
      for (size_t j = 0; j <= n; j++) {
        if (i == 0 || j == 0)
          L(i, j) = 0;
        else if (*X[i-1] == *Y[j-1])
          L(i, j) = L(i-1, j-1) + 1;
        else
          L(i, j) = std::max(L(i-1, j), L(i, j-1));
      }
    }

    // Following code is used to print LCS
    std::size_t index = L(m, n);

    // Create an array to store the lcs groups
    // std::vector<Selector_Group_Obj> lcs(index);

    // Start from the right-most-bottom-most corner and
    // one by one store objects in lcs[]
    std::size_t i = m, j = n;
    while (i > 0 && j > 0) {

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

  bool parent_superselector(Selector_Group_Obj lhs, Selector_Group_Obj rhs) {
    static Complex_Selector_Obj base = Placeholder_Selector(ParserState("[TMP]"), "<temp>").toComplexSelector();
    lhs = SASS_MEMORY_COPY(lhs); lhs->append(base);
    rhs = SASS_MEMORY_COPY(rhs); rhs->append(base);
    Complex_Selector_Obj lhcs = lhs->toComplexSelector();
    Complex_Selector_Obj rhcs = rhs->toComplexSelector();
    return lhcs->is_superselector_of(rhcs);
  }

  Selector_Groups_Obj chunks(Selector_Groups* seq1, Selector_Groups* seq2, const std::vector<Selector_Group_Obj> &lcs) {
    Selector_Groups_Obj chunks = SASS_MEMORY_NEW(Selector_Groups, ParserState("[TMP]"));
    Selector_Group_Obj chunks1 = SASS_MEMORY_NEW(Selector_Group, ParserState("[TMP]"));
    Selector_Group_Obj chunks2 = SASS_MEMORY_NEW(Selector_Group, ParserState("[TMP]"));

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
      auto lhs = SASS_MEMORY_COPY(chunks1);
      auto rhs = SASS_MEMORY_COPY(chunks2);
      lhs->concat(chunks2);
      rhs->concat(chunks1);
      chunks->append(lhs);
      chunks->append(rhs);
    }

    return chunks;
  }

  Selector_Groups_Obj group_selectors(Complex_Selector* s) {
    Complex_Selector* current = s->mutable_first();
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

  Selector_Groups* paths(const std::vector<Selector_Groups_Obj>& arrs) {
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

  Selector_Group* merge_initial_ops(Complex_Selector** lhsptr, Complex_Selector** rhsptr) {
    Complex_Selector* lhs = *lhsptr;
    Complex_Selector* rhs = *rhsptr;

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

    *lhsptr = lhs;
    *rhsptr = rhs;

    // return the bigger sequence
    return ops1->length() > ops2->length()
      ? ops1.detach() : ops2.detach();
  }

  Selector_List* Complex_Selector::subweaver(Complex_Selector* rhs) {
    if (empty()) { return rhs->toSelectorList(); }
    if (rhs->empty()) { return toSelectorList(); }

    Complex_Selector* lhs = this->mutable_first();
    rhs = rhs->mutable_first();

    auto init = merge_initial_ops(&lhs, &rhs);
    if (init == nullptr) return nullptr;

    // this one will be tricky, as we probably
    // need to create copies to cut them off
    // auto fin = merge_final_ops(lh, rh);
    // if (fin == NULL) return NULL;

    // return unless (init = merge_initial_ops(seq1, seq2))
    // return unless (fin = merge_final_ops(seq1, seq2))

    // this feature seems not very high prio
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
