// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include "ast_selectors.hpp"

namespace Sass {

  /*#########################################################################*/
  // Compare against base class on right hand side
  // try to find the most specialized implementation
  /*#########################################################################*/

  // Selector lists can be compared to comma lists
  bool SelectorList::operator== (const Expression& rhs) const
  {
    if (auto l = Cast<List>(&rhs)) { return *this == *l; }
    if (auto s = Cast<Selector>(&rhs)) { return *this == *s; }
    if (Cast<String>(&rhs) || Cast<Null>(&rhs)) { return false; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  // Selector lists can be compared to comma lists
  bool SelectorList::operator< (const Expression& rhs) const
  {
    if (auto l = Cast<List>(&rhs)) { return *this < *l; }
    if (auto s = Cast<Selector>(&rhs)) { return *this < *s; }
    if (Cast<String>(&rhs) || Cast<Null>(&rhs)) { return true; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool SelectorList::operator== (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this == *sel; }
    if (auto list = Cast<List>(&rhs)) { return *this == *list; }
    throw std::runtime_error("invalid selector base classes to compare");
  }
  bool SelectorList::operator< (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this < *sel; }
    if (auto list = Cast<List>(&rhs)) { return *this < *list; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool ComplexSelector::operator== (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *sel == *this; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this == *sel; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool ComplexSelector::operator< (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this < *sel; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool SelectorCombinator::operator== (const Selector& rhs) const
  {
    if (auto cpx = Cast<SelectorCombinator>(&rhs)) { return *this == *cpx; }
    return false;
  }
  bool SelectorCombinator::operator< (const Selector& rhs) const
  {
    if (auto cpx = Cast<SelectorCombinator>(&rhs)) { return *this < *cpx; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool CompoundSelector::operator== (const Selector& rhs) const
  {
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this == *sel; }
    throw std::runtime_error("invalid selector base classes to compare");
  }
  bool CompoundSelector::operator< (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this < *sel; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool SimpleSelector::operator== (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this == *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) return *this == *sel;
    throw std::runtime_error("invalid selector base classes to compare");
  }

  bool SimpleSelector::operator< (const Selector& rhs) const
  {
    if (auto sel = Cast<SelectorList>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<ComplexSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<CompoundSelector>(&rhs)) { return *this < *sel; }
    if (auto sel = Cast<SimpleSelector>(&rhs)) { return *this < *sel; }
    throw std::runtime_error("invalid selector base classes to compare");
  }

  /*#########################################################################*/
  /*#########################################################################*/

  bool SelectorList::operator< (const SelectorList& rhs) const
  {
    size_t l = rhs.length();
    if (length() < l) l = length();
    for (size_t i = 0; i < l; i++) {
      if (*at(i) < *rhs.at(i)) return true;
      if (*at(i) != *rhs.at(i)) return false;
    }
    return false;
  }

  bool SelectorList::operator== (const SelectorList& rhs) const
  {
    if (&rhs == this) return true;
    if (rhs.length() != length()) return false;
    std::unordered_set<const ComplexSelector*, PtrObjHash, PtrObjEquality> lhs_set;
    lhs_set.reserve(length());
    for (const ComplexSelectorObj& element : elements()) {
      lhs_set.insert(element.ptr());
    }
    for (const ComplexSelectorObj& element : rhs.elements()) {
      if (lhs_set.find(element.ptr()) == lhs_set.end()) return false;
    }
    return true;
  }



  /*#########################################################################*/
  // Compare SelectorList against all other selector types
  /*#########################################################################*/

  bool SelectorList::operator== (const ComplexSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (length() != 1) return false;
    // Compare simple selectors
    return *get(0) == rhs;
  }

  bool SelectorList::operator< (const ComplexSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = length();
    if (rlen > 1) return false;
    if (rlen == 0) return true;
    // Compare the actual nodes
    return *get(0) < rhs;
  }

  bool SelectorList::operator== (const CompoundSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (length() != 1) return false;
    // Compare simple selectors
    return *get(0) == rhs;
  }

  bool SelectorList::operator< (const CompoundSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = length();
    if (rlen > 1) return false;
    if (rlen == 0) return true;
    // Compare the actual nodes
    return *get(0) < rhs;
  }

  bool SelectorList::operator== (const SimpleSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (length() != 1) return false;
    // Compare simple selectors
    return *get(0) == rhs;
  }

  bool SelectorList::operator< (const SimpleSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = length();
    if (rlen > 1) return false;
    if (rlen == 0) return true;
    // Compare the actual nodes
    return *get(0) < rhs;
  }
  
  /*#########################################################################*/
  // Compare ComplexSelector against itself
  /*#########################################################################*/

  bool ComplexSelector::operator== (const ComplexSelector& rhs) const
  {
    size_t len = length();
    size_t rlen = rhs.length();
    if (len != rlen) return false;
    for (size_t i = 0; i < len; i += 1) {
      if (*get(i) != *rhs.get(i)) return false;
    }
    return true;
  }

  bool ComplexSelector::operator< (const ComplexSelector& rhs) const
  {
    size_t len = length();
    size_t rlen = rhs.length();
    if (len < rlen) return true;
    if (len > rlen) return false;
    for (size_t i = 0; i < len; i += 1) {
      // ToDo: implement cmp for one call
      if (*get(i) < *rhs.get(i)) return true;
      if (*get(i) != *rhs.get(i)) return false;
    }
    return false;
  }

  /*#########################################################################*/
  // Compare ComplexSelector against all other selector types
  /*#########################################################################*/

  bool ComplexSelector::operator== (const SelectorList& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (rhs.length() != 1) return false;
    // Compare complex selector
    return *this == *rhs.get(0);
  }

  bool ComplexSelector::operator< (const SelectorList& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t len = rhs.length();
    if (len > 1) return true;
    if (len == 0) return false;
    // Compare complex selector
    return *this < *rhs.get(0);
  }

  bool ComplexSelector::operator== (const CompoundSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (length() != 1) return false;
    // Compare compound selector
    return *get(0) == rhs;
  }

  bool ComplexSelector::operator< (const CompoundSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = length();
    if (rlen > 1) return false;
    if (rlen == 0) return true;
    // Compare compound selector
    return *get(0) < rhs;
  }

  bool ComplexSelector::operator== (const SimpleSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (length() != 1) return false;
    // Compare simple selectors
    return *get(0) == rhs;
  }

  bool ComplexSelector::operator< (const SimpleSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = length();
    if (rlen > 1) return false;
    if (rlen == 0) return true;
    // Compare simple selectors
    return *get(0) < rhs;
  }

  /*#########################################################################*/
  // Compare SelectorCombinator against itself
  /*#########################################################################*/

  bool SelectorCombinator::operator==(const SelectorCombinator& rhs) const
  {
    return combinator() == rhs.combinator();
  }

  bool SelectorCombinator::operator<(const SelectorCombinator& rhs) const
  {
    return combinator() < rhs.combinator();
  }

  /*#########################################################################*/
  // Compare SelectorCombinator against SelectorComponent
  /*#########################################################################*/

  bool SelectorCombinator::operator==(const SelectorComponent& rhs) const
  {
    if (const SelectorCombinator * sel = rhs.getCombinator()) {
      return *this == *sel;
    }
    return false;
  }

  bool SelectorCombinator::operator<(const SelectorComponent& rhs) const
  {
    if (const SelectorCombinator * sel = rhs.getCombinator()) {
      return *this < *sel;
    }
    return true;
  }

  bool CompoundSelector::operator==(const SelectorComponent& rhs) const
  {
    if (const CompoundSelector * sel = rhs.getCompound()) {
      return *this == *sel;
    }
    return false;
  }

  bool CompoundSelector::operator<(const SelectorComponent& rhs) const
  {
    if (const CompoundSelector * sel = rhs.getCompound()) {
      return *this < *sel;
    }
    return false;
  }

  /*#########################################################################*/
  // Compare CompoundSelector against itself
  /*#########################################################################*/
  // ToDo: Verifiy implementation
  /*#########################################################################*/

  bool CompoundSelector::operator< (const CompoundSelector& rhs) const
  {
    size_t L = std::min(length(), rhs.length());
    for (size_t i = 0; i < L; ++i)
    {
      SimpleSelector* l = (*this)[i];
      SimpleSelector* r = rhs[i];
      if (!l && !r) return false;
      else if (!r) return false;
      else if (!l) return true;
      else if (*l != *r)
      {
        return *l < *r;
      }
    }
    // just compare the length now
    return length() < rhs.length();
  }

  bool CompoundSelector::operator== (const CompoundSelector& rhs) const
  {
    // std::cerr << "comp vs comp\n";
    if (&rhs == this) return true;
    if (rhs.length() != length()) return false;
    std::unordered_set<const SimpleSelector*, PtrObjHash, PtrObjEquality> lhs_set;
    lhs_set.reserve(length());
    for (const SimpleSelectorObj& element : elements()) {
      lhs_set.insert(element.ptr());
    }
    // there is no break?!
    for (const SimpleSelectorObj& element : rhs.elements()) {
      if (lhs_set.find(element.ptr()) == lhs_set.end()) return false;
    }
    return true;
  }


  /*#########################################################################*/
  // Compare CompoundSelector against all other selector types
  /*#########################################################################*/

  bool CompoundSelector::operator== (const SelectorList& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (rhs.length() != 1) return false;
    // Compare complex selector
    return *this == *rhs.get(0);
  }

  bool CompoundSelector::operator< (const SelectorList& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t len = rhs.length();
    if (len > 1) return true;
    if (len == 0) return false;
    // Compare complex selector
    return *this < *rhs.get(0);
  }

  bool CompoundSelector::operator== (const ComplexSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (rhs.length() != 1) return false;
    // Compare compound selector
    return *this == *rhs.get(0);
  }

  bool CompoundSelector::operator< (const ComplexSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t len = rhs.length();
    if (len > 1) return true;
    if (len == 0) return false;
    // Compare compound selector
    return *this < *rhs.get(0);
  }

  bool CompoundSelector::operator< (const SimpleSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (length() != 1) return false;
    // Compare simple selectors
    return *get(0) == rhs;
  }

  bool CompoundSelector::operator== (const SimpleSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = length();
    if (rlen > 1) return false;
    if (rlen == 0) return true;
    // Compare simple selectors
    return *get(0) < rhs;
  }

  /*#########################################################################*/
  // Compare SimpleSelector against itself (upcast from abstract base)
  /*#########################################################################*/

  // DOES NOT EXIST FOR ABSTRACT BASE CLASS

  /*#########################################################################*/
  // Compare SimpleSelector against all other selector types
  /*#########################################################################*/

  bool SimpleSelector::operator== (const SelectorList& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (rhs.length() != 1) return false;
    // Compare complex selector
    return *this == *rhs.get(0);
  }

  bool SimpleSelector::operator< (const SelectorList& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = rhs.length();
    if (rlen > 1) return true;
    if (rlen == 0) return false;
    // Compare complex selector
    return *this < *rhs.get(0);
  }

  bool SimpleSelector::operator== (const ComplexSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return true;
    // Must have exactly one item
    if (rhs.length() != 1) return false;
    // Compare compound selector
    return *this == *rhs.get(0);
  }

  bool SimpleSelector::operator< (const ComplexSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = rhs.length();
    if (rlen > 1) return true;
    if (rlen == 0) return false;
    // Compare compound selector
    return *this < *rhs.get(0);
  }

  bool SimpleSelector::operator== (const CompoundSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    if (rhs.length() != 1) return false;
    // Compare simple selector
    return *this == *rhs.get(0);
  }

  bool SimpleSelector::operator< (const CompoundSelector& rhs) const
  {
    // If both are empty they are equal
    if (empty() && rhs.empty()) return false;
    // Must have exactly one item
    size_t rlen = rhs.length();
    if (rlen > 1) return true;
    if (rlen == 0) return false;
    // Compare simple selector
    return *this < *rhs.get(0);
  }


  /*#########################################################################*/
  /*#########################################################################*/

  bool Id_Selector::operator== (const SimpleSelector& rhs) const
  {
    auto sel = Cast<Id_Selector>(&rhs);
    return sel ? *this == *sel : false;
  }

  bool Type_Selector::operator== (const SimpleSelector& rhs) const
  {
    auto sel = Cast<Type_Selector>(&rhs);
    return sel ? *this == *sel : false;
  }

  bool Class_Selector::operator== (const SimpleSelector& rhs) const
  {
    auto sel = Cast<Class_Selector>(&rhs);
    return sel ? *this == *sel : false;
  }

  bool Pseudo_Selector::operator== (const SimpleSelector& rhs) const
  {
    auto sel = Cast<Pseudo_Selector>(&rhs);
    return sel ? *this == *sel : false;
  }

  bool Attribute_Selector::operator== (const SimpleSelector& rhs) const
  {
    auto sel = Cast<Attribute_Selector>(&rhs);
    return sel ? *this == *sel : false;
  }

  bool Placeholder_Selector::operator== (const SimpleSelector& rhs) const
  {
    auto sel = Cast<Placeholder_Selector>(&rhs);
    return sel ? *this == *sel : false;
  }

  /*#########################################################################*/
  /*#########################################################################*/

  bool SimpleSelector::operator< (const SimpleSelector& rhs) const
  {
    std::cerr << "Nopes\n";
    switch (simple_type()) {
      case ID_SEL: return (const Id_Selector&) *this < rhs; break;
      case TYPE_SEL: return (const Type_Selector&) *this < rhs; break;
      case CLASS_SEL: return (const Class_Selector&) *this < rhs; break;
      case PSEUDO_SEL: return (const Pseudo_Selector&) *this < rhs; break;
      case ATTRIBUTE_SEL: return (const Attribute_Selector&) *this < rhs; break;
      case PLACEHOLDER_SEL: return (const Placeholder_Selector&) *this < rhs; break;
    }
    return false;
  }

  bool Id_Selector::operator< (const SimpleSelector& rhs) const
  {
    std::cerr << "Nopes\n";
    switch (rhs.simple_type()) {
      case TYPE_SEL: return '#' < 's'; break;
      case CLASS_SEL: return '#' < '.'; break;
      case PSEUDO_SEL: return '#' < ':'; break;
      case ATTRIBUTE_SEL: return '#' < '['; break;
      case PLACEHOLDER_SEL: return '#' < '%'; break;
      case ID_SEL: /* let if fall through */ break;
    }
    const Id_Selector& sel =
      (const Id_Selector&) rhs;
    return *this < sel;
  }

  bool Type_Selector::operator< (const SimpleSelector& rhs) const
  {
    switch (rhs.simple_type()) {
      case ID_SEL: return 'e' < '#'; break;
      case CLASS_SEL: return 'e' < '.'; break;
      case PSEUDO_SEL: return 'e' < ':'; break;
      case ATTRIBUTE_SEL: return 'e' < '['; break;
      case PLACEHOLDER_SEL: return 'e' < '%'; break;
      case TYPE_SEL: /* let if fall through */ break;
    }
    const Type_Selector& sel =
      (const Type_Selector&) rhs;
    return *this < sel;
  }

  bool Class_Selector::operator< (const SimpleSelector& rhs) const
  {
    switch (rhs.simple_type()) {
      case ID_SEL: return '.' < '#'; break;
      case TYPE_SEL: return '.' < 's'; break;
      case PSEUDO_SEL: return '.' < ':'; break;
      case ATTRIBUTE_SEL: return '.' < '['; break;
      case PLACEHOLDER_SEL: return '.' < '%'; break;
      case CLASS_SEL: /* let if fall through */ break;
    }
    const Class_Selector& sel =
      (const Class_Selector&) rhs;
    return *this < sel;
  }

  bool Pseudo_Selector::operator< (const SimpleSelector& rhs) const
  {
    switch (rhs.simple_type()) {
      case ID_SEL: return ':' < '#'; break;
      case TYPE_SEL: return ':' < 's'; break;
      case CLASS_SEL: return ':' < '.'; break;
      case ATTRIBUTE_SEL: return ':' < '['; break;
      case PLACEHOLDER_SEL: return ':' < '%'; break;
      case PSEUDO_SEL: /* let if fall through */ break;
    }
    const Pseudo_Selector& sel =
      (const Pseudo_Selector&) rhs;
    return *this < sel;
  }

  bool Attribute_Selector::operator< (const SimpleSelector& rhs) const
  {
    switch (rhs.simple_type()) {
      case ID_SEL: return '[' < '#'; break;
      case TYPE_SEL: return '[' < 'e'; break;
      case CLASS_SEL: return '[' < '.'; break;
      case PSEUDO_SEL: return '[' < ':'; break;
      case PLACEHOLDER_SEL: return '[' < '%'; break;
      case ATTRIBUTE_SEL: /* let if fall through */ break;
    }
    const Attribute_Selector& sel =
      (const Attribute_Selector&) rhs;
    return *this < sel;
  }

  bool Placeholder_Selector::operator< (const SimpleSelector& rhs) const
  {
    switch (rhs.simple_type()) {
      case ID_SEL: return '%' < '#'; break;
      case TYPE_SEL: return '%' < 's'; break;
      case CLASS_SEL: return '%' < '.'; break;
      case PSEUDO_SEL: return '%' < ':'; break;
      case ATTRIBUTE_SEL: return '%' < '['; break;
      case PLACEHOLDER_SEL: /* let if fall through */ break;
    }
    const Placeholder_Selector& sel =
      (const Placeholder_Selector&) rhs;
    return *this < sel;
  }

  /*#########################################################################*/
  /*#########################################################################*/

  bool Id_Selector::operator== (const Id_Selector& rhs) const
  {
    // ID has no namespacing
    return name() == rhs.name();
  }

  bool Id_Selector::operator< (const Id_Selector& rhs) const
  {
    // ID has no namespacing
    return name() < rhs.name();
  }

  bool Type_Selector::operator== (const Type_Selector& rhs) const
  {
    return is_ns_eq(rhs) && name() == rhs.name();
  }

  bool Type_Selector::operator< (const Type_Selector& rhs) const
  {
    return has_ns_ == rhs.has_ns_
      ? (ns_ == rhs.ns_
         ? name_ < rhs.name_
         : ns_ < rhs.ns_)
      : has_ns_ < rhs.has_ns_;
  }

  bool Class_Selector::operator== (const Class_Selector& rhs) const
  {
    // Class has no namespacing
    return name() == rhs.name();
  }

  bool Class_Selector::operator< (const Class_Selector& rhs) const
  {
    // Class has no namespacing
    return name() < rhs.name();
  }

  bool Placeholder_Selector::operator== (const Placeholder_Selector& rhs) const
  {
    // Placeholder has no namespacing
    return name() == rhs.name();
  }

  bool Placeholder_Selector::operator< (const Placeholder_Selector& rhs) const
  {
    // Placeholder has no namespacing
    return name() < rhs.name();
  }

  bool Attribute_Selector::operator== (const Attribute_Selector& rhs) const
  {
    // smaller return, equal go on, bigger abort
    if (is_ns_eq(rhs)) {
      if (name() != rhs.name()) return false;
      if (matcher() != rhs.matcher()) return false;
      const String* lhs_val = value();
      const String* rhs_val = rhs.value();
      return PtrObjEquality()(lhs_val, rhs_val);
    }
    else { return false; }
  }

  bool Attribute_Selector::operator< (const Attribute_Selector& rhs) const
  {
    if (is_ns_eq(rhs)) {
      if (name() < rhs.name()) return true;
      if (name() != rhs.name()) return false;
      if (matcher() < rhs.matcher()) return true;
      if (matcher() != rhs.matcher()) return false;
      const String* lhs_value = value();
      const String* rhs_value = rhs.value();
      if (PtrObjLessThan()(lhs_value, rhs_value)) return true;
      // if (!PtrObjEquality()(lhs_value, rhs_value)) return false;
      return false;
    }
    else { return ns() < rhs.ns(); }
  }

  bool Pseudo_Selector::operator== (const Pseudo_Selector& rhs) const
  {
    if (is_ns_eq(rhs)) {
      if (name() != rhs.name()) return false;
      if (isElement() != rhs.isElement()) return false;
      const String* lhs_arg = argument();
      const String* rhs_arg = rhs.argument();
      if (!PtrObjEquality()(lhs_arg, rhs_arg)) return false;
      const SelectorList* lhs_sel = selector();
      const SelectorList* rhs_sel = rhs.selector();
      return PtrObjEquality()(lhs_sel, rhs_sel);
    }
    else { return false; }
  }

  bool Pseudo_Selector::operator< (const Pseudo_Selector& rhs) const
  {
    if (is_ns_eq(rhs)) {
      if (name() < rhs.name()) return true;
      if (name() != rhs.name()) return false;
      const String* lhs_arg = argument();
      const String* rhs_arg = rhs.argument();
      if (PtrObjLessThan()(lhs_arg, rhs_arg)) return true;
      if (!PtrObjEquality()(lhs_arg, rhs_arg)) return false;
      const SelectorList* lhs_sel = selector();
      const SelectorList* rhs_sel = rhs.selector();
      if (PtrObjLessThan()(lhs_sel, rhs_sel)) return true;
      // if (!PtrObjEquality()(lhs_sel, rhs_sel)) return false;
      return false;
    }
    else { return ns() < rhs.ns(); }
  }

  /*#########################################################################*/
  /*#########################################################################*/

}
