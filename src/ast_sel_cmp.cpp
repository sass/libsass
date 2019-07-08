/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* This file contains all ast operator functions in one compile unit.        */
/*****************************************************************************/
#include "ast_selectors.hpp"
#include "ast_statements.hpp"
#include "callstack.hpp"

namespace Sass {

  // We want to compare selector lists position independent, so we use a Set.
  // This means we either need to implement a less compare method or a hashing
  // function. Given that we might compare selectors quite often the hashing
  // approach has proven to be slightly faster. It has some memory overhead,
  // but trades off nicely for better runtime performance.
  bool SelectorList::operator== (const SelectorList& rhs) const
  {
    if (&rhs == this) return true;
    if (rhs.size() != size()) return false;
    std::unordered_set<const ComplexSelector*, PtrObjHash, PtrObjEquality> lhs_set;
    lhs_set.reserve(size());
    for (const ComplexSelectorObj& element : elements()) {
      lhs_set.insert(element.ptr());
    }
    for (const ComplexSelectorObj& element : rhs.elements()) {
      if (lhs_set.count(element.ptr()) == 0) return false;
    }
    return true;
  }

  bool ComplexSelector::operator== (const ComplexSelector& rhs) const
  {
    size_t len = size();
    size_t rlen = rhs.size();
    if (len != rlen) return false;
    for (size_t i = 0; i < len; i += 1) {
      if (!(*get(i) == *rhs.get(i))) return false;
    }
    return true;
  }

  bool SelectorCombinator::operator==(const SelectorCombinator& rhs) const
  {
    return combinator() == rhs.combinator();
  }

  bool CompoundSelector::operator== (const CompoundSelector& rhs) const
  {
    if (&rhs == this) return true;
    if (rhs.size() != size()) return false;
    std::unordered_set<const SimpleSelector*, PtrObjHash, PtrObjEquality> lhs_set;
    lhs_set.reserve(size());
    for (const SimpleSelectorObj& element : elements()) {
      lhs_set.insert(element.ptr());
    }
    // there is no break?!
    for (const SimpleSelectorObj& element : rhs.elements()) {
      if (lhs_set.find(element.ptr()) == lhs_set.end()) return false;
    }
    return true;
  }

  bool IDSelector::operator== (const IDSelector& rhs) const
  {
    // ID has no namespace
    return name() == rhs.name();
  }

  bool TypeSelector::operator== (const TypeSelector& rhs) const
  {
    return nsMatch(rhs) && name() == rhs.name();
  }

  bool ClassSelector::operator== (const ClassSelector& rhs) const
  {
    // Class has no namespace
    return name() == rhs.name();
  }

  bool PlaceholderSelector::operator== (const PlaceholderSelector& rhs) const
  {
    // Placeholder has no namespace
    return name() == rhs.name();
  }

  bool AttributeSelector::operator== (const AttributeSelector& rhs) const
  {
    // smaller return, equal go on, bigger abort
    return nsMatch(rhs)
      && op() == rhs.op()
      && name() == rhs.name()
      && value() == rhs.value()
      && modifier() == rhs.modifier();
  }

  bool PseudoSelector::operator== (const PseudoSelector& rhs) const
  {
    return nsMatch(rhs)
      && name() == rhs.name()
      && argument() == rhs.argument()
      && isPseudoElement() == rhs.isPseudoElement()
      && ObjEquality()(selector(), rhs.selector());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
