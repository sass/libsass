#include "ast_css.hpp"

#include "ast_selectors.hpp"

#include "css_invisible.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssNode::CssNode(
    const SourceSpan& pstate) :
    AstNode(pstate)
  {}

  CssNode::CssNode(const CssNode* ptr) :
    AstNode(ptr)
  {}

  bool CssNode::isInvisible() const
  {
    IsCssInvisibleVisitor visitor(true, false);
    return const_cast<CssNode*>(this)->accept(&visitor);
  }

  bool CssNode::isInvisibleCss() const
  {
    IsCssInvisibleVisitor visitor(true, false);
    return const_cast<CssNode*>(this)->accept(&visitor);
  }

  bool CssNode::isInvisibleHidingComments() const
  {
    IsCssInvisibleVisitor visitor(true, true);
    return const_cast<CssNode*>(this)->accept(&visitor);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssParentNode::CssParentNode(
    const SourceSpan& pstate,
    CssParentNode* parent,
    CssNodeVector&& children) :
    CssNode(pstate),
    Vectorized(std::move(children)),
    parent_(parent)
  {}
  
  CssParentNode::CssParentNode(
    const CssParentNode* ptr,
    bool childless) :
    CssNode(ptr),
    Vectorized(ptr, childless),
    parent_(ptr->parent_)
  {}

  // Adds [node] as a child of the given [parent]. The parent
  // is copied unless it's the latter most child of its parent.
  void CssParentNode::addChildAt(CssParentNode* child, bool outOfOrder)
  {
    // Check if we have a valid parent
    if (outOfOrder && parent() != nullptr) {
      // Check that parent is visible css
      // if (!parent()->isInvisibleCss()) {
        // Skip if we are last item in parent
        if (parent()->last() != this) {
          // Get iterator to following siblings
          auto it = parent()->begin();
          // Search ourself inside parent
          while (it != parent()->end()) {
            if (it->ptr() == this) break;
            it += 1;
          }
          // Search for first visible sibling
          while (++it != parent()->end()) {
            // Special context for invisibility!
            // dart calls this out to the parent
            const CssNode* sibling = *it;
            if (!sibling->isInvisibleCss()) {
              // Retain and append copy of parent
              auto copy = SASS_MEMORY_RESECT(this);
              parent()->addChildAt(copy, false);
              copy->elements_.push_back(child);
              child->parent(copy);
              return;
            }
          }
        }
      // }
    }
    // Add child to parent
    child->parent(this);
    append(child);
  }
  // EO addChildAt

  bool CssParentNode::isInvisibleCss() const
  {
    for (auto child : elements()) {
      if (!child->isInvisibleCss()) {
        return false;
      }
    }
    return true;
  }

  // CssNode* CssParentNode::produce() const {
  //     CssNodeVector copy;
  //     for (CssNode* child : elements_) {
  //         copy.emplace_back(child->produce());
  //     }
  //     return SASS_MEMORY_NEW(CssParentNode,
  //         pstate_, parent_, std::move(copy));
  // }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssRoot::CssRoot(
    const SourceSpan& pstate,
    CssNodeVector&& children) :
    CssParentNode(
      pstate, nullptr,
      std::move(children))
  {}

  CssRoot::CssRoot(
    const CssRoot* ptr,
    bool childless) :
    CssParentNode(
      ptr, childless)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssString::CssString(
    const SourceSpan& pstate,
    const sass::string& text) :
    AstNode(pstate),
    text_(text)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssStringList::CssStringList(
    const SourceSpan& pstate,
    StringVector&& texts) :
    AstNode(pstate),
    texts_(std::move(texts))
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssAtRule::CssAtRule(
    const SourceSpan& pstate,
    CssParentNode* parent,
    CssString* name,
    CssString* value,
    bool isChildless,
    CssNodeVector&& children) :
    CssParentNode(
      pstate, parent,
      std::move(children)),
    name_(name),
    value_(value),
    isChildless_(isChildless)
  {}

  CssAtRule::CssAtRule(
    const CssAtRule* ptr,
    bool childless) :
    CssParentNode(
      ptr, childless),
    name_(ptr->name_),
    value_(ptr->value_),
    isChildless_(ptr->isChildless_)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssComment::CssComment(
    const SourceSpan& pstate,
    const sass::string& text,
    bool preserve) :
    CssNode(pstate),
    text_(text),
    isPreserved_(preserve)
  {}

  CssComment::CssComment(
    const CssComment* ptr) :
    CssNode(ptr),
    text_(ptr->text_),
    isPreserved_(ptr->isPreserved_)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssDeclaration::CssDeclaration(
    const SourceSpan& pstate,
    CssString* name,
    Value* value,
    bool is_custom_property) :
    CssNode(pstate),
    name_(name),
    value_(value),
    is_custom_property_(is_custom_property)
  {}

  CssDeclaration::CssDeclaration(
    const CssDeclaration* ptr) :
    CssNode(ptr),
    name_(ptr->name_),
    value_(ptr->value_),
    is_custom_property_(ptr->is_custom_property_)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  CssImport::CssImport(
    const SourceSpan& pstate,
    CssString* url,
    CssString* modifiers) :
    CssNode(pstate),
    url_(url),
    modifiers_(modifiers),
    outOfOrder_(false)
  {}

  // Copy constructor
  CssImport::CssImport(
    const CssImport* ptr) :
    CssNode(ptr),
    url_(ptr->url_),
    modifiers_(ptr->modifiers_),
    outOfOrder_(ptr->outOfOrder_)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A block within a `@keyframes` rule.
  // For example, `10% {opacity: 0.5}`.
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  CssKeyframeBlock::CssKeyframeBlock(
    const SourceSpan& pstate,
    CssParentNode* parent,
    CssStringList* selector,
    CssNodeVector&& children) :
    CssParentNode(
      pstate, parent,
      std::move(children)),
    selector_(selector)
  {}

  // Copy constructor
  CssKeyframeBlock::CssKeyframeBlock(
    const CssKeyframeBlock* ptr,
    bool childless) :
    CssParentNode(
      ptr, childless),
    selector_(ptr->selector_)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssStyleRule::CssStyleRule(
    const SourceSpan& pstate,
    CssParentNode* parent,
    SelectorList* selector,
    CssNodeVector&& children) :
    CssParentNode(
      pstate, parent,
      std::move(children)),
    selector_(selector),
    original98_(selector ? selector->produce() : nullptr)
  {}

  CssStyleRule::CssStyleRule(
    const CssStyleRule* ptr,
    bool childless) :
    CssParentNode(
      ptr, childless),
    selector_(ptr->selector_),
    original98_(ptr->original98_)
  {}

  /////////////////////////////////////////////////////////////////////////

  bool CssStyleRule::isInvisibleCss() const
  {
    if (const SelectorList* sl = selector()) {
      if (sl->isInvisible()) return true;
    }
    for (const CssNode* item : elements()) {
      if (!item->isInvisibleCss()) {
        return false;
      }
    }
    return true;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssSupportsRule::CssSupportsRule(
    const SourceSpan& pstate,
    CssParentNode* parent,
    ValueObj condition,
    CssNodeVector&& children) :
    CssParentNode(
      pstate, parent,
      std::move(children)),
    condition_(condition)
  {}

  CssSupportsRule::CssSupportsRule(
    const CssSupportsRule* ptr,
    bool childless) :
    CssParentNode(
      ptr, childless),
    condition_(ptr->condition_)
  {}

  /////////////////////////////////////////////////////////////////////////
  // A plain CSS `@media` rule after it has been evaluated.
  /////////////////////////////////////////////////////////////////////////

  // Value constructor
  CssMediaRule::CssMediaRule(
    const SourceSpan& pstate,
    CssParentNode* parent,
    const CssMediaQueryVector& queries,
    CssNodeVector&& children) :
    CssParentNode(
      pstate, parent,
      std::move(children)),
    queries_(queries)
  {}

  // Copy constructor
  CssMediaRule::CssMediaRule(
    const CssMediaRule* ptr,
    bool childless) :
    CssParentNode(
      ptr, childless),
    queries_(ptr->queries_)
  {}

  // Used by Extension::assertCompatibleMediaContext
  bool CssMediaRule::operator== (const CssMediaRule& rhs) const {
    return queries_ == rhs.queries_;
  }
  // EO operator==

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  CssMediaQuery::CssMediaQuery(
    const SourceSpan& pstate,
    const sass::string& type,
    const sass::string& modifier,
    const StringVector& features) :
    AstNode(pstate),
    type_(type),
    modifier_(modifier),
    conjunction_(true),
    features_(features)
  {}

  CssMediaQuery::CssMediaQuery(
    const SourceSpan& pstate,
    sass::string&& type,
    sass::string&& modifier,
    StringVector&& features) :
    AstNode(pstate),
    type_(std::move(type)),
    modifier_(std::move(modifier)),
    conjunction_(true),
    features_(std::move(features))
  {}

  CssMediaQuery::CssMediaQuery(
    const SourceSpan& pstate,
    StringVector && conditions,
    bool conjunction) :
    AstNode(pstate),
    conjunction_(conjunction),
    features_(std::move(conditions))
  {}

  // Used by Extension::assertCompatibleMediaContext
  bool CssMediaQuery::operator==(const CssMediaQuery& rhs) const
  {
    return type_ == rhs.type_
      && modifier_ == rhs.modifier_
      && features_ == rhs.features_;
  }
  // EO operator==

  // Implemented after dart-sass (maybe move to other class?)
  CssMediaQuery* CssMediaQuery::merge(CssMediaQuery* other)
  {

    // Import namespace locally
    using namespace StringUtils;

    // Get a few references from both objects
    const sass::string& thisType(this->type());
    const sass::string& otherType(other->type());
    const sass::string& thisModifier(this->modifier());
    const sass::string& otherModifier(other->modifier());
    const StringVector& thisFeatures(this->features());
    const StringVector& otherFeatures(other->features());

    // Check for the most simplistic case first
    if (thisType.empty() && otherType.empty()) {
      StringVector features;
      features.reserve(thisFeatures.size() + otherFeatures.size());
      features.insert(features.end(), thisFeatures.begin(), thisFeatures.end());
      features.insert(features.end(), otherFeatures.begin(), otherFeatures.end());
      return SASS_MEMORY_NEW(CssMediaQuery, pstate(),
        "", "", std::move(features));
    }

    // bool typesAreEqual(equalsIgnoreCase(thisType, otherType));
    bool thisMatchesAll(this->matchesAllTypes());
    bool otherMatchesAll(other->matchesAllTypes());
    bool thisModifierIsNot(equalsIgnoreCase(thisModifier, "not", 3));
    bool otherModifierIsNot(equalsIgnoreCase(otherModifier, "not", 3));

    // The queries have different "not" modifier
    if (thisModifierIsNot != otherModifierIsNot) {
      // If types are equal, we can merge some cases
      if (equalsIgnoreCase(thisType, otherType)) {
        // Sort arrays into shorter and bigger one
        const StringVector& negativeFeatures = thisModifierIsNot ? thisFeatures : otherFeatures;
        const StringVector& positiveFeatures = thisModifierIsNot ? otherFeatures : thisFeatures;
        // If the negative features are a subset of the positive features, the
        // query is empty. For example, `not screen and (color)` has no
        // intersection with `screen and (color) and (grid)`.
        // However, `not screen and (color)` *does* intersect with `screen and
        // (grid)`, because it means `not (screen and (color))` and so it allows
        // a screen with no color but with a grid.
        if (listIsSubsetOrEqual(negativeFeatures, positiveFeatures)) {
          return SASS_MEMORY_NEW(CssMediaQuery, pstate(), "");
        }
        // Otherwise we can't merge them
        return nullptr;
      }
      // We established that types differ
      // Check if one matches everything
      if (thisMatchesAll) return nullptr;
      if (otherMatchesAll) return nullptr;
      // Check which modifier was "not"
      return thisModifierIsNot ? other : this;
    }
    // Our modifier equals "not"
    else if (thisModifierIsNot) {
      // CSS has no way of representing "neither screen nor print".
      if (!equalsIgnoreCase(thisType, otherType)) return nullptr;
      // Sort arrays into shorter and bigger one
      bool thisIsBigger(thisFeatures.size() > otherFeatures.size());
      const StringVector& moreFeatures = thisIsBigger ? thisFeatures : otherFeatures;
      const StringVector& fewerFeatures = thisIsBigger ? otherFeatures : thisFeatures;
      // If one set of features is a superset of the other,
      // use those features because they're strictly narrower.
      if (listIsSubsetOrEqual(fewerFeatures, moreFeatures)) {
        // Ignore the lesser features (included in other)
        return SASS_MEMORY_NEW(CssMediaQuery, pstate(),
          thisType, thisModifier, moreFeatures);
      }
      // Otherwise, there's no way to
      // represent the intersection.
      return nullptr;
    }

    // Check if types are not the same
    if (!equalsIgnoreCase(thisType, otherType)) {
      // Check that nothing has an "all" modifier
      if (!thisMatchesAll && !otherMatchesAll) {
        return SASS_MEMORY_NEW(CssMediaQuery, pstate(), "");
      }
    }

    // Concatenate both features
    StringVector features;
    features.reserve(thisFeatures.size() + otherFeatures.size());
    features.insert(features.end(), thisFeatures.begin(), thisFeatures.end());
    features.insert(features.end(), otherFeatures.begin(), otherFeatures.end());

    // Check if we should return other query
    if (this->matchesAllTypes() && !(otherMatchesAll && thisType.empty())) {
      return SASS_MEMORY_NEW(CssMediaQuery, pstate(),
        otherType, otherModifier, std::move(features));
    }

    // Return same query with concatenated features
    return SASS_MEMORY_NEW(CssMediaQuery, pstate(),
      thisType, thisModifier, std::move(features));
  }
  // EO merge

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
