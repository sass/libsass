/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* This file contains all ast superselector functions in one compile unit.   */
/*****************************************************************************/
#include "ast_selectors.hpp"

#include "dart_helpers.hpp"

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
    const CplxSelComponentVector& complex1,
    const CplxSelComponentVector& complex2);

  /////////////////////////////////////////////////////////////////////////
  // Returns all pseudo selectors in [compound] that have
  // a selector argument, and that have the given [name].
  /////////////////////////////////////////////////////////////////////////
  sass::vector<PseudoSelectorObj> _selectorPseudoArgs(
    const CompoundSelector* compound, const sass::string& name, bool isClass = true)
  {
    sass::vector<PseudoSelectorObj> rv;
    for (const SimpleSelectorObj& sel : compound->elements()) {
      if (const PseudoSelectorObj& pseudo = sel->isaPseudoSelector()) {
        if (pseudo->isClass() == isClass && pseudo->selector()) {
          if (sel->name() == name) {
            rv.emplace_back(pseudo);
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
      // if (!simple2->isUniversal()) return false;
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
//          if (auto compound = complex->at(0)->isaCompoundSelector()) {
//            // It must contain the lhs simple selector
//            if (!compound->contains(simple1)) { 
//              return false;
//            }
//          }
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
        if (const CompoundSelector* compound1 = parent->last()->selector()->isaCompoundSelector()) {
          if (typeIsSuperselectorOfCompound(type2, compound1)) return true;
        }
      }
      else if (const IDSelector* id2 = simple2->isaIDSelector()) {
        if (const CompoundSelector* compound1 = parent->last()->selector()->isaCompoundSelector()) {
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
  bool _selectorPseudoIsSuperselector(
    const PseudoSelector* pseudo1,
    const CompoundSelector* compound2,
    // ToDo: is this really the most convenient way to do this?
    CplxSelComponentVector::const_iterator parents_from,
    CplxSelComponentVector::const_iterator parents_to)
  {

    auto selector1 = pseudo1->selector();

    if (selector1 == nullptr) {
      throw ("Selector $pseudo1 must have a selector argument.");
    }

    // ToDo: move normalization function
    sass::string name(StringUtils::unvendor(pseudo1->normalized()));

    if (name == "is" || name == "matches" || name == "any" || name == "where") {

      sass::vector<PseudoSelectorObj> pseudos =
        _selectorPseudoArgs(compound2, pseudo1->name());

      for (auto selector2 : pseudos) {
        if (selector1->isSuperselectorOf(selector2->selector())) {
          // std::cerr << ("---- true1\n");
          return true;
        }
      }
      for (auto complex1 : selector1->elements()) {
        if (!complex1->leadingCombinators().empty()) continue;
        CplxSelComponentVector parents(parents_from, parents_to);
        parents.push_back(const_cast<CompoundSelector*>(compound2)->wrapInComponent({}));
        if (complexIsSuperselector(complex1->elements(), parents)) {
          // std::cerr << ("---- true2\n");
          return true;
        }
      }

      // std::cerr << ("---- false\n");
      return false;

      //return pseudos.any((selector2) = > selector1.isSuperselector(selector2)) ||
      //  selector1.components.any((complex1) = >
      //    complex1.leadingCombinators.isEmpty &&
      //    complexIsSuperselector(complex1.components, [
      //      ... ? parents,
      //        ComplexSelectorComponent(compound2, const [], compound2.span)
      //    ]));
      //const SelectorList* selector1 = pseudo1->selector();
      //for (PseudoSelectorObj pseudo2 : pseudos) {
      //  const SelectorList* selector = pseudo2->selector();
      //  if (selector1->isSuperselectorOf(selector)) {
      //    return true;
      //  }
      //}

      for (ComplexSelectorObj complex1 : selector1->elements()) {
//        CplxSelComponentVector parents;
//        for (auto cur = parents_from; cur != parents_to; cur++) {
//          parents.emplace_back(*cur);
//        }
//        parents.push_back(const_cast<CompoundSelector*>(compound2));
//        if (complexIsSuperselector(complex1->elements(), parents)) {
//          return true;
//        }
      }

    }
    else if (name == "has" || name == "host" || name == "host-context") {
      sass::vector<PseudoSelectorObj> pseudos =
        _selectorPseudoArgs(compound2, pseudo1->name(), name != "slotted");
      const SelectorList* selector1 = pseudo1->selector();
      for (const PseudoSelector* pseudo2 : pseudos) {
        const SelectorList* selector = pseudo2->selector();
        if (selector1->isSuperselectorOf(selector)) {
          return true;
        }
      }
    }
    else if (name == "not") {

      for (const ComplexSelectorObj& complex : pseudo1->selector()->elements()) {
        if (!pseudoNotIsSuperselectorOfCompound(pseudo1, compound2, complex)) return false;
      }
      return true;

    }
    else if (name == "current") {
      sass::vector<PseudoSelectorObj> pseudos =
        _selectorPseudoArgs(compound2, pseudo1->name());
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

  /// If [compound] contains a pseudo-element, returns it and its index in
/// [compound.components].
  PseudoSelector* _findPseudoElementIndexed(const CompoundSelector* compound, size_t& n)
  {
    for (size_t i = 0; i < compound->elements().size(); i++) {
      auto simple = compound->elements()[i];
      if (auto pseudo = simple->isaPseudoSelector()) {
        if (pseudo->isElement()) {
          n = i; return pseudo;
        }
      }
    }
    return nullptr;
  }

  /// Like [compoundIsSuperselector] but operates on the underlying lists of
/// simple selectors.
///
/// The [compound1] and [compound2] are expected to have efficient
/// [Iterable.length] fields.
  bool _compoundComponentsIsSuperselector(
    sass::vector<SimpleSelectorObj> compound1,
    sass::vector<SimpleSelectorObj> compound2,
    sass::vector<CplxSelComponentObj> parents)
  {
    if (compound1.empty()) return true;
    if (compound2.empty()) {
      sass::string name("*");
      sass::string ns("*");
      compound2.push_back(
        new TypeSelector(
          SourceSpan::internal("FAKE"),
          std::move(name),
          std::move(ns)));
    }
    auto bogus = SourceSpan::internal("FAKE");
    return compoundIsSuperselector(
      new CompoundSelector(bogus, std::move(compound1)),
      new CompoundSelector(bogus, std::move(compound2)),
      parents);
  }



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
    const CplxSelComponentVector::const_iterator parents_from,
    const CplxSelComponentVector::const_iterator parents_to)
  {
    /*
    size_t n1; size_t n2;
    auto pseudo1 = _findPseudoElementIndexed(compound1, n1);
    auto pseudo2 = _findPseudoElementIndexed(compound2, n2);

    if (pseudo1 && pseudo2) {
      pseudo1->isSuperselector
      // return pseudo1->isSuperselector(pseudo2) &&
      //   _compoundComponentsIsSuperselector(compound1.components.take(index1),
      //     compound2.components.take(index2), parents: parents) &&
      //   _compoundComponentsIsSuperselector(
      //     compound1.components.skip(index1 + 1),
      //     compound2.components.skip(index2 + 1),
      //     parents: parents);
    }
    else if (pseudo1 || pseudo2) {
      return false;
    }
    */

    // Every selector in [compound1.components] must have
    // a matching selector in [compound2.components].
    for (const SimpleSelector* simple1 : compound1->elements()) {
      const PseudoSelector* pseudo1 = simple1->isaPseudoSelector();
      if (pseudo1 && pseudo1->selector()) {
        if (!_selectorPseudoIsSuperselector(pseudo1, compound2, parents_from, parents_to)) {
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
    const CplxSelComponentVector& parents)
  {

    size_t n1; size_t n2;
    auto pseudo1 = _findPseudoElementIndexed(compound1, n1);
    auto pseudo2 = _findPseudoElementIndexed(compound2, n2);

    if (pseudo1 && pseudo2) {

      // std::cerr << "both are pseudo\n";
      if (pseudo1->isSuperselectorAF(pseudo2)) {
        // std::cerr << "First condition is true\n";
        auto l1 = sass::vector<SimpleSelectorObj>(compound1->begin(), compound1->begin() + n1);
        auto l2 = sass::vector<SimpleSelectorObj>(compound2->begin(), compound2->begin() + n2);
        if (!_compoundComponentsIsSuperselector(l1, l2, parents)) return false;
        // std::cerr << "Second condition is true\n";
        auto e1 = sass::vector<SimpleSelectorObj>(compound1->begin() + n1 + 1, compound1->end());
        auto e2 = sass::vector<SimpleSelectorObj>(compound2->begin() + n2 + 1, compound2->end());
        return _compoundComponentsIsSuperselector(e1, e2, parents);
      }
    }
    else if (pseudo1 || pseudo2) {
      return false;
    }

    // Every selector in [compound1.components] must have a matching selector in
// [compound2.components].
    for (auto simple1 : compound1->elements()) {
      // std::cerr << "Go check " << simple1->inspect() << "\n";
      // if (simple1 case PseudoSelector(selector: _ ? )) {
      auto pseudo = simple1->isaPseudoSelector();
      if (pseudo && pseudo->selector() != nullptr) {
        // std::cerr << "-- Check another pseudo inner\n";
        if (!_selectorPseudoIsSuperselector(pseudo, compound2,
          parents.begin(), parents.end())) {
          return false;
        }
      }
      else {
        // std::cerr << "Check via simple.isSuperselector\n";
        bool any = false;
        // std::cerr << "+++++++++ has any " << simple1->inspect() << "\n";
        for (auto& s2 : compound2->elements()) {
          if (simple1->isSuperselectorAF(s2)) {
            any = true;
            break;
          }
        }
        if (!any) {
          // std::cerr << "Has a non super selector\n";
          return false;
        }
        // std::cerr << "Checked via simple.isSUper\n";
      }
    }
    
    return true;
  }
  // EO compoundIsSuperselector



  bool SimpleSelector::isSuperselector(SimpleSelector* other) const
  {
    // std::cerr << "Simple::isSuper\n";
    // Check if matching dart
    return simpleIsSuperselector(this, other);
  }


  bool _compatibleWithPreviousCombinator(SelectorCombinator* previous,
    const CplxSelComponentVector& parents)
  {
    if (parents.empty()) return true;
    if (previous == nullptr) return true;

    // The child and next sibling combinators require that the *immediate*
    // following component be a superslector.
    if (!previous->isFollowingSibling()) return false;

    // The following sibling combinator does allow intermediate components, but
    // only if they're all siblings.

    for (auto& component : parents) {
      if (!component->combinators().empty()) {
        auto first = component->combinators().front();
        if (first->isFollowingSibling()) continue;
        if (first->isNextSibling()) continue;
      }
      return false;
    }

    return true;
  }

  /// Returns whether [combinator1] is a supercombinator of [combinator2].
///
/// That is, whether `X combinator1 Y` is a superselector of `X combinator2 Y`.
  bool _isSupercombinator2(
    SelectorCombinator* combinator1,
    SelectorCombinator* combinator2)
  {
    if (combinator1 == nullptr) return !combinator2 || combinator2->isChild();
    if (combinator2 == nullptr) return false;
    auto qwe = combinator1->combinator() == combinator2->combinator() ||
      (combinator1 == nullptr && combinator2->isChild()) ||
      (combinator1->isFollowingSibling() &&
        combinator2->isNextSibling());
    return qwe;
  }
  template<class T>
  sass::string ToString(T* asd) {
    if (asd == nullptr) return "nullptr";
    return asd->toString();
  }

  bool _isSupercombinator(
    SelectorCombinator* combinator1,
    SelectorCombinator* combinator2)
  {
    auto qwe = _isSupercombinator2(combinator1, combinator2);
    // std::cerr << "isSupercombi " << ToString(combinator1) << " vs " << ToString(combinator2) << " => " << qwe << "\n";
    return qwe;
  }



  template<class T>
  T frontOrNull(sass::vector<T> asd) {
    if (asd.size() == 0) return nullptr;
    return asd.front();
  }

  /////////////////////////////////////////////////////////////////////////
  // Returns whether [complex1] is a superselector of [complex2].
  // That is, whether [complex1] matches every element that
  // [complex2] matches, as well as possibly additional elements.
  /////////////////////////////////////////////////////////////////////////
  bool complexIsSuperselector(
    const CplxSelComponentVector& complex1,
    const CplxSelComponentVector& complex2)
  {

    // ADD AGAIN

    //std::cerr << "complexIsSuper1 " << InspectVector(complex1) << "\n";
    //std::cerr << "complexIsSuper2 " << InspectVector(complex2) << "\n";

    if (complex1.size() == 2 && complex2.size() == 3) {
    }
    // complexIsSuper1[b, % <temp>]
    // complexIsSuper2[a + , b, % <temp>]

    // std::cerr << "# EXEC complexIsSuperselector\n";

    if (complex1.empty()) return false;
    if (complex2.empty()) return false;

    if (!complex1.back()->combinators().empty()) return false;
    if (!complex2.back()->combinators().empty()) return false;

    size_t i1 = 0;
    size_t i2 = 0;

    SelectorCombinator* previousCombinator = nullptr;
    while (true) {

      // std::cerr << "Loop " << i1 << " " << i2 << "\n";
      size_t remaining1 = complex1.size() - i1;
      size_t remaining2 = complex2.size() - i2;
      // std::cerr << "Remain " << i1 << " " << i2 << "\n";

      if (remaining1 == 0 || remaining2 == 0) {
        // std::cerr << "No more remains\n";
        return false;
      }

      // More complex selectors are never superselectors of less complex ones.
      if (remaining1 > remaining2) {
        // std::cerr << "More complex\n";
        return false;
      }

      auto component1 = complex1[i1];

      if (component1->combinators().size() > 1) {
        // std::cerr << "invalid combinators\n";
        return false;
      }
      if (remaining1 == 1) {

        CplxSelComponentVector parents(
          complex2.begin() + i2,
          complex2.end() - 1);

        for (auto p : parents) {
          if (p->combinators().size() > 1) {
            // std::cerr << "invalid parent\n";
            return false;
          }
        }

        // std::cerr << "+ check comp super1 " << component1->selector()->inspect() << " vs " << complex2.back()->selector()->inspect() << "\n";

        auto rv = compoundIsSuperselector(
          component1->selector(),
          complex2.back()->selector(),
          parents);
        // stderr.write("Determined ${rv}\n");
        return rv;
      }


      // Find the first index [endOfSubselector] in [complex2] such that
// `complex2.sublist(i2, endOfSubselector + 1)` is a subselector of
// [component1.selector].
      auto endOfSubselector = i2;
      CplxSelComponentVector parents; // nullable?
      while (true && endOfSubselector < complex2.size()) {
        // std::cerr << "Get from complex2 size " << complex2.size() << " at " << endOfSubselector << "\n";
        auto component2 = complex2[endOfSubselector];
        if (component2->combinators().size() > 1) return false;

        // std::cerr << "+ check comp super2 " <<
        //  component1->selector()->inspect() << " vs " <<
        //  component2->selector()->inspect() << "\n";

        if (compoundIsSuperselector(component1->selector(), component2->selector(), parents)) {
          // std::cerr << "-- breakup\n";
          break;
        }

        endOfSubselector++;
        if (endOfSubselector == complex2.size() - 1) {
          // std::cerr << "End of subselector\n";
          // Stop before the superselector would encompass all of [complex2]
          // because we know [complex1] has more than one element, and consuming
          // all of [complex2] wouldn't leave anything for the rest of [complex1]
          // to match.
          return false;
        }

        // parents.clear();
        parents.push_back(component2);
      }


      if (!_compatibleWithPreviousCombinator(
        previousCombinator, parents)) {
        // std::cerr << ("Incompatible with previous\n");
        return false;
      }

      if (complex2.size() <= endOfSubselector) {
        // std::cerr << "End of subselector 2\n";
        break;
      }
      auto component2 = complex2[endOfSubselector];
      auto combinator1 = frontOrNull(component1->combinators());
      auto combinator2 = frontOrNull(component2->combinators());

      // std::cerr << "test " << ToString(combinator1.ptr()) << " vs " << ToString(combinator2.ptr()) << "\n";

      // stderr.write("test ${combinator1} vs ${combinator2}\n");
      if (!_isSupercombinator(combinator1, combinator2)) {
        // std::cerr << "Not a supercombinator1\n";
        return false;
      }


      i1++;
      i2 = endOfSubselector + 1;
      previousCombinator = combinator1;

      if (complex1.size() - i1 == 1) {
        if (combinator1 && combinator1->isFollowingSibling()) {
          // stderr.write("Has fallowing sibling\n");
          // The selector `.foo ~ .bar` is only a superselector of selectors that
          // *exclusively* contain subcombinators of `~`.
          // bool isEverySuper = true;
          for (size_t i3 = i2; i3 < complex2.size() - 1; i3++) {
            auto component = complex2[i3];
            if (!_isSupercombinator(combinator1, component->combinators().front())) {
              // std::cerr << "Not a supercombinator2\n";
              return false;
            }
          }
        }
        else if (combinator1 != nullptr) {
          //stderr.write("has other combinator\n");
          // `.foo > .bar` and `.foo + bar` aren't superselectors of any selectors
          // with more than one combinator.
          if (complex2.size() - i2 > 1) {
            // std::cerr << "End of subselector 3\n";
            return false;
          }
        }
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
    const CplxSelComponentVector& complex1,
    const CplxSelComponentVector& complex2)
  {
    // Try some simple heuristics to see if we can avoid allocations.
    //if (complex1.empty() && complex2.empty()) return false;
    ////if (complex1.front()->isaSelectorCombinator()) return false;
    ////if (complex2.front()->isaSelectorCombinator()) return false;
    //if (complex1.size() > complex2.size()) return false;
    //// TODO(nweiz): There's got to be a way to do this without a bunch of extra allocations...

    if (complex1.size() > complex2.size()) return false;
    //std::cerr << "complexIsParentSuper1 " << InspectVector(complex1) << "\n";
    //std::cerr << "complexIsParentSuper2 " << InspectVector(complex2) << "\n";

    CplxSelComponentVector cplx1(complex1);
    CplxSelComponentVector cplx2(complex2);
    PlaceholderSelectorObj phs = SASS_MEMORY_NEW(PlaceholderSelector,
      SourceSpan::internal("[BASE]"), "%<temp>");
    CplxSelComponentObj base = SASS_MEMORY_NEW(CplxSelComponent,
      SourceSpan::internal("[BASE]"), {}, phs->wrapInCompound() );
    //std::cerr << "BOGUS ==> " << base->inspecter() << "\n";
    cplx1.push_back(base);
    cplx2.push_back(base);
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
      // if (complexIsSuperselector(lhs->elements(), complex->elements())) {
      if (lhs->isSuperselectorOf(complex)) {
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

    return leadingCombinators_.empty() &&
      sub->leadingCombinators_.empty() &&
      complexIsSuperselector(elements(), sub->elements());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool SimpleSelector::isSuperselectorAF(SimpleSelector* other) const
  {
    // std::cerr << ("Simple::isSuper\n");

    if (PtrObjEqualityFn(this, other)) {
      // std::cerr << "Stuff is equal\n";
      return true;
    }

    if (auto pseudo = other->isaPseudoSelector()) {
      if (pseudo->isClass()) {
        // std::cerr << ("Test inner pseudo\n");
        auto list = pseudo->selector();
        if (list == nullptr) return false;

        if (isSubselectorPseudo(pseudo->normalized())) {
          for (auto complex : list->elements()) {
            if (complex->empty()) continue;
            for (auto simple : complex->last()->selector()->elements()) {
              if (!isSuperselectorAF(simple)) {
                return false;
              }
            }
          }
          return true;
        }
      }
    }
    return false;
  }

  bool TypeSelector::isSuperselectorAF(SimpleSelector* other) const
  {
    // std::cerr << "++ Type::isSuper " << this->inspect() << " vs " << other->inspect() << "\n";
    if (isUniversal()) {
      // std::cerr << "UNIVERSAL\n";
      return nsMatch(*other);
    }
    else {
      if (SimpleSelector::isSuperselectorAF(other)) {
        // std::cerr << (" res1 => true\n");
        return true;
      }
      if (auto type = other->isaTypeSelector()) {
        auto q = name_ == type->name_ && nsMatch(*type);
        // std::cerr << " res2 => " << q << "\n";
        return q;
      }
    }
    // std::cerr << (" res2 => false\n");
    return false;
  }

  bool PseudoSelector::isSuperselectorAF(SimpleSelector* other) const
  {
    // std::cerr << ("Pseudo::isSuper\n");
    auto selector = this->selector();
    if (selector == nullptr) return PtrObjEqualityFn((SimpleSelector*)this, other);
    if (auto pseudo = other->isaPseudoSelector()) {
      if (isElement() &&
        pseudo->isElement() &&
        normalized_ == "slotted" &&
        pseudo->name_ == name_)
      {
        if (pseudo->selector() == nullptr) return false;
        return selector->isSuperselectorOf(pseudo->selector());
      }
    }
    return false;
  }

  bool PseudoSelector::isSuperSelector(PseudoSelector* other) const
  {
    if (SimpleSelector::isSuperselectorAF(other)) return true;

    auto selector = this->selector();
    if (selector == nullptr) return this == other;
    if (other->isaPseudoSelector() &&
      isElement() &&
      other->isElement() &&
      normalized_ == "slotted" &&
      other->name_ == name_)
    {
      if (other->selector() == nullptr) return false;
      return selector->isSuperselectorOf(other->selector());
    }

    // Fall back to the logic defined in functions.dart, which knows how to
    // compare selector pseudoclasses against raw selectors.
    //return CompoundSelector([this], span)
    //  .isSuperselector(CompoundSelector([other], span));

    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
