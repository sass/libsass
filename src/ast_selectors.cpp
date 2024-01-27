#include "ast_selectors.hpp"

#include "permutate.hpp"
#include "callstack.hpp"
#include "dart_helpers.hpp"
#include "ast_values.hpp"
#include "exceptions.hpp"
#include "sel_invisible.hpp"
#include "sel_useless.hpp"
#include "sel_bogus.hpp"
#include "cssize.hpp"

#include "debugger.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Selector::Selector(
    const SourceSpan& pstate) :
    AstNode(pstate),
    hash_(0)
  {}

  Selector::Selector(
    const Selector* ptr) :
    AstNode(ptr),
    hash_(0)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool Selector::isUseless() const
  {
    IsUselessVisitor visitor;
    return const_cast<Selector*>(this)->accept(&visitor);
  }

  bool Selector::isInvisible() const
  {
    IsInvisibleVisitor visitor(true);
    return const_cast<Selector*>(this)->accept(&visitor);
  }

  bool Selector::isInvisibleOtherThanBogusCombinators() const
  {
    IsInvisibleVisitor visitor(false);
    return const_cast<Selector*>(this)->accept(&visitor);
  }

  bool Selector::isBogusOtherThanLeadingCombinator() const
  {
    IsBogusVisitor visitor(false);
    return const_cast<Selector*>(this)->accept(&visitor);
  }

  bool Selector::isBogusStrict() const
  {
    IsBogusVisitor visitor(true);
    return const_cast<Selector*>(this)->accept(&visitor);
  }

  bool Selector::isBogusLenient() const
  {
    IsBogusVisitor visitor(false);
    return const_cast<Selector*>(this)->accept(&visitor);
  }

  SelectorList* SelectorList::assertNotBogus(const sass::string& name)
  {
    return this;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SimpleSelector::SimpleSelector(
    const SourceSpan& pstate,
    const sass::string& name) :
    Selector(pstate),
    name_(name)
  {}

  SimpleSelector::SimpleSelector(
    const SourceSpan& pstate,
    sass::string&& name) :
    Selector(pstate),
    name_(name)
  {}

  SimpleSelector::SimpleSelector(
    const SimpleSelector* ptr) :
    Selector(ptr),
    name_(ptr->name_)
  {}

  size_t SimpleSelector::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(this).hash_code());
      hash_combine(hash_, name());
    }
    return hash_;
  }

  CompoundSelector* SimpleSelector::wrapInCompound()
  {
    return SASS_MEMORY_NEW(CompoundSelector, pstate(), { this }, false);
  }

  ComplexSelector* SimpleSelector::wrapInComplex(SelectorCombinatorVector prefixes)
  {
    auto* qwe = wrapInCompound()->wrapInComponent({});
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), std::move(prefixes), { qwe });
  }

  ComplexSelector* CplxSelComponent::wrapInComplex(SelectorCombinatorVector prefixes)
  {
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), std::move(prefixes), { this });
  }

  ComplexSelector* CplxSelComponent::wrapInComplex2()
  {
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), {}, { this });
  }

  ComplexSelector* CompoundSelector::wrapInComplex3()
  {
    auto comp = SASS_MEMORY_NEW(CplxSelComponent, pstate(), {}, this);
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), { comp });
  }

  ComplexSelector* CompoundSelector::wrapInComplex(SelectorCombinatorVector prefixes, SelectorCombinatorVector tails)
  {
    auto comp = SASS_MEMORY_NEW(CplxSelComponent, pstate(), std::move(tails), this);
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), std::move(prefixes), { comp });
  }

  CplxSelComponent* CompoundSelector::wrapInComponent(SelectorCombinatorVector postfixes)
  {
    return SASS_MEMORY_NEW(CplxSelComponent, pstate(), std::move(postfixes), this);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SelectorNS::SelectorNS(
    const SourceSpan& pstate,
    sass::string&& name,
    sass::string&& ns,
    bool hasNs) :
    SimpleSelector(pstate,
      std::move(name)),
    hasNs_(hasNs),
    ns_(std::move(ns))
  {}

  SelectorNS::SelectorNS(
    const SelectorNS* ptr) :
    SimpleSelector(ptr),
    hasNs_(ptr->hasNs_),
    ns_(ptr->ns_)
  {}

  size_t SelectorNS::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(this).hash_code());
      hash_combine(hash_, SimpleSelector::hash());
      if (hasNs_) hash_combine(hash_, ns());
    }
    return hash_;
  }

  /////////////////////////////////////////////////////////////////////////

  // Up-casts the right hand side first to find specialization
  bool SelectorNS::nsMatch(const SimpleSelector& rhs) const
  {
    // if (hasNs() == false || isUniversalNs()) {
    //   return SimpleSelector::nsMatch(rhs);
    // }
    if (auto simple = rhs.isaSelectorNS()) {
      return SelectorNS::nsMatch(*simple);
    }
    return !hasNs() || isUniversalNs();
  }

  /////////////////////////////////////////////////////////////////////////
  // A placeholder selector. (e.g. `%foo`). This doesn't match any elements.
  // It's intended to be extended using `@extend`. It's not a plain CSS
  // selector — it should be removed before emitting a CSS document.
  /////////////////////////////////////////////////////////////////////////

  PlaceholderSelector::PlaceholderSelector(
    const SourceSpan& pstate,
    const sass::string& name) :
    SimpleSelector(pstate, name)
  {}

  PlaceholderSelector::PlaceholderSelector(
    const PlaceholderSelector* ptr) :
    SimpleSelector(ptr)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A type selector. (e.g., `div`, `span` or `*`).
  // This selects elements whose name equals the given name.
  /////////////////////////////////////////////////////////////////////////

  TypeSelector::TypeSelector(
    const SourceSpan& pstate,
    sass::string&& name,
    sass::string&& ns,
    bool hasNs) :
    SelectorNS(pstate,
      std::move(name),
      std::move(ns),
      hasNs)
  {}

  TypeSelector::TypeSelector(
    const TypeSelector* ptr) :
    SelectorNS(ptr)
  {}


  /////////////////////////////////////////////////////////////////////////
  // Class selectors  -- i.e., .foo.
  /////////////////////////////////////////////////////////////////////////

  ClassSelector::ClassSelector(
    const SourceSpan& pstate,
    const sass::string& name)
    : SimpleSelector(pstate, name)
  {}

  ClassSelector::ClassSelector(
    const ClassSelector* ptr) :
    SimpleSelector(ptr)
  {}

  /////////////////////////////////////////////////////////////////////////
  // An ID selector (i.e. `#foo`). This selects elements 
  // whose `id` attribute exactly matches the given name.
  /////////////////////////////////////////////////////////////////////////

  IDSelector::IDSelector(
    const SourceSpan& pstate,
    const sass::string& name) :
    SimpleSelector(pstate, name)
  {}

  IDSelector::IDSelector(
    const IDSelector* ptr) :
    SimpleSelector(ptr)
  {}

  /////////////////////////////////////////////////////////////////////////
  // An attribute selector. This selects for elements
  // with the given attribute, and optionally with a
  // value matching certain conditions as well.
  /////////////////////////////////////////////////////////////////////////

  AttributeSelector::AttributeSelector(
    const SourceSpan& pstate,
    struct QualifiedName&& name,
    sass::string&& op,
    sass::string&& value,
    bool isIdentifier,
    char modifier) :
    SelectorNS(pstate,
      std::move(name.name),
      std::move(name.ns),
      name.hasNs),
    op_(std::move(op)),
    value_(std::move(value)),
    modifier_(modifier),
    isIdentifier_(isIdentifier)
  {}

  AttributeSelector::AttributeSelector(
    const AttributeSelector* ptr) :
    SelectorNS(ptr),
    op_(ptr->op_),
    value_(ptr->value_),
    modifier_(ptr->modifier_),
    isIdentifier_(ptr->isIdentifier_)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A pseudo-class or pseudo-element selector (e.g., `:content`
  // or `:nth-child`). The semantics of a specific pseudo selector
  // depends on its name. Some selectors take arguments, including
  // other selectors. Sass manually encodes logic for each pseudo
  // selector that takes a selector as an argument, to ensure that
  // extension and other selector operations work properly.
  /////////////////////////////////////////////////////////////////////////

  PseudoSelector::PseudoSelector(
    const SourceSpan& pstate,
    const sass::string& name,
    bool element) :
    SimpleSelector(pstate, name),
    normalized_(StringUtils::unvendor(name)),
    argument_(),
    selector_(),
    isSyntacticClass_(!element),
    isClass_(!element && !isFakePseudoElement(normalized_))
  {}

  PseudoSelector::PseudoSelector(
    const PseudoSelector* ptr) :
    SimpleSelector(ptr),
    normalized_(ptr->normalized()),
    argument_(ptr->argument()),
    selector_(ptr->selector()),
    isSyntacticClass_(ptr->isSyntacticClass()),
    isClass_(ptr->isClass())
  { }


  bool PseudoSelector::hasInvisible() const
  {
    return selector() && selector()->empty() && name() != "not";
  }

  size_t PseudoSelector::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(this).hash_code());
      hash_combine(hash_, argument_);
      if (selector_) hash_combine(
        hash_, selector_->hash());
    }
    return hash_;
  }

  // Implement for cleanup phase
  bool PseudoSelector::empty() const {
    // Only considered empty if selector is
    // available but has no items in it.
    return argument_.empty() && name().empty() &&
      (selector() && selector()->empty());
  }

  bool PseudoSelector::isHostContext() const {
    return isClass_ && name_ == "host-context" &&
      selector_ != nullptr; // && !selector_->empty();
  }

  PseudoSelector* PseudoSelector::withSelector(SelectorList* selector)
  {
    PseudoSelector* pseudo = SASS_MEMORY_COPY(this);
    pseudo->selector(selector);
    return pseudo;
  }

  const Selector* PseudoSelector::hasAnyExplicitParent() const {
    if (selector_ == nullptr) return nullptr;
    return selector_->getExplicitParent();
  }

  /////////////////////////////////////////////////////////////////////////
  // Complex Selectors are the most important class of selectors.
  // A Selector List consists of Complex Selectors (separated by comma)
  // Complex Selectors are itself a list of Compounds and Combinators
  // Between each item there is an implicit ancestor of combinator
  /////////////////////////////////////////////////////////////////////////


  ComplexSelector::ComplexSelector(
    const SourceSpan& pstate,
    CplxSelComponentVector&& components) :
    Selector(pstate),
    Vectorized(std::move(components)),
    chroots_(false),
    hasPreLineFeed_(false),
    hasLineBreak_(false),
    leadingCombinators_({})
  {}

  ComplexSelector::ComplexSelector(
    const SourceSpan& pstate,
    const SelectorCombinatorVector& leadingCombinators,
    const CplxSelComponentVector& components,
    bool hasLineBreak) :
    Selector(pstate),
    Vectorized(components),
    chroots_(hasLineBreak),
    hasPreLineFeed_(hasLineBreak),
    hasLineBreak_(hasLineBreak),
    leadingCombinators_(leadingCombinators)
  {}

  ComplexSelector::ComplexSelector(
    const SourceSpan& pstate,
    SelectorCombinatorVector&& leadingCombinators,
    CplxSelComponentVector && components,
    bool hasLineBreak) :
    Selector(pstate),
    Vectorized(std::move(components)),
    chroots_(false),
    hasPreLineFeed_(hasLineBreak),
    hasLineBreak_(hasLineBreak),
    leadingCombinators_(std::move(leadingCombinators))
  {}

  ComplexSelector::ComplexSelector(
    const ComplexSelector* ptr,
    bool childless) :
    Selector(ptr),
    Vectorized(ptr, childless),
    chroots_(ptr->chroots_),
    hasPreLineFeed_(ptr->hasPreLineFeed_),
    hasLineBreak_(ptr->hasLineBreak_),
    leadingCombinators_(ptr->leadingCombinators_)
  {}


  unsigned long ComplexSelector::specificity() const
  {
    if (specificity_ == 0xFFFFFFFF) {
      specificity_ = 0;
      for (const auto& component : elements()) {
        specificity_ += component->selector()->specificity();
      }
    }
    return specificity_;
  }

  unsigned long ComplexSelector::maxSpecificity() const
  {
    if (maxSpecificity_ == 0xFFFFFFFF) {
      maxSpecificity_ = 0;
      for (const auto& component : elements()) {
        maxSpecificity_ += component->selector()->maxSpecificity();
      }
    }
    return maxSpecificity_;
  }

  unsigned long ComplexSelector::minSpecificity() const
  {
    if (minSpecificity_ == 0xFFFFFFFF) {
      minSpecificity_ = 0;
      for (const auto& component : elements()) {
        minSpecificity_ += component->selector()->minSpecificity();
      }
    }
    return minSpecificity_;
  }

  ComplexSelector* ComplexSelector::produce() {
      sass::vector<CplxSelComponentObj> copy;
      for (CplxSelComponent* child : elements_) {
        if (child->selector() == nullptr) continue;
        CompoundSelectorObj asd = child->selector()->produce();
        copy.push_back(asd->wrapInComponent(child->combinators())); // ToDo combi
      }
      return SASS_MEMORY_NEW(ComplexSelector,
          pstate_, std::move(copy));
  }

  bool ComplexSelector::hasInvisible() const
  {
    if (empty()) return true;
    if (isBogusStrict()) return true;
    for (const auto& component : elements()) {
      if (component->hasInvisible()) return true;
    }
    return false;
  }

  bool ComplexSelector::hasPlaceholder() const
  {
    for (const auto& child : elements()) {
      if (child->hasPlaceholder()) {
        return true;
      }
    }
    return false;
  }

  //bool ComplexSelector::isUseless() const {
  //  return leadingCombinators_.size() > 1;
  //}

  bool ComplexSelector::hasOneLeadingCombinators() const {
    return leadingCombinators_.size() == 1;
  }

  SelectorCombinator* ComplexSelector::getLeadingCombinator() const
  {
    if (!leadingCombinators_.empty())
      return leadingCombinators_[0];
    return nullptr;
  }

  CompoundSelector* ComplexSelector::getSingleCompound() const
  {
    if (elements_.size() != 1) return nullptr;
    if (!leadingCombinators_.empty()) return nullptr;
    auto& first = elements_.front();
    if (!first->combinators().empty()) return nullptr;
    return first->selector();
  }

  SelectorList* ComplexSelector::wrapInList()
  {
    return SASS_MEMORY_NEW(SelectorList, pstate(), { this });
  }

  size_t ComplexSelector::hash() const
  {
    if (Vectorized<CplxSelComponent>::hash_ == 0) {
      Selector::hash_ = Vectorized<CplxSelComponent>::hash();
    }
    return Selector::hash_;
  }

  const Selector* ComplexSelector::getExplicitParent() const
  {
    const Selector* rv = nullptr;
    for (const CplxSelComponentObj& component : elements()) {
      rv = component->hasAnyExplicitParent();
      if (rv != nullptr) return rv;
    }
    return nullptr;
  }

  /////////////////////////////////////////////////////////////////////////
  // Base class for complex selector components
  /////////////////////////////////////////////////////////////////////////

  //CplxSelComponent::CplxSelComponent(
  //  const SourceSpan& pstate,
  //  bool hasPostLineBreak) :
  //  AstNode(pstate),
  //  hasPostLineBreak_(hasPostLineBreak)
  //{}

  CplxSelComponent::CplxSelComponent(
    const SourceSpan & pstate,
    SelectorCombinatorVector&& combinators,
    CompoundSelector* selector,
    bool hasPostLineBreak) :
    AstNode(pstate),
    combinators_(std::move(combinators)),
    selector_(selector),
    hasPostLineBreak_(hasPostLineBreak)
  {}

  CplxSelComponent::CplxSelComponent(
    const CplxSelComponent* ptr) :
    AstNode(ptr),
    combinators_(ptr->combinators_),
    selector_(ptr->selector_),
    hasPostLineBreak_(ptr->hasPostLineBreak())
  {}

  size_t CplxSelComponent::hash() const
  {
    return 123123;
  }

  /////////////////////////////////////////////////////////////////////////
  // A specific combinator between compound selectors
  /////////////////////////////////////////////////////////////////////////

//  SelectorCombinator::SelectorCombinator(
//    const SourceSpan& pstate,
//    SelectorCombinator::Combinator combinator,
//    bool hasPostLineBreak) :
//    CplxSelComponent(pstate, hasPostLineBreak),
//    combinator_(combinator)
//  {}
//
//  SelectorCombinator::SelectorCombinator(
//    const SelectorCombinator* ptr) :
//    CplxSelComponent(ptr),
//    combinator_(ptr->combinator_)
//  {}
//
//  // Hash implementation is very simple
//  size_t SelectorCombinator::hash() const
//  {
//    if (hash_ == 0) {
//      hash_start(hash_, typeid(this).hash_code());
//      hash_combine(hash_, (size_t)combinator());
//    }
//    return hash_;
//  }

  /////////////////////////////////////////////////////////////////////////
  // A compound selector consists of multiple simple selectors. It will be
  // either implicitly or explicitly connected to its parent sass selector.
  // According to the specs we could also unify the tag selector into this,
  // as AFAICT only one tag selector is ever allowed. Further we could free
  // up the pseudo selectors from being virtual, as they must be last always.
  // https://github.com/sass/libsass/pull/3101
  /////////////////////////////////////////////////////////////////////////

  CompoundSelector::CompoundSelector(
    const SourceSpan& pstate,
    bool hasPostLineBreak) :
    Selector(pstate),
    withExplicitParent_(false),
    hasPostLineBreak_(hasPostLineBreak)
  {}

  CompoundSelector::CompoundSelector(
    const SourceSpan& pstate,
    sass::vector<SimpleSelectorObj>&& selectors,
    bool hasPostLineBreak) :
    Selector(pstate),
    Vectorized(std::move(selectors)),
    withExplicitParent_(false),
    hasPostLineBreak_(hasPostLineBreak)
  {}

  CompoundSelector::CompoundSelector(
    const CompoundSelector* ptr,
    bool childless) :
    Selector(ptr),
    Vectorized(ptr, childless),
    withExplicitParent_(ptr->withExplicitParent()),
    hasPostLineBreak_(ptr->hasPostLineBreak_)
  {}

  size_t CompoundSelector::hash() const
  {
    if (Vectorized<SimpleSelector>::hash_ == 0) {
      Selector::hash_ = Vectorized<SimpleSelector>::hash();
    }
    return Selector::hash_;
  }

  unsigned long CompoundSelector::specificity() const
  {
    if (specificity_ == 0xFFFFFFFF) {
      specificity_ = 0;
      for (const auto& component : elements()) {
        specificity_ += component->specificity();
      }
    }
    return specificity_;
  }

  unsigned long CompoundSelector::maxSpecificity() const
  {
    if (maxSpecificity_ == 0xFFFFFFFF) {
      maxSpecificity_ = 0;
      for (const auto& simple : elements()) {
        maxSpecificity_ += simple->maxSpecificity();
      }
    }
    return maxSpecificity_;
  }

  unsigned long CompoundSelector::minSpecificity() const
  {
    if (minSpecificity_ == 0xFFFFFFFF) {
      minSpecificity_ = 0;
      for (const auto& simple : elements()) {
        minSpecificity_ += simple->minSpecificity();
      }
    }
    return minSpecificity_;
  }

  CompoundSelector* CompoundSelector::produce()
  {
    sass::vector<SimpleSelectorObj> copy;
    for (SimpleSelector* child : elements_) {
      copy.emplace_back(child);
    }
    return SASS_MEMORY_NEW(CompoundSelector,
      pstate_, std::move(copy));
  }

  const Selector* CompoundSelector::hasAnyExplicitParent() const
  {
    if (withExplicitParent()) return this;
    // ToDo: dart sass has another check?
    // if (front->isaNameSpaceSelector()) {
    //  if (front->ns() != "") return false;
    // }
    const Selector* rv = nullptr;
    for (const SimpleSelectorObj& s : elements()) {
      if (s) rv = s->hasAnyExplicitParent();
      if (rv != nullptr) return rv;
    }
    return nullptr;
  }

  bool CompoundSelector::hasPlaceholder() const
  {
    if (size() == 0) return false;
    for (const SimpleSelectorObj& ss : elements()) {
      if (ss && ss->isaPlaceholderSelector()) return true;
    }
    return false;
  }

  bool CompoundSelector::hasInvisible() const
  {
    for (const SimpleSelectorObj& sel : elements()) {
      if (sel && sel->hasInvisible()) return true;
    }
    return false;
  }

  // Determine if given `this` is a sub-selector of `sub`
  bool CompoundSelector::isSuperselectorOf(const CompoundSelector* sub) const
  {
    return compoundIsSuperselector(this, sub);
  }

  /////////////////////////////////////////////////////////////////////////
  // Comma-separated selector groups.
  /////////////////////////////////////////////////////////////////////////

  SelectorList::SelectorList(
    const SourceSpan& pstate,
    sass::vector<ComplexSelectorObj>&& complexes) :
    Selector(pstate),
    Vectorized(std::move(complexes))
  {}

  SelectorList::SelectorList(
    const SelectorList* ptr,
    bool childless) :
    Selector(ptr),
    Vectorized(ptr, childless)
  {}

  size_t SelectorList::hash() const
  {
    if (Vectorized<ComplexSelector>::hash_ == 0) {
      Selector::hash_ = Vectorized<ComplexSelector>::hash();
    }
    return Selector::hash_;
  }

  unsigned long SelectorList::maxSpecificity() const
  {
    if (maxSpecificity_ == 0xFFFFFFFF) {
      maxSpecificity_ = 0;
      for (const auto& complex : elements()) {
        maxSpecificity_ = std::max(
          complex->maxSpecificity(),
          maxSpecificity_);
      }
    }
    return maxSpecificity_;
  }

  unsigned long SelectorList::minSpecificity() const
  {
    if (minSpecificity_ == 0xFFFFFFFF) {
      minSpecificity_ = 0;
      for (const auto& complex : elements()) {
        maxSpecificity_ = std::min(
          complex->minSpecificity(),
          maxSpecificity_);
      }
    }
    return minSpecificity_;
  }

  const Selector* SelectorList::getExplicitParent() const
  {
    const Selector* rv = nullptr;
    for (const auto& s : elements()) {
      rv = s->getExplicitParent();
      if (rv != nullptr) return rv;
    }
    return nullptr;
  }

  Value* SelectorList::toValue() const
  {
    ListObj list = SASS_MEMORY_NEW(List,
      pstate(), {}, SASS_COMMA);
    list->reserve(size());

    for (ComplexSelector* complex : elements()) {
      list->append(complex->toList());
    }
    if (list->size()) return list.detach();
    return SASS_MEMORY_NEW(Null, pstate());
  }

  bool SelectorList::hasPlaceholder() const
  {
    for (const auto& child : elements()) {
      if (child->hasPlaceholder()) {
        return true;
      }
    }
    return false;
  }

  bool CplxSelComponent::operator==(const CplxSelComponent& rhs) const
  {
    if (combinators_ != rhs.combinators_) return false;
    if (selector_ && rhs.selector_) return *selector_ == *rhs.selector_;
    return selector_ == nullptr && rhs.selector_ == nullptr;
  }

  const Selector* CplxSelComponent::hasAnyExplicitParent() const
  {
    if (selector_ == nullptr) return nullptr;
    return selector_->hasAnyExplicitParent();
  }

  bool CplxSelComponent::hasPlaceholder() const
  {
    if (selector() == nullptr) return false;
    for (const auto& child : selector()->elements()) {
      if (child->hasPlaceholder()) {
        return true;
      }
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  // Below are the resolveParentSelectors implementations
  /////////////////////////////////////////////////////////////////////////


  List* ComplexSelector::toList() const
  {
    ListObj list = SASS_MEMORY_NEW(List,
      pstate(), {}, SASS_SPACE);
    if (leadingCombinators_.size() > 0) {
      for (size_t i = 0; i < leadingCombinators_.size(); i++) {
        sass::string prefix(leadingCombinators_[i]->toString());
        list->append(SASS_MEMORY_NEW(String,
          pstate(), std::move(prefix)));
      }
    }
    for (CplxSelComponent* component : elements()) {
      if (component->selector()) {
        sass::string prefix(component->selector()->inspect());
        list->append(SASS_MEMORY_NEW(String,
          pstate(), std::move(prefix)));
      }
      for (auto combi : component->combinators()) {
        sass::string prefix(combi->toString());
        list->append(SASS_MEMORY_NEW(String,
          pstate(), std::move(prefix)));
      }
    }
    return list.detach();
  }


  template <typename T>
  sass::string VecToString(sass::vector<T> exts) {
    sass::string msg = "[";
    for (auto& entry : exts) {
      msg += entry->inspect();
    }
    return msg + "]";
  }


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SelectorCombinator::SelectorCombinator(
    const SourceSpan& pstate,
    SelectorPrefix combinator,
    bool hasPostLineBreak) :
    AstNode(pstate),
    combinator_(combinator),
    hasPostLineBreak_(hasPostLineBreak)
  {}

  SelectorCombinator::SelectorCombinator(
    const SelectorCombinator* ptr) :
    AstNode(ptr),
    combinator_(ptr->combinator_),
    hasPostLineBreak_(ptr->hasPostLineBreak_)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////









  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  sass::string CplxSelComponent::inspect() const {
    sass::string text;
    text += selector_->inspect();
    for (auto asd : combinators_) {
      text += " " + asd->toString();
    }
    return text;
  }

  sass::string CplxSelComponent::inspecter() const {
    sass::string text;
    text += selector_->inspect();
    for (auto asd : combinators_) {
      text += " " + asd->toString();
    }
    return text;
  }

  void CplxSelComponent::appendCombinators(SelectorCombinatorVector trails)
  {
    combinators_.insert(combinators_.end(),
      trails.begin(), trails.end());
  }

  // Fully converted 16.01.2024 (untested)
  CplxSelComponent* CplxSelComponent::withAdditionalCombinators(
    const SelectorCombinatorVector& others)
  {
    if (others.empty()) return this;
    SelectorCombinatorVector merged(combinators_);
    merged.insert(merged.end(),
      others.begin(), others.end());
    return SASS_MEMORY_NEW(CplxSelComponent,
      pstate(), std::move(merged), selector());
  }

  // Fully converted 16.01.2024 (untested)
  ComplexSelector* ComplexSelector::withAdditionalCombinators(
    const SelectorCombinatorVector& combinators)
  {
    if (combinators.empty()) return this;
    CplxSelComponentVector components(elements_);
    if (empty()) {
      // Just add to existing leading combinators
      SelectorCombinatorVector merged(leadingCombinators_);
      merged.insert(merged.end(), combinators.begin(), combinators.end());
      return SASS_MEMORY_NEW(ComplexSelector, pstate_,
        std::move(combinators), std::move(components));
    }
    else {
      // SelectorCombinatorVector merged(elements_.back()->combinators());
      components.back() = components.back()->withAdditionalCombinators(combinators);
      return SASS_MEMORY_NEW(ComplexSelector, pstate_,
        leadingCombinators_, std::move(components));
    }

    // SelectorCombinatorVector merged(combinators_);
    // merged.insert(merged.end(),
    //   others.begin(), others.end());
    // return SASS_MEMORY_NEW(CplxSelComponent,
    //   pstate(), std::move(merged), selector());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Fully converted 16.01.2024 (untested)
  ComplexSelector* ComplexSelector::concatenate(
    ComplexSelector* child,
    const SourceSpan& span,
    bool forceLineBreak)
  {
    // Create copies and move later
    SelectorCombinatorVector leads;
    CplxSelComponentVector merged;
    // Case when child has no leading combinators
    if (child->leadingCombinators().empty()) {
      leads.insert(leads.end(),
        this->leadingCombinators_.begin(),
        this->leadingCombinators_.end());
      merged.insert(merged.end(),
        this->begin(), this->end());
      merged.insert(merged.end(),
        child->begin(), child->end());
    }
    // Case when lha has some components
    else if (size() > 0) {
      leads.insert(leads.end(),
        leadingCombinators_.begin(),
        leadingCombinators_.end());
      // Append all but last items
      merged.insert(merged.end(),
        this->begin(), this->end() - 1);
      // last with additional combinator
      merged.push_back(last()
        ->withAdditionalCombinators(
          child->leadingCombinators()));
      // merge in child componentes
      merged.insert(merged.end(),
        child->begin(), child->end());
    }
    // Case when lhs has no componentes
    else {
      leads.insert(leads.end(),
        this->leadingCombinators_.begin(),
        this->leadingCombinators_.end());
      leads.insert(leads.end(),
        child->leadingCombinators_.begin(),
        child->leadingCombinators_.end());
      merged.insert(merged.end(),
        child->begin(), child->end());
    }
    // Return the concated complex selector
    return SASS_MEMORY_NEW(ComplexSelector,
      span, std::move(leads), std::move(merged), // avoid copy
      hasLineBreak_ || child->hasLineBreak_ || forceLineBreak);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////








  sass::vector<ComplexSelectorObj> ComplexSelector::resolveParentSelectors(
    SelectorList* parent, BackTraces& traces, bool implicit_parent)
  {

    const Selector* expl = getExplicitParent();
    // debug_ast(this, "test: ");
    if (!parent && expl != nullptr) {
      throw Exception::TopLevelParent(traces, expl->pstate());
    }

    sass::vector<sass::vector<ComplexSelectorObj>> selectors;

    // bool cr = chroots();
    // bool he = hasExplicitParent();

    // Check if selector should implicit get a parent
    if (!chroots() && expl == nullptr) {
      // Check if we should never connect to parent
      if (!implicit_parent) { return { this }; }
      // Otherwise add parent selectors at the beginning
      if (parent) { selectors.emplace_back(parent->elements()); }
    }

    if (elements_.size() == 0) {
      // Has no things to append
      // Preserve component combinators
      // std::cerr << "do me\n";
    }

    // bool first = true;
    // Loop all items from the complex selector
    // for (CplxSelComponent* component : this->elements()) {
    for (size_t n = 0; n < this->size(); n++) {

      CplxSelComponent* component = elements_[n];

      // std::cerr << "============\n";

      // if (parent) std::cerr << "parent [" << parent->inspect() << "]\n";
      // std::cerr << "resolve [" << component->inspecter() << "]\n";
      if (CompoundSelector* compound = component->selector()) {

        SelectorCombinatorVector leads;
        if (n == 0) leads.insert(leads.end(),
          leadingCombinators_.begin(),
          leadingCombinators_.end());
        auto tails(component->combinators());

        //if (selectors.size() > 0) leads.clear();
        // loosing postfix combinators of component?
        sass::vector<ComplexSelectorObj> complexes =
          compound->resolveParentSelectors2(parent, traces,
            leads, tails, implicit_parent);

        for (auto qwe : complexes) {
          // std::cerr << "RESOL [" << qwe->inspect() << "]\n";
        }

        // for (auto sel : complexes) { sel->hasPreLineFeed(hasPreLineFeed()); }
        if (complexes.size() > 0) {
          selectors.emplace_back(complexes);
        }
      }
      else {
        // component->hasPreLineFeed(hasPreLineFeed());
        selectors.push_back({ component->wrapInComplex(leadingCombinators_) });
      }
    }

    // std::cerr << "permutate now\n";

    // Permutate through all paths
    // for (auto s : selectors) { for (auto q : s) { std::cerr << "sel [" << q->inspect() << "]\n"; } }
    selectors = permutateAlt(selectors);
    // for (auto s : selectors) { for (auto q : s) { std::cerr << "perm [" << q->inspect() << "]\n"; } }

    // Create final selectors from path permutations
    sass::vector<ComplexSelectorObj> resolved;
    for (sass::vector<ComplexSelectorObj>& append : selectors) {

      if (append.empty()) continue;

      ComplexSelectorObj front = SASS_MEMORY_COPY(append[0]);
      // ToDo: this seems suspicious, why this logic?
      if (hasPreLineFeed() && expl == nullptr) {
        front->hasPreLineFeed(true);
      }
      // ToDo: remove once we know how to handle line feeds
      // ToDo: currently a mash-up between ruby and dart sass
      // if (has_real_parent_ref()) first->has_line_feed(false);
      // first->has_line_break(first->has_line_break() || has_line_break());
      front->chroots(true); // has been resolved by now

      for (size_t i = 1; i < append.size(); i += 1) {
        if (append[i]->hasPreLineFeed()) {
          front->hasPreLineFeed(true);
        }
        for (auto tail : append[i]->elements()) {
          // if (front->elements().size() > 0) {
          //   SelectorCombinatorVector trails
          //     = front->elements().back()->combinators();
          //   //
          //   trails.insert(trails.end(),
          //     append[i]->leadingCombinators_.begin(),
          //     append[i]->leadingCombinators_.end());
          // 
          //   front->elements().back()->combinators(trails);
          // }
          if (front->size() > 0 && append[i]->leadingCombinators_.size() > 0) {
            front->elements().back() = SASS_MEMORY_NEW(CplxSelComponent, front->elements().back().ptr());
            front->elements().back()->appendCombinators(append[i]->leadingCombinators_);
          }
          front->elements().push_back(tail);
        }
        // first->concat(items[i]);
      }
      // debug_ast(first, "resolved: ");
     // std::cerr << " + [" << first->inspect() << "]\n";
      resolved.emplace_back(front);
    }

    if (elements_.size() == 0) {
      // Has no things to append
      // Preserve component combinators
      for (size_t i = 0; i < resolved.size(); i++) {
        if (resolved[i]->size() == 0) {
          std::cerr << "more weird edge case\n";
        }
        else {
          resolved[i]->elements().back() = SASS_MEMORY_NEW(CplxSelComponent, resolved[i]->elements().back().ptr());
          resolved[i]->elements().back()->appendCombinators(leadingCombinators_);
        }
      }
    }

    for (auto q : resolved) {
      // std::cerr << "res => [" << q->inspect() << "]\n";
    }

    // std::cerr << "=> " << VecToString(resolved) << "\n";

    return resolved;
  }
  // EO ComplexSelector::resolveParentSelectors

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // 

  sass::vector<ComplexSelectorObj>
    CompoundSelector::resolveParentSelectors2(
      SelectorList* parents, BackTraces& traces,
      SelectorCombinatorVector prefixes, // from complex selector
      SelectorCombinatorVector tails, // From compound component
      bool implicit_parent)
  {

//    prefixes.clear();
//    tails.clear();

    // must add prefixes to parent's last selector combinators

    sass::vector<ComplexSelectorObj> rv;

    // Missing this->combinators

    if (parents) {
      for (SimpleSelectorObj simple : elements()) {
        if (PseudoSelector* pseudo = simple->isaPseudoSelector()) {
          if (SelectorList* sel = pseudo->selector()) {
            auto asd = sel->resolveParentSelectors(
              parents, traces, implicit_parent);
            std::cerr << "Resolved [" << asd->inspect() << "]\n";
            pseudo->selector(asd);
          }
        }
      }
    }

    // Mix with parents from stack
    // Equivalent to dart-sass parent selector tail
    if (withExplicitParent()) {
      if (parents == nullptr) return { this->wrapInComplex(prefixes, tails) };
      SASS_ASSERT(parents != nullptr, "Parent must be defined");

      for (auto parent1 : parents->elements()) {
        // The parent complex selector has a compound selector
        if (parent1->size() == 0) {
          // Can't insert parent that ends with a combinator
          // where the parent selector is followed by something
          //callStackFrame frame(traces, complex->last()->pstate());
          //if (size() > 0) { throw Exception::InvalidParent(parent, traces, this); }
          // Just append ourself to results
          //std::cerr << "have no parent\n";
          rv.emplace_back(wrapInComplex(prefixes, tails));
        }
        else {
          CplxSelComponent* cptail = parent1->last();

          if (cptail->combinators().size() > 0) {
            callStackFrame frame(traces, cptail->combinators().back()->pstate());
            if (size() > 0) throw Exception::InvalidParent(parent1, traces, this);
          }

          cptail = SASS_MEMORY_NEW(CplxSelComponent, cptail);

          // Add prefixes from incoming to parent
//          cptail->appendCombinators(tails);
          // ctail->combinators()

          CompoundSelector* ptail = cptail->selector();
          ptail = SASS_MEMORY_COPY(ptail);

          if (ptail && ptail->size() > 0) {
            // Create copies to alter them
            ComplexSelectorObj parent = SASS_MEMORY_COPY(parent1);

            // Check if we can merge front with back
            if (size() > 0 && ptail->size() > 0) {
              SimpleSelector* front = first();
              auto simple_back = ptail->last();
              auto simple_front = front->isaTypeSelector();
              // If they are type/simple selectors ...
              if (simple_front && simple_back) {
                // ... we can combine the names into one
                simple_back = SASS_MEMORY_COPY(simple_back);
                auto name = simple_back->name();
                name += simple_front->name();
                simple_back->name(name);
                // Replace with modified simple selector
                ptail->setLast(simple_back);
                // Append rest of selector components
                ptail->concat({ begin() + 1, end() });
              }
              else {
                // Append us to parent
                ptail->concat(this);
              }
            }
            else {
              // Append us to parent
              ptail->concat(this);
            }
            // Reset the parent selector tail with
            // the combination of parent plus ourself
            // complex->setLast(tail->wrapInComponent());
            SelectorCombinatorVector pre;

            // pre.insert(pre.end(), prefixes.begin(), prefixes.end());

            // complex->elements().back() = SASS_MEMORY_NEW(CplxSelComponent, complex->last());
            //auto pres = complex->last()->combinators();
            //cp.insert(cp.end(), pres.begin(), pres.end());
            //cp.insert(cp.end(), tails.begin(), tails.end());
            cptail->selector(ptail);
            // cptail->appendCombinators(tails);
            parent->setLast(cptail);
            // Append to results

            parent = parent->withAdditionalCombinators(tails);
            prefixes.insert(prefixes.end(),
              parent->leadingCombinators().begin(),
              parent->leadingCombinators().end());
            parent->leadingCombinators(prefixes);

            rv.emplace_back(parent);
          }
          // SelectorCombinator
          else {
            //std::cerr << "last parent has only combinators\n";
            // Can't insert parent that ends with a combinator
            // where the parent selector is followed by something
            callStackFrame frame(traces, parent1->last()->pstate());
            if (size() > 0) { throw Exception::InvalidParent(parents, traces, this); }
            // Just append ourself to results
            rv.emplace_back(wrapInComplex(prefixes, tails));
          }
        }
      }

    }
    // No parent
    else {
      // Wrap and append compound selector
      SelectorCombinatorVector trailing;
      trailing.insert(trailing.end(),
        tails.begin(), tails.end());
      //trailing.insert(trailing.end(),
      //  combinator().begin(), tails.end());

      rv.emplace_back(wrapInComplex(prefixes, tails));
    }

    for (auto a : rv) {
      // std::cerr << "final { " << a->inspect() << " }\n";
    }

    return rv;

  }
  // EO CompoundSelector::resolveParentSelectors

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  SelectorList* SelectorList::resolveParentSelectors(
    SelectorList* parent, BackTraces& traces, bool implicit_parent)
  {
    sass::vector<sass::vector<ComplexSelectorObj>> lists;
    for (ComplexSelector* sel : elements()) {
      lists.emplace_back(sel->resolveParentSelectors
      (parent, traces, implicit_parent));
    }
    return SASS_MEMORY_NEW(SelectorList, pstate(),
      flattenVertically<ComplexSelectorObj>(lists));
  }
  // EO SelectorList::resolveParentSelectors


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////


}


