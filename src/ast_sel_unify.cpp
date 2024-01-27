/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* This file contains all ast unify functions in one compile unit.           */
/*****************************************************************************/
#include "ast_selectors.hpp"

#include "debugger.hpp"

namespace Sass {


  template <typename T>
  sass::string VecToString2(sass::vector<T> exts) {
    sass::string msg = "[";
    bool joiner = false;
    for (auto& entry : exts) {
      if (joiner) msg += ", ";
      msg += entry->inspect();
      joiner = true;
    }
    return msg + "]";

  }
  /////////////////////////////////////////////////////////////////////////
  // Returns the contents of a [SelectorList] that matches only 
  // elements that are matched by both [complex1] and [complex2].
  // If no such list can be produced, returns `null`.
  /////////////////////////////////////////////////////////////////////////
  // ToDo: fine-tune API to avoid unnecessary wrapper allocations
  /////////////////////////////////////////////////////////////////////////
  sass::vector<ComplexSelectorObj> _unifyComplex(
    sass::vector<ComplexSelectorObj> complexes,
    const SourceSpan& pstate)
  {

    for (auto qwe : complexes) {
      //std::cerr << "- IN " << qwe->inspect() << "\n";
    }

    sass::vector<SimpleSelectorObj> unifiedBase;
    SelectorCombinatorObj leadingCombinator;
    SelectorCombinatorObj trailingCombinator;

    for (const auto& complex : complexes) {
      if (complex->isUseless()) return {};
      if (complex->elements().size() == 1) {
        if (complex->hasOneLeadingCombinators()) {
          const SelectorCombinatorObj& lead
            = complex->getLeadingCombinator();
          if (leadingCombinator.isNull()) {
            //std::cerr << "===== has leading " << lead->toString() << "\n";
            leadingCombinator = lead;
          }
          else if (!ObjEqualityFn(leadingCombinator, lead)) {
          // else if (leadingCombinator != lead) {
            return {}; // Return empty list
          }
        }
      }
      else {
        // std::cerr << "Edge case detected, check\n";
      }

      if (complex->size() == 0) continue;
      // Get last compound of current complex selector
      // This is the one that will connect to next lead
      auto base = complex->last();

      //std::cerr << " base [" << base->inspect() << "]\n";

      if (base->combinators().size() == 1) {
        const auto& trail = base->combinators().back();
        if (trailingCombinator != nullptr) {
          if (!ObjEqualityFn(trailingCombinator, trail)) {
            return {}; // Return empty list
          }
        }
        trailingCombinator = trail;
      }

      /*if (trailingCombinator)
        std::cerr << " trail [" << trailingCombinator->toString() << "]\n";
      else         std::cerr << " trail [N/A]\n";*/


      if (unifiedBase.empty()) {
        unifiedBase = base->selector()->elements();
      }
      else {
        for (auto simple : base->selector()->elements()) {
          //std::cerr << "Unify lhs : " << simple->inspect() << "\n";
          //std::cerr << "Unify rhs : " << unifiedBase[0]->inspect() << "\n";
          unifiedBase = simple->unify(unifiedBase);
          if (unifiedBase.empty()) return {};
        }

      }
    }

    // unifiedBase is nullptr, abort?

    for (auto qwe : unifiedBase) {
      // std::cerr << "- base " << qwe->inspect() << "\n";
    }

    for (auto qwe : complexes) {
      // std::cerr << "- CPLX " << qwe->inspect() << "\n";
    }

    sass::vector<ComplexSelectorObj> withoutBases;
    for (size_t i = 0; i < complexes.size(); i += 1) {
      if (complexes[i]->size() < 2) continue;
      ComplexSelector* unbase = SASS_MEMORY_NEW
        (ComplexSelector, complexes[i].ptr()); // copy
      unbase->elements().pop_back(); // remove last
      withoutBases.push_back(unbase); // add unbase
    }

    //std::cerr << "- NOBASE " << VecToString2(withoutBases) << "\n";

    CompoundSelector* compound = SASS_MEMORY_NEW(
      CompoundSelector, pstate, std::move(unifiedBase));


    sass::vector<SelectorCombinatorObj> trailing;
    if (trailingCombinator != nullptr)
      trailing.push_back(trailingCombinator);
    CplxSelComponent* component = SASS_MEMORY_NEW(
      CplxSelComponent, pstate, std::move(trailing), compound);
    ComplexSelector* base;
    if (!leadingCombinator) base = SASS_MEMORY_NEW(ComplexSelector, pstate, {}, { component });
    else base = SASS_MEMORY_NEW(ComplexSelector, pstate, { leadingCombinator }, { component });

    sass::vector<ComplexSelectorObj> weaving;


    if (withoutBases.empty()) {
      weaving.push_back(base);
    }
    else {
      weaving.insert(weaving.end(),
        withoutBases.begin(),
        withoutBases.end() - 1);
      weaving.push_back(withoutBases.back()->concatenate(base, pstate, false));
    }
    // lineBreak: complexes.any((complex) => complex.lineBreak));

    for (auto qwe : weaving) {
      //std::cerr << "- WEAVE " << qwe->inspect() << "\n";
    }

    auto rv = weave(weaving, false); // TODO

    for (auto qwe : rv) {
      //std::cerr << "+ WEAVED " << qwe->inspect() << "\n";
    }

    return rv;

  }
  // EO unifyComplex

  /////////////////////////////////////////////////////////////////////////
  // Returns a [CompoundSelector] that matches only elements
  // that are matched by both [compound1] and [compound2].
  // If no such selector can be produced, returns `null`.
  /////////////////////////////////////////////////////////////////////////
//  CompoundSelector* CompoundSelector::unifyWith(sass::vector<CompoundSelectorObj> rhs)
//  {
//    if (empty()) return rhs;
//    CompoundSelectorObj unified = SASS_MEMORY_COPY(rhs);
//    for (const SimpleSelectorObj& sel : elements()) {
//      unified = sel->unifyWith(unified);
//      if (unified.isNull()) break;
//    }
//    return unified.detach();
//  }
  // EO CompoundSelector::unifyWith(CompoundSelector*)

  /////////////////////////////////////////////////////////////////////////
  // Returns the components of a [CompoundSelector] that matches only elements
  // matched by both this and [compound]. By default, this just returns a copy
  // of [compound] with this selector added to the end, or returns the original
  // array if this selector already exists in it. Returns `null` if unification
  // is impossible—for example, if there are multiple ID selectors.
  /////////////////////////////////////////////////////////////////////////
  // This is implemented in `selector/simple.dart` as `SimpleSelector::unify`
  /////////////////////////////////////////////////////////////////////////
  sass::vector<SimpleSelectorObj> SimpleSelector::unify(
    const sass::vector<SimpleSelectorObj>& others)
  {

    if (name_ == "host" || name_ == "host-context") {
      for (const auto& simple : others) { // every
        auto pseudo = simple->isaPseudoSelector();
        if (pseudo == nullptr) return {}; // abort
        if (pseudo->isHost()) continue;
        if (pseudo->selector()) continue;
        return {}; // abort
      }
    }

    // Unify the simple case
    else if (others.size() == 1) {
      if (others[0]->isUniversal()) {
        return others[0]->unify({ this });
      }
      else if (const auto* pseudo = others[0]->isaPseudoSelector()) {
        if (pseudo->isHost() || pseudo->isHostContext())
          return others[0]->unify({ this });
      }
    }
    // Check if we are already part of other compound selectors
    for (auto& qwe : others) {
      if (PtrObjEqualityFn(qwe.ptr(),
        (SimpleSelector*)this))
        return others;
    }

    //if (std::find(others.begin(), others.end(), this) != others.end()) {
    //  return others; // simply return what we already have
    //}

    sass::vector<SimpleSelectorObj> results;
    // results.reserve(rhs->size() + 1);
    bool addedThis = false;
    for (auto simple : others) {
      // Make sure pseudo selectors always come last.
      if (!addedThis && simple->isaPseudoSelector()) {
        results.push_back(this);
        addedThis = true;
      }
      results.push_back(simple);
    }
    if (!addedThis) {
      results.push_back(this);
    }
    return results;
  }
  // EO SimpleSelector::unifyWith(CompoundSelector*)

  /////////////////////////////////////////////////////////////////////////
  // This is implemented in `selector/id.dart` as `PseudoSelector::unify`
  /////////////////////////////////////////////////////////////////////////
  sass::vector<SimpleSelectorObj> IDSelector::unify(
    const sass::vector<SimpleSelectorObj>& rhs)
  {
    for (const SimpleSelector* sel : rhs) {
      if (const IDSelector* id_sel = sel->isaIDSelector()) {
        if (id_sel->name() != name()) return {};
      }
    }
    // Dispatch to base implementation
    return SimpleSelector::unify(rhs);
  }



  /////////////////////////////////////////////////////////////////////////
  // This is implemented in `selector/pseudo.dart` as `PseudoSelector::unify`
  /////////////////////////////////////////////////////////////////////////
  sass::vector<SimpleSelectorObj> PseudoSelector::unify(
    const sass::vector<SimpleSelectorObj>& compound)
  {
    if (name_ == "host" || name_ == "host-context") {
      for (const auto& simple : compound) { // every
        auto pseudo = simple->isaPseudoSelector();
        if (pseudo == nullptr) return {}; // abort
        if (pseudo->isHost()) continue;
        if (pseudo->selector()) continue;
        return {}; // abort
      }
    }
    else if (compound.size() == 1) {
      if (compound[0]->isUniversal()) {
        return compound[0]->unify({ this });
      }
      else if (const auto pseudo = compound[0]->isaPseudoSelector()) {
        if (pseudo->isHost() || pseudo->isHostContext())
          return compound[0]->unify({ this });
      }
    }

    // std::cerr << "CHECK " << this->inspect() << "\n";
    // std::cerr << " VS " << compound[0]->inspect() << "\n";


    // Check if we are already part of other compound selectors
    for (auto& qwe : compound) {
      if (PtrObjEqualityFn(qwe.ptr(),
        (SimpleSelector*)this))
          return compound;
    }
    // if (std::any_of(compound.begin(), compound.end(), this, PtrObjEqualityFn) != compound.end()) {
    //   return compound; // simply return what we already have
    // }

    sass::vector<SimpleSelectorObj> results;
    // results.reserve(rhs->size() + 1);
    bool addedThis = false;
    for (auto simple : compound) {
      if (auto pseudo = simple->isaPseudoSelector()) {
        if (pseudo->isPseudoElement()) {
          if (isPseudoElement()) return {};
          results.push_back(this);
          addedThis = true;
        }
      }
      results.push_back(simple);
    }
    if (!addedThis) {
      results.push_back(this);
    }
    return results;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SelectorNS* SelectorNS::unity(SelectorNS* rhs)
  {
    sass::string ns;
    const sass::string& namespace1(this->ns());
    const sass::string& namespace2(rhs->ns());
    if (nsEqual(*rhs) || rhs->isUniversalNs()) {
      ns = namespace1;
    }
    else if (isUniversalNs()) {
      ns = namespace2;
    }
    else {
      return nullptr;
    }

    sass::string name;
    const sass::string& name1(this->name());
    const sass::string& name2(rhs->name());
    if (name_ == rhs->name() || rhs->isUniversal()) {
      name = name1;
    }
    else if (name1.empty() || isUniversal()) {
      name = name2;
    }
    else {
      return nullptr;
    }

    auto qwe = SASS_MEMORY_NEW(
      TypeSelector, pstate(),
      sass::string(name), std::move(ns),
      hasNs() && rhs->hasNs()); // Fixup
    //std::cerr << "UnifyTypeAndEl " << qwe->inspect() << "\n";
    return qwe;
  }


  /////////////////////////////////////////////////////////////////////////
  // This is implemented in `extend/functions.dart` as `unifyUniversalAndElement`
  // Returns a [SimpleSelector] that matches only elements that are matched by
  // both [lhs] and [rhs], which must both be either [UniversalSelector]s
  // or [TypeSelector]s. If no such selector can be produced, returns `null`.
  // Note: libsass handles universal selector directly within the type selector
  /////////////////////////////////////////////////////////////////////////
  //SimpleSelector* TypeSelector::unifyWith(const SimpleSelector* rhs2)
  //{
  //  if (auto rhs = rhs2->isaNameSpaceSelector()) {
  //    bool rhs_ns = false;
  //    if (!(nsEqual(*rhs) || rhs->isUniversalNs())) {
  //      if (!isUniversalNs()) {
  //        return nullptr;
  //      }
  //      rhs_ns = true;
  //    }
  //    bool rhs_name = false;
  //    if (!(name_ == rhs->name() || rhs->isUniversal())) {
  //      if (!(isUniversal())) {
  //        return nullptr;
  //      }
  //      rhs_name = true;
  //    }
  //    if (rhs_ns) {
  //      ns(rhs->ns());
  //      hasNs(rhs->hasNs());
  //    }
  //    if (rhs_name) name(rhs->name());
  //  }
  //  return this;
  //}
  // EO TypeSelector::unifyWith(const SimpleSelector*)

  sass::vector<SimpleSelectorObj> TypeSelector::unifyUniversal(
    const sass::vector<SimpleSelectorObj>& compound)
  {
    if (compound.size() == 0) {
      return { this };
    }
    else if (compound.size() > 0) {
      if (auto type = compound[0]->isaTypeSelector()) {
        auto unified = SelectorNS::unity(type);
        if (unified == nullptr) return {};
        sass::vector<SimpleSelectorObj> rv;
        rv.push_back(unified);
        rv.insert(rv.end(),
          compound.begin() + 1,
          compound.end());
        return rv;
      }

      else if (auto pseudo = compound[0]->isaPseudoSelector()) {
        if (pseudo->isHost() || pseudo->isHostContext()) return {};
      }
      else {
        // std::cerr << "hasNs: " << hasNs_ << ", ns " << ns_ << "\n";
        if (!hasNs_ || ns_ == "*") {
          return compound;
        }
        else {
          sass::vector<SimpleSelectorObj> rv;
          rv.push_back(this);
          rv.insert(rv.end(),
            compound.begin(),
            compound.end());
          return rv;
        }
      }
    }
    return compound;
  }

  /////////////////////////////////////////////////////////////////////////
  // This is implemented in `selector/type.dart` as `PseudoSelector::unify`
  /////////////////////////////////////////////////////////////////////////
  sass::vector<SimpleSelectorObj> TypeSelector::unify(
    const sass::vector<SimpleSelectorObj>& compound)
  {

    if (compound.empty()) return {};
    if (isUniversal()) {
      return unifyUniversal(compound);
    }
    auto first = compound.front();
    if (first->isUniversal()) {
      return first->unify({ this });
    }
    else if (const auto& type = first->isaTypeSelector()) {
      auto unified = SelectorNS::unity(type);
      if (unified == nullptr) return {}; // abort here
      if (unified->empty()) return {}; // abort here
      sass::vector<SimpleSelectorObj> result;
      result.push_back(unified); // prepend result
      result.insert(result.end(),
        compound.begin() + 1,
        compound.end());
      return result;
    }
    else {
      sass::vector<SimpleSelectorObj> result;
      result.push_back(this); // prepend myself
      result.insert(result.end(),
        compound.begin(),
        compound.end());
      return result;
    }
  }


  /////////////////////////////////////////////////////////////////////////
  // Unify two complex selectors. Internally calls `unifyComplex`
  // and then wraps the result in newly create ComplexSelectors.
  /////////////////////////////////////////////////////////////////////////
  SelectorList* ComplexSelector::unifyList(ComplexSelector* rhs)
  {
    sass::vector<ComplexSelectorObj> rv =
       _unifyComplex({ this, rhs }, pstate());
    sass::vector<ComplexSelectorObj> list;
  ////  list.reserve(rv.size());
    if (rv.empty()) return nullptr;
    for (ComplexSelectorObj& items : rv) {
      list.push_back(items);
    }
    SelectorListObj qwe = SASS_MEMORY_NEW(SelectorList,
      pstate(), std::move(list));
    // debug_ast(qwe);
    return qwe.detach();
  }
  // EO ComplexSelector::unifyWith(ComplexSelector*)

  /////////////////////////////////////////////////////////////////////////
  // only called from the sass function `selector-unify`
  /////////////////////////////////////////////////////////////////////////
  SelectorList* SelectorList::unifyWith(SelectorList* rhs)
  {
    sass::vector<ComplexSelectorObj> selectors;
    // Unify all of children with RHS's children,
    // storing the results in `unified_complex_selectors`
    for (const ComplexSelectorObj& seq1 : elements()) {
      for (const ComplexSelectorObj& seq2 : rhs->elements()) {
        if (SelectorList* unified = seq1->unifyList(seq2)) {
          selectors.insert(selectors.end(),
            unified->begin(), unified->end());
        }
      }
    }
    return SASS_MEMORY_NEW(SelectorList,
      pstate(), std::move(selectors));
  }
  // EO SelectorList::unifyWith(SelectorList*)

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
