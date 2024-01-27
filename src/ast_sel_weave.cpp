/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "ast_selectors.hpp"

#include "permutate.hpp"
#include "dart_helpers.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Returns whether or not [compound] contains a `::root` selector.
  /////////////////////////////////////////////////////////////////////////
  bool hasRoot(const CompoundSelector* compound)
  {
    for (const SimpleSelector* simple : compound->elements()) {
      if (const PseudoSelector* pseudo = simple->isaPseudoSelector()) {
        if (pseudo->isClass() && pseudo->normalized() == "root") {
          return true;
        }
      }
    }
    return false;
  }
  // EO hasRoot

  bool hasRootish(const CompoundSelector* compound)
  {
    for (const SimpleSelector* simple : compound->elements()) {
      if (const PseudoSelector* pseudo = simple->isaPseudoSelector()) {
        if (pseudo->isClass()) {
          if (pseudo->normalized() == "root") return true;
          if (pseudo->normalized() == "scope") return true;
          if (pseudo->normalized() == "host") return true;
          if (pseudo->normalized() == "host-context") return true;
        }
      }
    }
    return false;
  }
  // EO hasRoot

  /////////////////////////////////////////////////////////////////////////
  // Returns whether a [CompoundSelector] may contain only
  // one simple selector of the same type as [simple].
  /////////////////////////////////////////////////////////////////////////
  bool isUnique(const SimpleSelector* simple)
  {
    if (simple->isaIDSelector()) return true;
    if (const PseudoSelector* pseudo = simple->isaPseudoSelector()) {
      if (pseudo->isPseudoElement()) return true;
    }
    return false;
  }
  // EO isUnique

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [complex1] and [complex2] need to be unified to
  // produce a valid combined selector. This is necessary when both
  // selectors contain the same unique simple selector, such as an ID.
  /////////////////////////////////////////////////////////////////////////
  bool mustUnify(
    const CplxSelComponentVector& complex1,
    const CplxSelComponentVector& complex2)
  {

    sass::vector<const SimpleSelector*> uniqueSelectors1;
    for (const CplxSelComponent* component : complex1) {
      if (const CompoundSelector* compound = component->selector()) {
        for (const SimpleSelector* sel : compound->elements()) {
          if (isUnique(sel)) {
            uniqueSelectors1.emplace_back(sel);
          }
        }
      }
    }
    if (uniqueSelectors1.empty()) return false;
    for (const CplxSelComponent* component : complex2) {
      if (const CompoundSelector* compound = component->selector()) {
        for (const SimpleSelector* sel : compound->elements()) {
          if (isUnique(sel)) {
            for (auto check : uniqueSelectors1) {
              if (*check == *sel) return true;
            }
          }
        }
      }
    }

    return false;
  }
  // EO isUnique

  /////////////////////////////////////////////////////////////////////////
  // Helper function used by `weaveParents`
  /////////////////////////////////////////////////////////////////////////
  bool cmpGroups(
    const CplxSelComponentVector& group1,
    const CplxSelComponentVector& group2,
    CplxSelComponentVector& select)
  {

    if (ListEquality(group1, group2, PtrObjEqualityFn<CplxSelComponent>))
    {
      // std::cerr << ("List is equal\n");
      select = group1;
      return true;
    }

    // std::cerr << "cmp1: " << InspectVector(group1) << "\n";
    // std::cerr << "cmp2: " << InspectVector(group2) << "\n";

    if (!group1.front()->selector()) {
      // std::cerr << "group1 front has no selector\n";
      select = {};
      return false;
    }
    if (!group2.front()->selector()) {
      // std::cerr << "group2 front has no selector\n";
      select = {};
      return false;
    }

    if (complexIsParentSuperselector(group1, group2)) {
      // std::cerr << ("!Complex is parent super 1\n");
      select = group2;
      return true;
    }
    if (complexIsParentSuperselector(group2, group1)) {
      // std::cerr << ("!Complex is parent super 2\n");
      select = group1;
      return true;
    }

    if (!mustUnify(group1, group2)) {
      // std::cerr << ("!Must not unify\n");
      select.clear();
      return false;
    }

    auto span = SourceSpan::internal("[BASE]");
    CplxSelComponentVector comp1(group1);
    CplxSelComponentVector comp2(group2);
    auto q1 = SASS_MEMORY_NEW(ComplexSelector, span, std::move(comp1));
    auto q2 = SASS_MEMORY_NEW(ComplexSelector, span, std::move(comp2));
    auto unified = _unifyComplex({ q2, q1 }, span);
    for (auto q : unified) {
      // std::cerr << "_weaveParents => " << q->toString() << "\n";
    }
    if (unified.size() == 1) {
      select = unified[0]->elements();
    }
    else {
      return false;
    }
    return true;
  }
  // EO cmpGroups

  /////////////////////////////////////////////////////////////////////////
  // Helper function used by `weaveParents`
  /////////////////////////////////////////////////////////////////////////
  template <class T>
  bool checkForEmptyChild(const T& item) {
    return item.empty();
  }
  // EO checkForEmptyChild

  /////////////////////////////////////////////////////////////////////////
  // Helper function used by `weaveParents`
  /////////////////////////////////////////////////////////////////////////
  bool cmpChunkForEmptySequence(
    const sass::vector<CplxSelComponentVector>& seq,
    const CplxSelComponentVector& group)
  {
    return seq.empty();
  }
  // EO cmpChunkForEmptySequence

  /////////////////////////////////////////////////////////////////////////
  // Helper function used by `weaveParents`
  /////////////////////////////////////////////////////////////////////////
  bool cmpChunkForParentSuperselector(
    const sass::vector<CplxSelComponentVector>& seq,
    const CplxSelComponentVector& group)
  {
    return seq.empty() || complexIsParentSuperselector(seq.front(), group);
  }
  // EO cmpChunkForParentSuperselector

 /////////////////////////////////////////////////////////////////////////
 // Returns all orderings of initial subsequences of [queue1] and [queue2].
 // The [done] callback is used to determine the extent of the initial
 // subsequences. It's called with each queue until it returns `true`.
 // Destructively removes the initial subsequences of [queue1] and [queue2].
 // For example, given `(A B C | D E)` and `(1 2 | 3 4 5)` (with `|` denoting
 // the boundary of the initial subsequence), this would return `[(A B C 1 2),
 // (1 2 A B C)]`. The queues would then contain `(D E)` and `(3 4 5)`.
 /////////////////////////////////////////////////////////////////////////
  template <class T>
  sass::vector<sass::vector<T>> getChunks(
    sass::vector<T>& queue1, sass::vector<T>& queue2,
    const T& group, bool(*done)(const sass::vector<T>&, const T&)
  ) {

    sass::vector<T> chunk1;
    while (!done(queue1, group)) {
      chunk1.emplace_back(queue1.front());
      queue1.erase(queue1.begin());
    }

    sass::vector<T> chunk2;
    while (!done(queue2, group)) {
      chunk2.emplace_back(queue2.front());
      queue2.erase(queue2.begin());
    }

    if (chunk1.empty() && chunk2.empty()) return {};
    else if (chunk1.empty()) {
      return { chunk2 };
    }
    else if (chunk2.empty()) {
      return { chunk1 };
    }

    sass::vector<sass::vector<T>> result;
    result.emplace_back(chunk1);
    result.emplace_back(chunk2);
    result.front().insert(result.front().end(),
      std::make_move_iterator(chunk2.begin()),
      std::make_move_iterator(chunk2.end()));
    result.back().insert(result.back().end(),
      std::make_move_iterator(chunk1.begin()),
      std::make_move_iterator(chunk1.end()));
    return result;
  }
  // EO getChunks

  /////////////////////////////////////////////////////////////////////////
  // If the first element of [queue] has a `::root`
  // selector, removes and returns that element.
  /////////////////////////////////////////////////////////////////////////
  CplxSelComponentObj getFirstIfRoot(CplxSelComponentVector& queue) {
    if (queue.empty()) return {};
    CplxSelComponent* first = queue.front();
    if (CompoundSelector* sel = first->selector()) {
      if (!hasRoot(sel)) return {};
      queue.erase(queue.begin());
      return first;
    }
    return {};
  }
  // EO getFirstIfRoot

  CplxSelComponentObj _firstIfRootish(CplxSelComponentVector& queue) {
    if (queue.empty()) return {};
    CplxSelComponent* first = queue.front();
    if (CompoundSelector* sel = first->selector()) {

      if (!hasRootish(sel)) return {};
      queue.erase(queue.begin());
      return first;

    }
    return {};
  }
  // EO getFirstIfRoot

  

  /////////////////////////////////////////////////////////////////////////
  // Returns [complex], grouped into sub-lists such that no sub-list
  // contains two adjacent [ComplexSelector]s. For example,
  // `(A B > C D + E ~ > G)` is grouped into `[(A) (B > C) (D + E ~ > G)]`.
  /////////////////////////////////////////////////////////////////////////
  sass::vector<CplxSelComponentVector> groupSelectors(
    const CplxSelComponentVector& components)
  {
    sass::vector<CplxSelComponentVector> groups;
    CplxSelComponentVector group;
    for (auto component : components) {
      group.push_back(component);
      if (component->combinators().empty()) {
        groups.emplace_back(std::move(group));
        group.clear(); // needed after move?
      }
    }
    if (!group.empty()) {
      groups.emplace_back(group);
    }
    return groups;
  }
  // EO groupSelectors

  /////////////////////////////////////////////////////////////////////////
  // Extracts leading [Combinator]s from [components1] and [components2]
  // and merges them together into a single list of combinators.
  // If there are no combinators to be merged, returns an empty list.
  // If the combinators can't be merged, returns `null`.
  /////////////////////////////////////////////////////////////////////////
  bool mergeInitialCombinators(
    CplxSelComponentVector& components1,
    CplxSelComponentVector& components2,
    CplxSelComponentVector& result)
  {

    CplxSelComponentVector combinators1;
    while (!components1.empty() && components1.front()->selector()) {
      // SelectorCombinator* front = components1.front()->isaSelectorCombinator();
      components1.erase(components1.begin());
      // combinators1.emplace_back(front);
    }

    CplxSelComponentVector combinators2;
    while (!components2.empty() && components2.front()->selector()) {
      // SelectorCombinator* front = components2.front()->isaSelectorCombinator();
      components2.erase(components2.begin());
      // combinators2.emplace_back(front);
    }

    // If neither sequence of combinators is a subsequence
    // of the other, they cannot be merged successfully.
    CplxSelComponentVector LCS = lcs<CplxSelComponentObj>(combinators1, combinators2);

    if (ListEquality(LCS, combinators1, PtrObjEqualityFn<CplxSelComponent>)) {
      result = combinators2;
      return true;
    }
    if (ListEquality(LCS, combinators2, PtrObjEqualityFn<CplxSelComponent>)) {
      result = combinators1;
      return true;
    }

    return false;

  }
  // EO mergeInitialCombinators


  /////////////////////////////////////////////////////////////////////////
  // Expands "parenthesized selectors" in [complexes]. That is, if
  // we have `.A .B {@extend .C}` and `.D .C {...}`, this conceptually
  // expands into `.D .C, .D (.A .B)`, and this function translates
  // `.D (.A .B)` into `.D .A .B, .A .D .B`. For thoroughness, `.A.D .B`
  // would also be required, but including merged selectors results in
  // exponential output for very little gain. The selector `.D (.A .B)`
  // is represented as the list `[[.D], [.A, .B]]`.
  /////////////////////////////////////////////////////////////////////////

  /// Expands "parenthesized selectors" in [complexes].
  ///
  /// That is, if we have `.A .B {@extend .C}` and `.D .C {...}`, this
  /// conceptually expands into `.D .C, .D (.A .B)`, and this function translates
  /// `.D (.A .B)` into `.D .A .B, .A .D .B`. For thoroughness, `.A.D .B` would
  /// also be required, but including merged selectors results in exponential
  /// output for very little gain.
  ///
  /// The selector `.D (.A .B)` is represented as the list `[.D, .A .B]`.
  ///
  /// The [span] will be used for any new combined selectors.
  ///
  /// If [forceLineBreak] is `true`, this will mark all returned complex selectors
  /// as having line breaks.
  ComplexSelector* ComplexSelector::withAdditionalComponent(
    CplxSelComponent* component, SourceSpan& span,
    bool forceLineBreak = false)
    {
    SelectorCombinatorVector combo(leadingCombinators_);
    CplxSelComponentVector comps(elements_);
    comps.push_back(component);
    return SASS_MEMORY_NEW(ComplexSelector, span,
      std::move(combo), std::move(comps),
      hasLineBreak_ || forceLineBreak);
  }

  sass::vector<ComplexSelectorObj> weave(
    const sass::vector<ComplexSelectorObj>& complexes,
    bool forceLineBreak)
  {


    /*


    sass::vector<ComplexSelectorObj> prefixes;

    prefixes.emplace_back(complexes.at(0));

    for (size_t i = 1; i < complexes.size(); i += 1) {

      if (complexes[i]->empty()) {
        continue;
      }
      const ComplexSelectorObj& complex = complexes[i];
      CplxSelComponent* target = complex->elements().back();
      if (complex->size() == 1) {
        for (auto& prefix : prefixes) {
          prefix->elements().push_back(target);
        }
        continue;
      }

      ComplexSelectorObj parents = SASS_MEMORY_COPY(complex);

      parents->elements().pop_back();

      sass::vector<ComplexSelectorObj> newPrefixes;
      for (ComplexSelectorObj prefix : prefixes) {
        sass::vector<ComplexSelectorObj>
          parentPrefixes = weaveParents(prefix, parents);
        if (parentPrefixes.empty()) continue;
        for (auto& parentPrefix : parentPrefixes) {
          parentPrefix->elements().emplace_back(target);
          newPrefixes.push_back(parentPrefix);
        }
      }
      prefixes = newPrefixes;

    }
    return prefixes;

    */

    if (complexes.empty()) return complexes;

    for (auto qwe : complexes) {
      for (auto qwe2 : qwe->elements()) {
        // std::cerr << "weaving [" << qwe2->inspecter() << "]\n";
      }
    }

    if (complexes.size() == 1) {
      auto complex = complexes.front();
      // Force line breaks if required
      return complexes;
    }

    sass::vector<ComplexSelectorObj> prefixes;
    prefixes.emplace_back(complexes.front());

    for (size_t i = 1; i < complexes.size(); i += 1) {
      const ComplexSelectorObj& complex = complexes[i];
      if (complex->elements().size() == 1) {
        for (auto& prefix : prefixes) {
          prefix = prefix->concatenate(complex, complex->pstate(), forceLineBreak);
          // prefix->elements().push_back(complex);
         // prefix->concatenate(complex);
        }
        continue;
      }

      for (auto asd : prefixes) {
        // std::cerr << "weaver : " << asd->inspect() << "\n";
      }

      // CplxSelComponentVector parents(complex);
      // parents.pop_back();

      sass::vector<ComplexSelectorObj> newPrefixes;
      for (const ComplexSelectorObj& prefix : prefixes) {
        sass::vector<ComplexSelectorObj> weaveds
          = weaveParents(prefix, complex);
        if (weaveds.empty()) continue;
        for (const auto& parent : weaveds) {
          SourceSpan span(complex->pstate());
          auto asd = parent->withAdditionalComponent(
            complex->elements().back(),
            span, forceLineBreak);
          newPrefixes.push_back(asd);

        //  // Still returns multiple parents here
        //  prefix->elements().insert(prefix->end(),
        //    std::make_move_iterator(parents.begin()),
        //    std::make_move_iterator(parents.end()));
        }
      }
      prefixes = newPrefixes;

    }
    return prefixes;

  }
  // EO weave


  bool _mergeLeadingCombinators(
    const SelectorCombinatorVector& combinators1,
    const SelectorCombinatorVector& combinators2,
    SelectorCombinatorVector& result)
  {
    if (combinators1.empty()) {
      result = combinators2;
    }
    else if (combinators2.empty()) {
      result = combinators1;
    }
    else if (combinators1.size() > 1) {
      // Nothing to add in this case?
    }
    else if (combinators2.size() > 1) {
      // Nothing to add in this case?
    }
    else if (ListEquality(combinators1, combinators2, PtrObjEqualityFn<SelectorCombinator>)) {
      result = combinators1;
    }
    return true;
  }
    // // Allow null arguments just to make calls to `Iterable.reduce()` easier.
    // switch ((combinators1, combinators2)) {
    //   (null, _) || (_, null) = > null,
    //     (List(length: > 1), _) || (_, List(length: > 1)) = > null,
    //     ([], var combinators) || (var combinators, []) = > combinators,
    //     _ = > listEquals(combinators1, combinators2) ? combinators1 : null
    // };


  template<class T>
  T backOrNull(sass::vector<T> asd) {
    if (asd.size() == 0) return nullptr;
    return asd.back();
  }


  /////////////////////////////////////////////////////////////////////////
  // Interweaves [parents1] and [parents2] as parents of the same target
  // selector. Returns all possible orderings of the selectors in the
  // inputs (including using unification) that maintain the relative
  // ordering of the input. For example, given `.foo .bar` and `.baz .bang`,
  // this would return `.foo .bar .baz .bang`, `.foo .bar.baz .bang`,
  // `.foo .baz .bar .bang`, `.foo .baz .bar.bang`, `.foo .baz .bang .bar`,
  // and so on until `.baz .bang .foo .bar`. Semantically, for selectors A
  // and B, this returns all selectors `AB_i` such that the union over all i
  // of elements matched by `AB_i X` is identical to the intersection of all
  // elements matched by `A X` and all elements matched by `B X`. Some `AB_i`
  // are elided to reduce the size of the output.
  /////////////////////////////////////////////////////////////////////////
  CompoundSelector* unifyCompound(
    CompoundSelector* compound1,
    CompoundSelector* compound2)
  {
    auto result = compound2->elements();
    for (auto simple : compound1->elements()) {
      // std::cerr << "==== unifyCompound [" << simple->inspect() << "][" << compound2->inspect() << "]\n";
      auto unified = simple->unify(result);
      if (unified.empty()) return nullptr;
      // for (auto& foo : unified) { std::cerr << "  ==> " << foo->inspect() << "\n"; }
      result = unified;
    }
    return SASS_MEMORY_NEW(CompoundSelector,
      compound1->pstate(), std::move(result));
  }


  bool _mergeTrailingCombinators(const SourceSpan& span,
    CplxSelComponentVector& components1, CplxSelComponentVector& components2,
    sass::vector<sass::vector<CplxSelComponentVector>>& result)
  {

    // std::cerr << "process trailing\n";

    // for (const auto& c1 : components1) { if (c1) std::cerr << "merge trails in1 " << c1->inspecter() << "\n"; }
    // for (const auto& c2 : components2) { if (c2) std::cerr << "merge trails in2 " << c2->inspecter() << "\n"; }

    SelectorCombinatorVector combinators1, combinators2;
    if (components1.size() > 0) combinators1 = components1.back()->combinators();
    if (components2.size() > 0) combinators2 = components2.back()->combinators();

    if (combinators1.empty() && combinators2.empty()) return true;
    if (combinators1.size() > 1 || combinators2.size() > 1) return false;

    auto first1 = combinators1.empty() ? nullptr : combinators1.front();
    auto first2 = combinators2.empty() ? nullptr : combinators2.front();

    // if (first1 == nullptr) std::cerr << " first1 null\n";
    // else std::cerr << " first1 " << first1->toString() << "\n";
    // if (first2 == nullptr) std::cerr << " first2 null\n";
    // else std::cerr << " first2 " << first2->toString() << "\n";

    if (first1 != nullptr && first2 != nullptr)
    {
      if (first1->isFollowingSibling() && first2->isFollowingSibling()) {
        auto component1 = components1.back();
        auto component2 = components2.back();
        components1.pop_back(); // consumed
        components2.pop_back(); // consumed
        if (component1->selector()->isSuperselectorOf(component2->selector())) {
          result.push_back({ { component2 } });
        }
        else if (component2->selector()->isSuperselectorOf(component1->selector())) {
          result.push_back({ { component1 } });
        }
        else {
          sass::vector<CplxSelComponentVector> choices;
          choices.push_back({ component1, component2 });
          choices.push_back({ component2, component1 });
          if (CompoundSelectorObj unified = unifyCompound(
            component1->selector(),component2->selector())) {
            choices.push_back({ SASS_MEMORY_NEW(CplxSelComponent,
               span, { combinators1.front() }, unified) });
          }
          result.push_back(choices);
        }
        // std::cerr << "Merge case 1\n";
      }
      else if (first1->isFollowingSibling() && first2->isNextSibling()) {
        auto next = components2.back();
        auto following = components1.back();
        components1.pop_back(); // consumed
        components2.pop_back(); // consumed

        // std::cerr << "next1 " << next->inspecter() << "\n";
        // std::cerr << "following1 " << following->inspecter() << "\n";

        if (following->selector()->isSuperselectorOf(next->selector())) {
          result.push_back({ { next } });
        }
        else if (auto unified = unifyCompound(following->selector(), next->selector()))
        {
          SelectorCombinatorVector asd = next->combinators();
          result.push_back({
            {following, next},
            { new CplxSelComponent(span, std::move(asd), unified)}
            });
        }
        else {
          result.push_back({
            {following, next}
            });
        }
        // std::cerr << "Merge case 2a\n";
      }
      else if (first1->isNextSibling() && first2->isFollowingSibling()) {
        auto next = components1.back();
        auto following = components2.back();
        components1.pop_back(); // consumed
        components2.pop_back(); // consumed

        // std::cerr << "next2 " << next->inspecter() << "\n";
        // std::cerr << "following2 " << following->inspecter() << "\n";

        if (following->selector()->isSuperselectorOf(next->selector())) {
          result.push_back({ { next } });
        }
        else if (auto unified = unifyCompound(following->selector(), next->selector()))
        {
          SelectorCombinatorVector asd = next->combinators();
          result.push_back({
            {following, next},
            { new CplxSelComponent(span, std::move(asd), unified)}
            });
        }
        else {
          result.push_back({
            {following, next}
            });
        }
        // std::cerr << "Merge case 2b\n";
      }
      else if (first1->isChild() && !first2->isChild()) {
        result.push_back({ { components2.back() } });
        components2.pop_back(); // has been consumed
        // std::cerr << "Merge case 3\n";
      }
      else if (!first1->isChild() && first2->isChild()) {
        result.push_back({ { components1.back() } });
        components1.pop_back(); // has been consumed
        // std::cerr << "Merge case 3\n";
      }
      else if (first1->combinator() == first2->combinator()) {

        auto lst1 = components1.back();
        components1.pop_back();

        auto lst2 = components2.back();
        components2.pop_back();

        auto unified = unifyCompound(
          lst1->selector(), lst2->selector());
        if (unified == nullptr) return false;

        // std::cerr << " cmp " << unified->inspect() << "\n";

        result.push_back({ { SASS_MEMORY_NEW(CplxSelComponent, span, { combinators1.front() }, unified) } });

        // std::cerr << "Merge case 4\n";
      }
      else {
        // std::cerr << "Merge case other\n";
        return false;
      }
    }
    else if (first1 != nullptr)
    {
      auto descendantComponents = backOrNull(components2);
      auto combinatorComponents = backOrNull(components1);

      //std::cerr << "descendantComponents " << descendantComponents->inspecter() << "\n";
      //std::cerr << "combinatorComponents " << combinatorComponents->inspecter() << "\n";

      if (first1->isChild()) {
        if (descendantComponents > 0 && descendantComponents->selector()->isSuperselectorOf(combinatorComponents->selector())) {
          components2.pop_back();
        }
      }
      result.push_back({ { components1.back() } });
      components1.pop_back();
      // std::cerr << "Merge case 5a\n";
    }
    else if (first2 != nullptr)
    {
      auto descendantComponents = backOrNull(components1);
      auto combinatorComponents = backOrNull(components2);
      if (first2->isChild()) {
        if (descendantComponents > 0 && descendantComponents->selector()->isSuperselectorOf(combinatorComponents->selector())) {
          components1.pop_back();
        }
      }
      result.push_back({ { components2.back() } });
      components2.pop_back();
      // std::cerr << "Merge case 5b\n";
    }
    else {
      return false;
    }

    for (auto f1 : components1) {
      // std::cerr << "final trail merge 1 " << f1->inspecter() << "\n";
    }
    for (auto f2 : components2) {
      // std::cerr << "final trail merge 2 " << f2->inspecter() << "\n";
    }

    return _mergeTrailingCombinators(span, components1, components2, result);;

  }



  /////////////////////////////////////////////////////////////////////////
  // Extracts trailing [Combinator]s, and the selectors to which they apply,
  // from [components1] and [components2] and merges them together into a
  // single list. If there are no combinators to be merged, returns an
  // empty list. If the sequences can't be merged, returns `null`.
  /////////////////////////////////////////////////////////////////////////
  bool mergeFinalCombinators(
    CplxSelComponentVector& components1,
    CplxSelComponentVector& components2,
    sass::vector<sass::vector<CplxSelComponentVector>>& result)
  {

    if (components1.empty() || components1.back()->combinators().empty()) {
      if (components2.empty() || components2.back()->combinators().empty()) {
        return true;
      }
    }

    CplxSelComponentVector combinators1;
    while (!components1.empty() && components1.back()->combinators().size() != 0) {
      //SelectorCombinatorObj back = components1.back()->combinators();
      components1.erase(components1.end() - 1);
      //combinators1.emplace_back(back);
    }

    CplxSelComponentVector combinators2;
    while (!components2.empty() && components2.back()->combinators().size() != 0) {
      //SelectorCombinatorObj back = components2.back()->combinators();
      components2.erase(components2.end() - 1);
      //combinators2.emplace_back(back);
    }

    // reverse now as we used emplace_back (faster than new alloc)
    std::reverse(combinators1.begin(), combinators1.end());
    std::reverse(combinators2.begin(), combinators2.end());

    if (combinators1.size() > 1 || combinators2.size() > 1) {
      // If there are multiple combinators, something strange going on. If one
      // is a super-sequence of the other, use that, otherwise give up.
      auto LCS = lcs<CplxSelComponentObj>(combinators1, combinators2);
      if (ListEquality(LCS, combinators1, PtrObjEqualityFn<CplxSelComponent>)) {
        result.push_back({ combinators2 });
      }
      else if (ListEquality(LCS, combinators2, PtrObjEqualityFn<CplxSelComponent>)) {
        result.push_back({ combinators1 });
      }
      else {
        return false;
      }
      return true;
    }

    // This code looks complicated, but it's actually just a bunch of special
    // cases for interactions between different combinators.
    SelectorCombinatorObj combinator1, combinator2;
    //if (!combinators1.empty()) combinator1 = combinators1.back()->isaSelectorCombinator();
    //if (!combinators2.empty()) combinator2 = combinators2.back()->isaSelectorCombinator();

    if (!combinator1.isNull() && !combinator2.isNull()) {

      // CompoundSelector* compound1 = components1.back()->selector();
      // CompoundSelector* compound2 = components2.back()->selector();

      components1.pop_back();
      components2.pop_back();
      /*
      if (combinator1->isGeneralCombinator() && combinator2->isGeneralCombinator()) {

        if (compound1->isSuperselectorOf(compound2)) {
          result.push_back({ { compound2, combinator2.ptr() } });
        }
        else if (compound2->isSuperselectorOf(compound1)) {
          result.push_back({ { compound1, combinator1.ptr() } });
        }
        else {
          sass::vector<CplxSelComponentVector> choices;
          choices.push_back({ compound1, combinator1.ptr(), compound2, combinator2.ptr() });
          choices.push_back({ compound2, combinator2.ptr(), compound1, combinator1.ptr() });
          if (CompoundSelector* unified = compound1->unifyWith(compound2)) {
            choices.push_back({ unified, combinator1.ptr() });
          }
          result.emplace_back(choices);
        }
      }
      else if ((combinator1->isGeneralCombinator() && combinator2->isAdjacentCombinator()) ||
        (combinator1->isAdjacentCombinator() && combinator2->isGeneralCombinator())) {

        CompoundSelector* followingSiblingSelector = combinator1->isGeneralCombinator() ? compound1 : compound2;
        CompoundSelector* nextSiblingSelector = combinator1->isGeneralCombinator() ? compound2 : compound1;
        SelectorCombinator* followingSiblingCombinator = combinator1->isGeneralCombinator() ? combinator1 : combinator2;
        SelectorCombinator* nextSiblingCombinator = combinator1->isGeneralCombinator() ? combinator2 : combinator1;

        if (followingSiblingSelector->isSuperselectorOf(nextSiblingSelector)) {
          result.push_back({ { nextSiblingSelector, nextSiblingCombinator } });
        }
        else {
          CompoundSelectorObj unified = compound1->unifyWith(compound2);
          sass::vector<CplxSelComponentVector> items;

          if (!unified.isNull()) {
            items.push_back({
              unified.ptr(), nextSiblingCombinator
            });
          }

          items.insert(items.begin(), {
            followingSiblingSelector,
            followingSiblingCombinator,
            nextSiblingSelector,
            nextSiblingCombinator,
          });

          result.emplace_back(items);
        }

      }
      else if (combinator1->isChildCombinator() && (combinator2->isAdjacentCombinator() || combinator2->isGeneralCombinator())) {
        result.push_back({ { compound2, combinator2.ptr() } });
        components1.emplace_back(compound1);
        components1.emplace_back(combinator1);
      }
      else if (combinator2->isChildCombinator() && (combinator1->isAdjacentCombinator() || combinator1->isGeneralCombinator())) {
        result.push_back({ { compound1, combinator1.ptr() } });
        components2.emplace_back(compound2);
        components2.emplace_back(combinator2);
      }
      else if (*combinator1 == *combinator2) {
        CompoundSelectorObj unified = compound1->unifyWith(compound2);
        if (unified.isNull()) return false;
        result.push_back({ { unified.ptr(), combinator1.ptr() } });
      }
      else {
        return false;
      }
      */

      return mergeFinalCombinators(components1, components2, result);

    }
    // else if (!combinator1.isNull()) {
      /*
      if (combinator1->isChildCombinator() && !components2.empty()) {
        const CompoundSelector* back1 = components1.back()->isaCompoundSelector();
        const CompoundSelector* back2 = components2.back()->isaCompoundSelector();
        if (back1 && back2 && back2->isSuperselectorOf(back1)) {
          components2.pop_back();
        }
      }

      result.push_back({ { components1.back(), combinator1.ptr() } });

      components1.pop_back();
      */
      //return mergeFinalCombinators(components1, components2, result);

    //}
    /*
    if (combinator2->isChildCombinator() && !components1.empty()) {
      const CompoundSelector* back1 = components1.back()->isaCompoundSelector();
      const CompoundSelector* back2 = components2.back()->isaCompoundSelector();
      if (back1 && back2 && back1->isSuperselectorOf(back2)) {
        components1.pop_back();
      }
    }

    result.push_back({ { components2.back(), combinator2.ptr() } });

    components2.pop_back();
    */
    return mergeFinalCombinators(components1, components2, result);

  }
  // EO mergeFinalCombinators




  sass::vector<ComplexSelectorObj> weaveParents(
    ComplexSelector* prefix, ComplexSelector* base)
  {

    //std::cerr << "wp prefix " << prefix->inspect() << "\n";
    //std::cerr << "wp base " << base->inspect() << "\n";

    SelectorCombinatorVector lead;
    bool rs1 = _mergeLeadingCombinators(
      prefix->leadingCombinators(),
      base->leadingCombinators(),
      lead);

    // _mergeLeadingCombinators must report success or not
    if (rs1 == false) return {};

    // for (SelectorCombinator* asd : lead) { std::cerr << "merged lead " << asd->toString() << "\n"; }

    if (base->empty()) throw "Need base";

    CplxSelComponentVector leads;


    CplxSelComponentVector queue1(prefix->begin(), prefix->end());
    CplxSelComponentVector queue2(base->begin(), base->end() - 1);

    for (auto g1 : queue1) {
      //std::cerr << "q1: " << g1->inspecter() << "\n";
    }
    for (auto g2 : queue2) {
      //std::cerr << "q2: " << g2->inspecter() << "\n";
    }


    // sass::vector<sass::vector<CplxSelComponentVector>> trails;

    sass::vector<sass::vector<CplxSelComponentVector>> trails;
    bool ok = _mergeTrailingCombinators(
      base->pstate(), queue1, queue2, trails);

    if (ok == false) return {};

    for (auto g2 : queue2) {
      //std::cerr << "after q2: " << g2->inspecter() << "\n";
    }


    // list comes out in reverse order for performance
    std::reverse(trails.begin(), trails.end());

    for (auto as : trails) {
      //std::cerr << "---\n";
      for (auto bs : as) {
        //std::cerr << "{{{\n";
        for (auto cs : bs) {
          //std::cerr << "merged trail " << cs->toString() << "\n";
        }
        //std::cerr << "}}}\n";
      }
    }


    //    std::cerr << "======= OK\n";
    // if (!mergeInitialCombinators(queue1, queue2, leads)) return {};
    // if (!mergeFinalCombinators(queue1, queue2, trails)) return {};


    // Make sure there's at most one `:root` in the output.
    // Note: does not yet do anything in libsass (no root selector)
    CplxSelComponentObj root1(_firstIfRootish(queue1));
    CplxSelComponentObj root2(_firstIfRootish(queue2));

    if (!root1.isNull() && !root2.isNull()) {
      // CompoundSelectorObj root = root1->selector()->unifyWith(root2->selector());
      CompoundSelectorObj root = unifyCompound(root1->selector(), root2->selector());
      if (root.isNull()) return {}; // null
      queue1.insert(queue1.begin(), root.ptr()->wrapInComponent(root1->combinators()));
      queue2.insert(queue2.begin(), root.ptr()->wrapInComponent(root2->combinators()));
    }
    else if (!root1.isNull()) {
      queue1.insert(queue1.begin(), root1.ptr());
      queue2.insert(queue2.begin(), root1.ptr());
    }
    else if (!root2.isNull()) {
      queue1.insert(queue1.begin(), root2.ptr());
      queue2.insert(queue2.begin(), root2.ptr());
    }

    //for (auto g1 : queue1) { std::cerr << "q1: " << g1->inspect() << "\n"; }
    //for (auto g2 : queue2) { std::cerr << "q2: " << g2->inspect() << "\n"; }

    // group into sub-lists so no sub-list contains two adjacent ComplexSelectors.
    sass::vector<CplxSelComponentVector> groups1 = groupSelectors(queue1);
    sass::vector<CplxSelComponentVector> groups2 = groupSelectors(queue2);

    //for (auto g1 : groups1) { std::cerr << "g1: " << InspectVector(g1) << "\n"; }
    //for (auto g2 : groups2) { std::cerr << "g2: " << InspectVector(g2) << "\n"; }

    // The main array to store our choices that will be permutated
    sass::vector<sass::vector<CplxSelComponentVector>> choices;

    // append initial combinators
//    choices.push_back({ std::move(leads) });

    // std::cerr << "---- start lcs\n";

    // std::reverse(groups2.begin(), groups2.end());

    sass::vector<CplxSelComponentVector> LCS =
      lcs<CplxSelComponentVector>(groups1, groups2, cmpGroups);

    //std::cerr << "---- got lcs:\n";

    for (auto g2 : LCS) {
      for (auto g : g2) {
        //std::cerr << "lcs: " << g->inspecter() << "\n";
      }
    }

    //std::cerr << "---- EO lcs: !!!!!!!\n";

    for (auto group : LCS) {

      // Create junks from groups1 and groups2
      sass::vector<sass::vector<CplxSelComponentVector>>
        chunks = getChunks<CplxSelComponentVector>(
          groups1, groups2, group, cmpChunkForParentSuperselector);

      // Create expanded array by flattening chunks2 inner
      sass::vector<CplxSelComponentVector>
        expanded = flattenInner(chunks);

      // Prepare data structures
      choices.emplace_back(expanded);
      choices.push_back({ group });
      if (!groups1.empty()) {
        groups1.erase(groups1.begin());
      }
      if (!groups2.empty()) {
        groups2.erase(groups2.begin());
      }

    }

    //// std::cerr << "============================== HERE\n";

    for (auto g1 : groups1) {
      for (auto g : g1) {
        //std::cerr << "g1: " << g->inspecter() << "\n";
      }
    }
    for (auto g2 : groups2) {
      for (auto g : g2) {
        //std::cerr << "g2: " << g->inspecter() << "\n";
      }
    }

    // Create junks from groups1 and groups2
    sass::vector<sass::vector<CplxSelComponentVector>>
      chunks = getChunks<CplxSelComponentVector>(
        groups1, groups2, {}, cmpChunkForEmptySequence);

    // Append chunks with inner arrays flattened
    choices.emplace_back(flattenInner(chunks));

    // append all trailing selectors to choices
    choices.insert(choices.end(),
      std::make_move_iterator(trails.begin()),
      std::make_move_iterator(trails.end()));

    // move all non empty items to the front, then erase the trailing ones
    choices.erase(std::remove_if(choices.begin(), choices.end(), checkForEmptyChild
      <sass::vector<CplxSelComponentVector>>), choices.end());

    for (auto g2 : choices) {
      //std::cerr << "---\n";
      for (auto g3 : g2) {
        //std::cerr << "[[[\n";
        for (auto g : g3) {
          //std::cerr << "- choice: " << g->inspecter() << "\n";
        }
        //std::cerr << "]]]\n";
      }
    }

    auto perm = permutate(choices);

    for (auto g2 : perm) {
      for (auto g3 : g2) {
        for (auto g : g3) {
          //std::cerr << "+ path: " << g->inspecter() << "\n";
        }
      }
    }

    sass::vector<ComplexSelectorObj> foobar;
    for (auto path : perm) {
      CplxSelComponentVector comps;
      for (auto compis : path) {
        for (auto compa : compis) {
          comps.push_back(compa);
        }
      }
      auto cply = SASS_MEMORY_NEW(ComplexSelector, base->pstate(),
        lead, std::move(comps));
      foobar.push_back(cply);
    }

    // permutate all possible paths through selectors
    // auto qwe = flattenInner(perm);
    for (auto g : foobar) {
      //std::cerr << "flat: " << g->inspect() << "\n";
    }

    return foobar;

  }
  // EO weaveParents

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
