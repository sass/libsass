#include "ast_selectors.hpp"

#include "permutate.hpp"
#include "callstack.hpp"
#include "dart_helpers.hpp"
#include "ast_values.hpp"
#include "exceptions.hpp"
#include "cssize.hpp"

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

  ComplexSelector* SimpleSelector::wrapInComplex()
  {
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), { wrapInCompound() });
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  NameSpaceSelector::NameSpaceSelector(
    const SourceSpan& pstate,
    sass::string&& name,
    sass::string&& ns,
    bool hasNs) :
    SimpleSelector(pstate,
      std::move(name)),
    hasNs_(hasNs),
    ns_(std::move(ns))
  {}

  NameSpaceSelector::NameSpaceSelector(
    const NameSpaceSelector* ptr) :
    SimpleSelector(ptr),
    hasNs_(ptr->hasNs_),
    ns_(ptr->ns_)
  {}

  size_t NameSpaceSelector::hash() const
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
  bool NameSpaceSelector::nsMatch(const SimpleSelector& rhs) const
  {
    // if (hasNs() == false || isUniversalNs()) {
    //   return SimpleSelector::nsMatch(rhs);
    // }
    if (auto simple = rhs.isaNameSpaceSelector()) {
      return NameSpaceSelector::nsMatch(*simple);
    }
    return false;
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
    NameSpaceSelector(pstate,
      std::move(name),
      std::move(ns),
      hasNs)
  {}

  TypeSelector::TypeSelector(
    const TypeSelector* ptr) :
    NameSpaceSelector(ptr)
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
    NameSpaceSelector(pstate,
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
    NameSpaceSelector(ptr),
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
    normalized_(Util::unvendor(name)),
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

  PseudoSelector* PseudoSelector::withSelector(SelectorList* selector)
  {
    PseudoSelector* pseudo = SASS_MEMORY_COPY(this);
    pseudo->selector(selector);
    return pseudo;
  }

  bool PseudoSelector::hasAnyExplicitParent() const {
    return selector() && selector()->hasExplicitParent();
  }

  /////////////////////////////////////////////////////////////////////////
  // Complex Selectors are the most important class of selectors.
  // A Selector List consists of Complex Selectors (separated by comma)
  // Complex Selectors are itself a list of Compounds and Combinators
  // Between each item there is an implicit ancestor of combinator
  /////////////////////////////////////////////////////////////////////////

  ComplexSelector::ComplexSelector(
    const SourceSpan& pstate,
    SelectorComponentVector&& components) :
    Selector(pstate),
    Vectorized(std::move(components)),
    chroots_(false),
    hasPreLineFeed_(false)
  {}

  ComplexSelector::ComplexSelector(
    const ComplexSelector* ptr,
    bool childless) :
    Selector(ptr),
    Vectorized(ptr, childless),
    chroots_(ptr->chroots()),
    hasPreLineFeed_(ptr->hasPreLineFeed())
  {}

  unsigned long ComplexSelector::specificity() const
  {
    if (specificity_ == 0xFFFFFFFF) {
      specificity_ = 0;
      for (const auto& component : elements()) {
        specificity_ += component->specificity();
      }
    }
    return specificity_;
  }

  unsigned long ComplexSelector::maxSpecificity() const
  {
    if (maxSpecificity_ == 0xFFFFFFFF) {
      maxSpecificity_ = 0;
      for (const auto& component : elements()) {
        maxSpecificity_ += component->maxSpecificity();
      }
    }
    return maxSpecificity_;
  }

  unsigned long ComplexSelector::minSpecificity() const
  {
    if (minSpecificity_ == 0xFFFFFFFF) {
      minSpecificity_ = 0;
      for (const auto& component : elements()) {
        minSpecificity_ += component->minSpecificity();
      }
    }
    return minSpecificity_;
  }

  bool ComplexSelector::hasInvisible() const
  {
    if (empty()) return true;
    for (const auto& component : elements()) {
      if (component->hasInvisible()) return true;
    }
    return false;
  }

  SelectorList* ComplexSelector::wrapInList()
  {
    return SASS_MEMORY_NEW(SelectorList, pstate(), { this });
  }

  size_t ComplexSelector::hash() const
  {
    if (Vectorized<SelectorComponent>::hash_ == 0) {
      Selector::hash_ = Vectorized<SelectorComponent>::hash();
    }
    return Selector::hash_;
  }

  bool ComplexSelector::hasExplicitParent() const
  {
    for (const SelectorComponentObj& component : elements()) {
      if (component->hasAnyExplicitParent()) return true;
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  // Base class for complex selector components
  /////////////////////////////////////////////////////////////////////////

  SelectorComponent::SelectorComponent(
    const SourceSpan& pstate,
    bool hasPostLineBreak) :
    Selector(pstate),
    hasPostLineBreak_(hasPostLineBreak)
  {}

  SelectorComponent::SelectorComponent(
    const SelectorComponent* ptr) :
    Selector(ptr),
    hasPostLineBreak_(ptr->hasPostLineBreak())
  {}

  /////////////////////////////////////////////////////////////////////////
  // A specific combinator between compound selectors
  /////////////////////////////////////////////////////////////////////////

  SelectorCombinator::SelectorCombinator(
    const SourceSpan& pstate,
    SelectorCombinator::Combinator combinator,
    bool hasPostLineBreak) :
    SelectorComponent(pstate, hasPostLineBreak),
    combinator_(combinator)
  {}

  SelectorCombinator::SelectorCombinator(
    const SelectorCombinator* ptr) :
    SelectorComponent(ptr),
    combinator_(ptr->combinator_)
  {}

  // Hash implementation is very simple
  size_t SelectorCombinator::hash() const
  {
    if (hash_ == 0) {
      hash_start(hash_, typeid(this).hash_code());
      hash_combine(hash_, (size_t)combinator());
    }
    return hash_;
  }

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
    SelectorComponent(
      pstate, hasPostLineBreak),
    withExplicitParent_(false)
  {}

  CompoundSelector::CompoundSelector(
    const SourceSpan& pstate,
    sass::vector<SimpleSelectorObj>&& selectors,
    bool hasPostLineBreak) :
    SelectorComponent(
      pstate, hasPostLineBreak),
    Vectorized(std::move(selectors)),
    withExplicitParent_(false)
  {}

  CompoundSelector::CompoundSelector(
    const CompoundSelector* ptr,
    bool childless) :
    SelectorComponent(ptr),
    Vectorized(ptr, childless),
    withExplicitParent_(ptr->withExplicitParent())
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

  bool CompoundSelector::hasAnyExplicitParent() const
  {
    if (withExplicitParent()) return true;
    // ToDo: dart sass has another check?
    // if (front->isaNameSpaceSelector()) {
    //  if (front->ns() != "") return false;
    // }
    for (const SimpleSelectorObj& s : elements()) {
      if (s && s->hasAnyExplicitParent()) return true;
    }
    return false;
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

  bool SelectorList::hasExplicitParent() const
  {
    for (const auto& s : elements()) {
      if (s && s->hasExplicitParent()) return true;
    }
    return false;
  }

  Value* SelectorList::toValue() const
  {
    ListObj list = SASS_MEMORY_NEW(List,
      pstate(), {}, SASS_COMMA);
    list->elements().reserve(size());
    for (ComplexSelector* complex : elements()) {
      list->append(complex->toList());
    }
    if (list->size()) return list.detach();
    return SASS_MEMORY_NEW(Null, pstate());
  }

  // Wrap the compound selector with a complex selector
  ComplexSelector* SelectorComponent::wrapInComplex()
  {
    return SASS_MEMORY_NEW(ComplexSelector, pstate(), { this });
  }

  /////////////////////////////////////////////////////////////////////////
  // Below are the resolveParentSelectors implementations
  /////////////////////////////////////////////////////////////////////////

  sass::vector<ComplexSelectorObj>
    CompoundSelector::resolveParentSelectors(
      SelectorList* parent, BackTraces& traces, bool implicit_parent)
  {

    sass::vector<ComplexSelectorObj> rv;

    if (parent) {
      for (SimpleSelectorObj simple : elements()) {
        if (PseudoSelector * pseudo = simple->isaPseudoSelector()) {
          if (SelectorList* sel = pseudo->selector()) {
            pseudo->selector(sel->resolveParentSelectors(
              parent, traces, implicit_parent));
          }
        }
      }
    }

    // Mix with parents from stack
    // Equivalent to dart-sass parent selector tail
    if (withExplicitParent()) {
      SASS_ASSERT(parent != nullptr, "Parent must be defined");
      for (auto complex : parent->elements()) {
        // The parent complex selector has a compound selector
        if (CompoundSelector* tail = complex->last()->isaCompoundSelector()) {
          // Create copies to alter them
          tail = SASS_MEMORY_COPY(tail);
          complex = SASS_MEMORY_COPY(complex);

          // Check if we can merge front with back
          if (size() > 0 && tail->size() > 0) {
            SimpleSelector* front = first();
            auto simple_back = tail->last();
            auto simple_front = front->isaTypeSelector();
            // If they are type/simple selectors ...
            if (simple_front && simple_back) {
              // ... we can combine the names into one
              simple_back = SASS_MEMORY_COPY(simple_back);
              auto name = simple_back->name();
              name += simple_front->name();
              simple_back->name(name);
              // Replace with modified simple selector
              tail->elements().back() = simple_back;
              // Append rest of selector components
              tail->elements().insert(tail->end(),
                begin() + 1, end());
            }
            else {
              // Append us to parent
              tail->concat(this);
            }
          }
          else {
            // Append us to parent
            tail->concat(this);
          }
          // Reset the parent selector tail with
          // the combination of parent plus ourself
          complex->elements().back() = tail;
          // Append to results
          rv.emplace_back(complex);
        }
        // SelectorCombinator
        else {
          // Can't insert parent that ends with a combinator
          // where the parent selector is followed by something
          callStackFrame frame(traces, complex->last()->pstate());
          if (size() > 0) { throw Exception::InvalidParent(parent, traces, this); }
          // Just append ourself to results
          rv.emplace_back(wrapInComplex());
        }
      }

    }
    // No parent
    else {
      // Wrap and append compound selector
      rv.emplace_back(wrapInComplex());
    }

    return rv;

  }
  // EO CompoundSelector::resolveParentSelectors

  List* ComplexSelector::toList() const
  {
    ListObj list = SASS_MEMORY_NEW(List,
      pstate(), {}, SASS_SPACE);
    for (SelectorComponent* component : elements()) {
      SassOutputOptionsCpp out({
        SASS_STYLE_TO_CSS,
        SassDefaultPrecision });
      Cssize inspect(out, false);
      component->accept(&inspect);
      list->append(SASS_MEMORY_NEW(String,
        pstate(), inspect.get_buffer()));
    }
    return list.detach();
  }

  sass::vector<ComplexSelectorObj> ComplexSelector::resolveParentSelectors(
    SelectorList* parent, BackTraces& traces, bool implicit_parent)
  {

    if (hasExplicitParent() && !parent) {
      throw Exception::TopLevelParent(traces, pstate());
    }

    sass::vector<sass::vector<ComplexSelectorObj>> selectors;

    // Check if selector should implicit get a parent
    if (!chroots() && !hasExplicitParent()) {
      // Check if we should never connect to parent
      if (!implicit_parent) { return { this }; }
      // Otherwise add parent selectors at the beginning
      if (parent) { selectors.emplace_back(parent->elements()); }
    }

    // Loop all items from the complex selector
    for (SelectorComponent* component : this->elements()) {
      if (CompoundSelector* compound = component->isaCompoundSelector()) {
        sass::vector<ComplexSelectorObj> complexes =
          compound->resolveParentSelectors(parent, traces, implicit_parent);
        // for (auto sel : complexes) { sel->hasPreLineFeed(hasPreLineFeed()); }
        if (complexes.size() > 0) selectors.emplace_back(complexes);
      }
      else {
        // component->hasPreLineFeed(hasPreLineFeed());
        selectors.push_back({ component->wrapInComplex() });
      }
    }

    // Permutate through all paths
    selectors = permutateAlt(selectors);

    // Create final selectors from path permutations
    sass::vector<ComplexSelectorObj> resolved;
    for (sass::vector<ComplexSelectorObj>& items : selectors) {
      if (items.empty()) continue;
      ComplexSelectorObj first = SASS_MEMORY_COPY(items[0]);
      // ToDo: this seems suspicious, why this logic?
      if (hasPreLineFeed() && !hasExplicitParent()) {
        first->hasPreLineFeed(true);
      }
      // ToDo: remove once we know how to handle line feeds
      // ToDo: currently a mash-up between ruby and dart sass
      // if (has_real_parent_ref()) first->has_line_feed(false);
      // first->has_line_break(first->has_line_break() || has_line_break());
      first->chroots(true); // has been resolved by now
      for (size_t i = 1; i < items.size(); i += 1) {
        if (items[i]->hasPreLineFeed()) {
          first->hasPreLineFeed(true);
        }
        first->concat(items[i]);
      }
      resolved.emplace_back(first);
    }

    return resolved;
  }
  // EO ComplexSelector::resolveParentSelectors

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
