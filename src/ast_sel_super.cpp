/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* This file contains all ast superselector functions in one compile unit.   */
/*****************************************************************************/
#include "ast_selectors.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // To compare/debug dart-sass vs libsass you can use debugger.hpp:
  // c++: std::cerr << "result " << debug_vec(compound) << "\n";
  // dart: stderr.writeln("result " + compound.toString());
  /////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [list1] is a superselector of [list2].
  // That is, whether [list1] matches every element that
  // [list2] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool listIsSuperslector(
    const sass::vector<ComplexSelectorObj>& list1,
    const sass::vector<ComplexSelectorObj>& list2);

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [complex1] is a superselector of [complex2].
  // That is, whether [complex1] matches every element that
  // [complex2] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool complexIsSuperselector(
    const SelectorComponentVector& complex1,
    const SelectorComponentVector& complex2);

  /////////////////////////////////////////////////////////////////////////
  // Returns all pseudo selectors in [compound] that have
  // a selector argument, and that have the given [name].
  /////////////////////////////////////////////////////////////////////////
  sass::vector<PseudoSelectorObj> selectorPseudoNamed(
    const CompoundSelector* compound, const sass::string& name, bool isClass = true)
  {
    sass::vector<PseudoSelectorObj> rv;
    for (const SimpleSelectorObj& sel : compound->elements()) {
      if (const PseudoSelector* pseudo = sel->isaPseudoSelector()) {
        if (pseudo->isClass() == isClass && pseudo->selector()) {
          if (sel->name() == name) {
            rv.emplace_back(sel);
          }
        }
      }
    }
    return rv;
  }
  // EO selectorPseudoNamed

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [simple1] is a superselector of [simple2].
  // That is, whether [simple1] matches every element that
  // [simple2] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool simpleIsSuperselector(
    const SimpleSelector* simple1,
    const SimpleSelector* simple2)
  {

    if (simple1->isUniversal()) {
      if (!simple2->isUniversal()) return false;
      return simple1->nsMatch(*simple2);
    }

    // If they are equal they are superselectors
    if (PtrObjEqualityFn(simple1, simple2)) {
      return true;
    }
    // Some selector pseudo-classes can match normal selectors.
    if (const PseudoSelector* pseudo = simple2->isaPseudoSelector()) {
      if (pseudo->selector() && isSubselectorPseudo(pseudo->normalized())) {
        for (auto& complex : pseudo->selector()->elements()) {
          // Make sure we have exactly one items
          if (complex->size() != 1) {
            return false;
          }
          // That items must be a compound selector
          if (auto compound = complex->at(0)->isaCompoundSelector()) {
            // It must contain the lhs simple selector
            if (!compound->contains(simple1)) { 
              return false;
            }
          }
        }
        return true;
      }
    }
    return false;
  }
  // EO simpleIsSuperselector

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [simple] is a superselector of [compound].
  // That is, whether [simple] matches every element that
  // [compound] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool simpleIsSuperselectorOfCompound(
    const SimpleSelector* simple,
    const CompoundSelector* compound)
  {
    for (const SimpleSelectorObj& theirSimple : compound->elements()) {
      if (simpleIsSuperselector(simple, theirSimple)) {
        return true;
      }
    }
    return false;
  }
  // EO simpleIsSuperselectorOfCompound

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  bool typeIsSuperselectorOfCompound(
    const TypeSelector* type,
    const CompoundSelector* compound)
  {
    for (const SimpleSelectorObj& simple : compound->elements()) {
      if (const TypeSelector* rhs = simple->isaTypeSelector()) {
        if (!(*type == *rhs)) return true;
      }
    }
    return false;
  }
  // EO typeIsSuperselectorOfCompound

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  bool idIsSuperselectorOfCompound(
    const IDSelector* id,
    const CompoundSelector* compound)
  {
    for (const SimpleSelectorObj& simple : compound->elements()) {
      if (const IDSelector* rhs = simple->isaIDSelector()) {
        if (!(*id == *rhs)) return true;
      }
    }
    return false;
  }
  // EO idIsSuperselectorOfCompound

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  bool pseudoIsSuperselectorOfPseudo(
    const PseudoSelector* pseudo1,
    const PseudoSelector* pseudo2,
    const ComplexSelectorObj& parent
  )
  {
    if (!pseudo2->selector()) return false;
    if (pseudo1->name() == pseudo2->name()) {
      const SelectorList* list = pseudo2->selector();
      return listIsSuperslector(list->elements(), { parent });
    }
    return false;
  }
  // EO pseudoIsSuperselectorOfPseudo

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  bool pseudoNotIsSuperselectorOfCompound(
    const PseudoSelector* pseudo1,
    const CompoundSelector* compound2,
    const ComplexSelectorObj& parent)
  {
    for (const SimpleSelectorObj& simple2 : compound2->elements()) {
      if (const TypeSelectorObj& type2 = simple2->isaTypeSelector()) {
        if (const CompoundSelector* compound1 = parent->last()->isaCompoundSelector()) {
          if (typeIsSuperselectorOfCompound(type2, compound1)) return true;
        }
      }
      else if (const IDSelector* id2 = simple2->isaIDSelector()) {
        if (const CompoundSelector* compound1 = parent->last()->isaCompoundSelector()) {
          if (idIsSuperselectorOfCompound(id2, compound1)) return true;
        }
      }
      else if (const PseudoSelector* pseudo2 = simple2->isaPseudoSelector()) {
        if (pseudoIsSuperselectorOfPseudo(pseudo1, pseudo2, parent)) return true;
      }
    }
    return false;
  }
  // pseudoNotIsSuperselectorOfCompound

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [pseudo1] is a superselector of [compound2].
  // That is, whether [pseudo1] matches every element that [compound2]
  // matches, as well as possibly additional elements. This assumes that
  // [pseudo1]'s `selector` argument is not `null`. If [parents] is passed,
  // it represents the parents of [compound2]. This is relevant for pseudo
  // selectors with selector arguments, where we may need to know if the
  // parent selectors in the selector argument match [parents].
  /////////////////////////////////////////////////////////////////////////
  bool selectorPseudoIsSuperselector(
    const PseudoSelector* pseudo1,
    const CompoundSelector* compound2,
    // ToDo: is this really the most convenient way to do this?
    SelectorComponentVector::const_iterator parents_from,
    SelectorComponentVector::const_iterator parents_to)
  {

    // ToDo: move normalization function
    sass::string name(Util::unvendor(pseudo1->name()));

    if (name == "matches" || name == "any") {
      sass::vector<PseudoSelectorObj> pseudos =
        selectorPseudoNamed(compound2, pseudo1->name());
      const SelectorList* selector1 = pseudo1->selector();
      for (PseudoSelectorObj pseudo2 : pseudos) {
        const SelectorList* selector = pseudo2->selector();
        if (selector1->isSuperselectorOf(selector)) {
          return true;
        }
      }

      for (ComplexSelectorObj complex1 : selector1->elements()) {
        SelectorComponentVector parents;
        for (auto cur = parents_from; cur != parents_to; cur++) {
          parents.emplace_back(*cur);
        }
        parents.push_back(const_cast<CompoundSelector*>(compound2));
        if (complexIsSuperselector(complex1->elements(), parents)) {
          return true;
        }
      }

    }
    else if (name == "has" || name == "host" || name == "host-context" || name == "slotted") {
      sass::vector<PseudoSelectorObj> pseudos =
        selectorPseudoNamed(compound2, pseudo1->name(), name != "slotted");
      const SelectorList* selector1 = pseudo1->selector();
      for (const PseudoSelector* pseudo2 : pseudos) {
        const SelectorList* selector = pseudo2->selector();
        if (selector1->isSuperselectorOf(selector)) {
          return true;
        }
      }

    }
    else if (name == "not") {
      for (ComplexSelectorObj& complex : pseudo1->selector()->elements()) {
        if (!pseudoNotIsSuperselectorOfCompound(pseudo1, compound2, complex)) return false;
      }
      return true;
    }
    else if (name == "current") {
      sass::vector<PseudoSelectorObj> pseudos =
        selectorPseudoNamed(compound2, pseudo1->name());
      for (const PseudoSelector* pseudo2 : pseudos) {
        if (PtrObjEqualityFn(pseudo1, pseudo2)) return true;
      }

    }
    else if (name == "nth-child" || name == "nth-last-child") {
      for (auto simple2 : compound2->elements()) {
        if (const PseudoSelector* pseudo2 = simple2->isaPseudoSelector()) {
          if (pseudo1->name() != pseudo2->name()) continue;
          if (pseudo1->argument() != pseudo2->argument()) continue;
          if (pseudo1->selector()->isSuperselectorOf(pseudo2->selector())) return true;
        }
      }
      return false;
    }

    return false;

  }
  // EO selectorPseudoIsSuperselector

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [compound1] is a superselector of [compound2].
  // That is, whether [compound1] matches every element that [compound2]
  // matches, as well as possibly additional elements. If [parents] is
  // passed, it represents the parents of [compound2]. This is relevant
  // for pseudo selectors with selector arguments, where we may need to
  // know if the parent selectors in the selector argument match [parents].
  /////////////////////////////////////////////////////////////////////////
  bool compoundIsSuperselector(
    const CompoundSelector* compound1,
    const CompoundSelector* compound2,
    // ToDo: is this really the most convenient way to do this?
    const SelectorComponentVector::const_iterator parents_from,
    const SelectorComponentVector::const_iterator parents_to)
  {
    // Every selector in [compound1.components] must have
    // a matching selector in [compound2.components].
    for (const SimpleSelector* simple1 : compound1->elements()) {
      const PseudoSelector* pseudo1 = simple1->isaPseudoSelector();
      if (pseudo1 && pseudo1->selector()) {
        if (!selectorPseudoIsSuperselector(pseudo1, compound2, parents_from, parents_to)) {
          return false;
        }
      }
      else if (!simpleIsSuperselectorOfCompound(simple1, compound2)) {
        return false;
      }
    }
    // [compound1] can't be a superselector of a selector
    // with pseudo-elements that [compound2] doesn't share.
    for (const SimpleSelector* simple2 : compound2->elements()) {
      const PseudoSelector* pseudo2 = simple2->isaPseudoSelector();
      if (pseudo2 && pseudo2->isPseudoElement() && pseudo2->selector() == nullptr) {
        if (!simpleIsSuperselectorOfCompound(pseudo2, compound1)) {
          return false;
        }
      }
    }
    return true;
  }
  // EO compoundIsSuperselector

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [compound1] is a superselector of [compound2].
  // That is, whether [compound1] matches every element that [compound2]
  // matches, as well as possibly additional elements. If [parents] is
  // passed, it represents the parents of [compound2]. This is relevant
  // for pseudo selectors with selector arguments, where we may need to
  // know if the parent selectors in the selector argument match [parents].
  /////////////////////////////////////////////////////////////////////////
  bool compoundIsSuperselector(
    const CompoundSelector* compound1,
    const CompoundSelector* compound2,
    const SelectorComponentVector& parents)
  {
    return compoundIsSuperselector(
      compound1, compound2,
      parents.begin(), parents.end()
    );
  }
  // EO compoundIsSuperselector

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [complex1] is a superselector of [complex2].
  // That is, whether [complex1] matches every element that
  // [complex2] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool complexIsSuperselector(
    const SelectorComponentVector& complex1,
    const SelectorComponentVector& complex2)
  {

    // Selectors with trailing operators are neither superselectors nor subselectors.
    if (!complex1.empty() && complex1.back()->isaSelectorCombinator()) return false;
    if (!complex2.empty() && complex2.back()->isaSelectorCombinator()) return false;

    size_t i1 = 0, i2 = 0;
    while (true) {

      size_t remaining1 = complex1.size() - i1;
      size_t remaining2 = complex2.size() - i2;

      if (remaining1 == 0 || remaining2 == 0) {
        return false;
      }
      // More complex selectors are never
      // superselectors of less complex ones.
      if (remaining1 > remaining2) {
        return false;
      }

      // Selectors with leading operators are
      // neither superselectors nor subselectors.
      if (complex1[i1]->isaSelectorCombinator()) return false;
      if (complex2[i2]->isaSelectorCombinator()) return false;

      const CompoundSelector* compound1 = complex1[i1]->isaCompoundSelector();
      const CompoundSelector* compound2 = complex2.back()->isaCompoundSelector();

      if (remaining1 == 1) {
        SelectorComponentVector::const_iterator parents_to = complex2.end();
        SelectorComponentVector::const_iterator parents_from = complex2.begin();
        std::advance(parents_from, i2); std::advance(parents_to, -1);
        bool rv = compoundIsSuperselector(compound1, compound2, parents_from, parents_to);
        SelectorComponentVector pp;

        SelectorComponentVector::const_iterator end = parents_to;
        SelectorComponentVector::const_iterator beg = parents_from;
        while (beg != end) {
          pp.emplace_back(*beg);
          beg++;
        }

        return rv;
      }

      // Find the first index where `complex2.sublist(i2, afterSuperselector)`
      // is a sub-selector of [compound1]. We stop before the superselector
      // would encompass all of [complex2] because we know [complex1] has 
      // more than one element, and consuming all of [complex2] wouldn't 
      // leave anything for the rest of [complex1] to match.
      size_t afterSuperselector = i2 + 1;
      for (; afterSuperselector < complex2.size(); afterSuperselector++) {
        SelectorComponentObj component2 = complex2[afterSuperselector - 1];
        if (const CompoundSelector* compound2 = component2->isaCompoundSelector()) {
          SelectorComponentVector::const_iterator parents_to = complex2.begin();
          SelectorComponentVector::const_iterator parents_from = complex2.begin();
          // complex2.take(afterSuperselector - 1).skip(i2 + 1)
          std::advance(parents_from, i2 + 1); // equivalent to dart `.skip`
          std::advance(parents_to, afterSuperselector); // equivalent to dart `.take`
          if (compoundIsSuperselector(compound1, compound2, parents_from, parents_to)) {
            break;
          }
        }
      }
      if (afterSuperselector == complex2.size()) {
        return false;
      }

      SelectorComponentObj component1 = complex1[i1 + 1],
        component2 = complex2[afterSuperselector];

      SelectorCombinator* combinator1 = component1->isaSelectorCombinator();
      SelectorCombinator* combinator2 = component2->isaSelectorCombinator();

      if (combinator1 != nullptr) {

        if (combinator2 == nullptr) {
          return false;
        }
        // `.a ~ .b` is a superselector of `.a + .b`,
        // but otherwise the combinators must match.
        if (combinator1->isGeneralCombinator()) {
          if (combinator2->isChildCombinator()) {
            return false;
          }
        }
        else if (!(*combinator1 == *combinator2)) {
          return false;
        }

        // `.foo > .baz` is not a superselector of `.foo > .bar > .baz` or
        // `.foo > .bar .baz`, despite the fact that `.baz` is a superselector of
        // `.bar > .baz` and `.bar .baz`. Same goes for `+` and `~`.
        if (remaining1 == 3 && remaining2 > 3) {
          return false;
        }

        i1 += 2; i2 = afterSuperselector + 1;

      }
      else if (combinator2 != nullptr) {
        if (!combinator2->isChildCombinator()) {
          return false;
        }
        i1 += 1; i2 = afterSuperselector + 1;
      }
      else {
        i1 += 1; i2 = afterSuperselector;
      }
    }

    return false;

  }
  // EO complexIsSuperselector

  /////////////////////////////////////////////////////////////////////////
  // Like [complexIsSuperselector], but compares [complex1]
  // and [complex2] as though they shared an implicit base
  // [SimpleSelector]. For example, `B` is not normally a
  // superselector of `B A`, since it doesn't match elements
  // that match `A`. However, it *is* a parent superselector,
  // since `B X` is a superselector of `B A X`.
  /////////////////////////////////////////////////////////////////////////
  bool complexIsParentSuperselector(
    const SelectorComponentVector& complex1,
    const SelectorComponentVector& complex2)
  {
    // Try some simple heuristics to see if we can avoid allocations.
    if (complex1.empty() && complex2.empty()) return false;
    if (complex1.front()->isaSelectorCombinator()) return false;
    if (complex2.front()->isaSelectorCombinator()) return false;
    if (complex1.size() > complex2.size()) return false;
    // TODO(nweiz): There's got to be a way to do this without a bunch of extra allocations...
    SelectorComponentVector cplx1(complex1);
    SelectorComponentVector cplx2(complex2);
    CompoundSelectorObj base = SASS_MEMORY_NEW(CompoundSelector,
      SourceSpan::tmp("[BASE]"));
    cplx1.emplace_back(base); cplx2.emplace_back(base);
    return complexIsSuperselector(cplx1, cplx2);
  }
  // EO complexIsParentSuperselector

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [list] has a superselector for [complex].
  // That is, whether an item in [list] matches every element that
  // [complex] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool listHasSuperslectorForComplex(
    sass::vector<ComplexSelectorObj> list,
    ComplexSelectorObj complex)
  {
    // Return true if every [complex] selector on [list2]
    // is a super selector of the full selector [list1].
    for (const ComplexSelector* lhs : list) {
      if (complexIsSuperselector(lhs->elements(), complex->elements())) {
        return true;
      }
    }
    return false;
  }
  // listIsSuperslectorOfComplex

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [list1] is a superselector of [list2].
  // That is, whether [list1] matches every element that
  // [list2] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool listIsSuperslector(
    const sass::vector<ComplexSelectorObj>& list1,
    const sass::vector<ComplexSelectorObj>& list2)
  {
    // Return true if every [complex] selector on [list2]
    // is a super selector of the full selector [list1].
    for (const ComplexSelectorObj& complex : list2) {
      if (!listHasSuperslectorForComplex(list1, complex)) {
        return false;
      }
    }
    return true;
  }
  // EO listIsSuperslector

  /////////////////////////////////////////////////////////////////////////
  // Implement selector methods (dispatch to functions)
  /////////////////////////////////////////////////////////////////////////
  bool SelectorList::isSuperselectorOf(const SelectorList* sub) const
  {
    return listIsSuperslector(elements(), sub->elements());
  }
  bool ComplexSelector::isSuperselectorOf(const ComplexSelector* sub) const
  {
    return complexIsSuperselector(elements(), sub->elements());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
