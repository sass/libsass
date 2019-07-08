#ifndef SASS_AST_CSS_HPP
#define SASS_AST_CSS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_nodes.hpp"
#include "visitor_css.hpp"
// #include "ast_def_macros.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssNode : public AstNode,
    public CssVisitable<void>
  {

  private:

    // Whether this was generated from the last node in a
    // nested Sass tree that got flattened during evaluation.
    // ADD_CONSTREF(bool, isGroupEnd);

  public:

    // Value constructor
    CssNode(const SourceSpan& pstate);

    // Copy constructor
    CssNode(const CssNode* ptr);

    // Virtual destructor
    // virtual ~CssNode() {}

    // Needed here to avoid ambiguity from base-classes!??
    virtual void accept(CssVisitor<void>* visitor) override = 0;

    virtual bool isInvisibleCss() const { return false; }

    // Returns the at-rule name for [node], or `null` if it's not an at-rule.
    virtual const sass::string& getAtRuleName() const { return Strings::empty; }

    // size_t tabs() const { return 0; }
    // void tabs(size_t tabs) const { }

    // Declare up-casting methods
    DECLARE_ISA_CASTER(CssMediaRule);
    DECLARE_ISA_CASTER(CssStyleRule);
    DECLARE_ISA_CASTER(CssSupportsRule);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssParentNode : public CssNode,
    public Vectorized<CssNode>
  {
  private:

    // This must be a pointer to avoid circular references
    // Means it has a possibility of being a dangling pointer
    ADD_PROPERTY(CssParentNode*, parent);

  public:

    // Value constructor
    CssParentNode(
      const SourceSpan& pstate,
      CssParentNode* parent,
      CssNodeVector&& children = {});

    // Copy constructor
    CssParentNode(
      const CssParentNode* ptr,
      bool childless = false);

    void addChildAt(CssParentNode* node,
      bool doStrangeStuff = false);

    void addNode(CssNode* element);

    bool isInvisibleCss() const override;


    // Must be implemented in derived classes
    virtual CssParentNode* copy(SASS_MEMORY_ARGS bool childless) const = 0;

    // 

    CssParentNode* bubbleThrough(bool stopAtMediaRule = false)
    {
      return parent_ && bubbles(stopAtMediaRule) ?
        parent_->bubbleThrough(stopAtMediaRule) : this;
    }

    // Media rules are sometimes transparent, sometimes not
    virtual bool bubbles(bool stopAtMediaRule = false) const {
      return false;
    }

    // Declare up-casting methods
    DECLARE_ISA_CASTER(CssAtRule);
  };

  /////////////////////////////////////////////////////////////////////////
  // A plain CSS string
  /////////////////////////////////////////////////////////////////////////

  class CssString final : public AstNode
  {
  private:

    ADD_CONSTREF(sass::string, text);

  public:

    // Value constructor
    CssString(
      const SourceSpan& pstate,
      const sass::string& text);

    bool empty() const { return text_.empty(); }

  };

  /////////////////////////////////////////////////////////////////////////
  // A plain list of CSS strings
  /////////////////////////////////////////////////////////////////////////

  class CssStringList final : public AstNode
  {
  private:

    ADD_CONSTREF(StringVector, texts);

  public:

    // Value constructor
    CssStringList(
      const SourceSpan& pstate,
      StringVector&& texts);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssComment final : public CssNode {
    ADD_CONSTREF(sass::string, text);
    ADD_CONSTREF(bool, isPreserved);
  public:
    CssComment(const SourceSpan& pstate, const sass::string& text, bool preserve = false);
    CssComment(const CssComment* ptr);

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssComment(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssDeclaration final : public CssNode {
    // The name of this declaration.
    ADD_CONSTREF(CssStringObj, name);
    // The value of this declaration.
    ADD_CONSTREF(ValueObj, value);
    ADD_CONSTREF(bool, is_custom_property);
  public:
    CssDeclaration(const CssDeclaration* ptr);
    CssDeclaration(const SourceSpan& pstate, CssString* name, Value* value,
      bool is_custom_property = false);

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssDeclaration(this);
    }
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // A css import is static in nature and
  // can only have one single import url.
  class CssImport final : public CssNode {

    // The url including quotes.
    ADD_CONSTREF(CssStringObj, url);

    // The supports condition attached to this import.
    ADD_CONSTREF(CssStringObj, supports);

    // The media query attached to this import.
    ADD_CONSTREF(CssMediaQueryVector, media);

    // Flag to hoist import to the top.
    ADD_CONSTREF(bool, outOfOrder);

  public:

    // Standard value constructor
    CssImport(
      const SourceSpan& pstate,
      CssString* url = nullptr,
      CssString* supports = nullptr,
      CssMediaQueryVector media = {});

    // Copy constructor
    CssImport(const CssImport* ptr);

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssImport(this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssRoot final : public CssParentNode
  {
  public:

    // Value constructor
    CssRoot(
      const SourceSpan& pstate,
      CssNodeVector&& children = {});

    // Copy constructor
    CssRoot(
      const CssRoot* ptr,
      bool childless = false);

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssRoot(this);
    }

    CssRoot* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CssRoot, this);
    }

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssAtRule final : public CssParentNode
  {
  private:

    ADD_CONSTREF(CssStringObj, name);

    ADD_CONSTREF(CssStringObj, value);

    // Whether the rule has no children and should be emitted
    // without curly braces. This implies `children.isEmpty`,
    // but the reverse is not true - for a rule like `@foo {}`,
    // [children] is empty but [isChildless] is `false`.
    // It means we didn't see any `{` when parsed.
    ADD_CONSTREF(bool, isChildless);

  public:

    // Value constructor
    CssAtRule(
      const SourceSpan& pstate,
      CssParentNode* parent,
      CssString* name,
      CssString* value,
      bool isChildless = false,
      CssNodeVector&& children = {});

    // Copy constructor
    CssAtRule(
      const CssAtRule* ptr,
      bool childless = false);

    bool isInvisibleCss() const override final {
      return false;
    }

    // Returns the at-rule name for [node], or `null` if it's not an at-rule.
    const sass::string& getAtRuleName() const override final {
      return name()->text();
    }


    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssAtRule(this);
    }

    CssAtRule* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CssAtRule, this);
    }

    IMPLEMENT_ISA_CASTER(CssAtRule);
  };

  /////////////////////////////////////////////////////////////////////////
  // A block within a `@keyframes` rule.
  // For example, `10% {opacity: 0.5}`.
  /////////////////////////////////////////////////////////////////////////
  class CssKeyframeBlock final : public CssParentNode
  {
  private:

    // The selector for this block.
    ADD_CONSTREF(CssStringListObj, selector);

  public:

    // Value constructor
    CssKeyframeBlock(
      const SourceSpan& pstate,
      CssParentNode* parent,
      CssStringList* selector,
      CssNodeVector&& children = {});

    // Copy constructor
    CssKeyframeBlock(
      const CssKeyframeBlock* ptr,
      bool childless = false);

    // Return a copy with empty children
    // CssKeyframeBlock* copyWithoutChildren();

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssKeyframeBlock(this);
    }

    CssKeyframeBlock* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CssKeyframeBlock, this);
    }

  };
  // EO CssKeyframeBlock

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssStyleRule final : public CssParentNode
  {
  private:

    ADD_CONSTREF(SelectorListObj, selector);

  public:

    // Value constructor
    CssStyleRule(
      const SourceSpan& pstate,
      CssParentNode* parent,
      SelectorList* selector,
      CssNodeVector&& children = {});

    // Copy constructor
    CssStyleRule(
      const CssStyleRule* ptr,
      bool childless = false);

    // Selector and one child must be visible
    bool isInvisibleCss() const override final;

    // Media rules are sometimes transparent, sometimes not
    bool bubbles(bool stopAtMediaRule) const override final { return true; }

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssStyleRule(this);
    }

    // Declare via macro to allow line/col debugging
    CssStyleRule* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CssStyleRule, this, childless);
    }

    IMPLEMENT_ISA_CASTER(CssStyleRule);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class CssSupportsRule final : public CssParentNode
  {
  private:

    ADD_CONSTREF(ValueObj, condition);

  public:

    // Value constructor
    CssSupportsRule(
      const SourceSpan& pstate,
      CssParentNode* parent,
      ValueObj condition,
      CssNodeVector&& children = {});

    // Copy constructor
    CssSupportsRule(
      const CssSupportsRule* ptr,
      bool childless = false);

    // Returns the at-rule name for [node], or `null` if it's not an at-rule.
    const sass::string& getAtRuleName() const override final { return Strings::supports; }

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssSupportsRule(this);
    }

    // Declare via macro to allow line/col debugging
    CssSupportsRule* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CssSupportsRule, this, childless);
    }

    IMPLEMENT_ISA_CASTER(CssSupportsRule);
  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Media Queries after they have been evaluated
  // Representing the static or resulting css
  class CssMediaQuery final : public AstNode {

    // The media type, for example "screen" or "print".
    // This may be `null`. If so, [features] will not be empty.
    ADD_CONSTREF(sass::string, type);

    // The modifier, probably either "not" or "only".
    // This may be `null` if no modifier is in use.
    ADD_CONSTREF(sass::string, modifier);

    // Feature queries, including parentheses.
    ADD_REF(StringVector, features);

  public:

    // Value copy constructor
    CssMediaQuery(
      const SourceSpan& pstate,
      const sass::string& type,
      const sass::string& modifier,
      const StringVector& features);

    // Value move constructor
    CssMediaQuery(
      const SourceSpan& pstate,
      sass::string&& type,
      sass::string&& modifier = "",
      StringVector&& features = {});

    // Returns true if this query is empty
    // Meaning it has no type and features
    bool empty() const {
      return type_.empty()
        && modifier_.empty()
        && features_.empty();
    }

    // Whether this media query matches all media types.
    bool matchesAllTypes() const {
      return type_.empty() || StringUtils::equalsIgnoreCase(type_, "all", 3);
    }

    // Check if two instances are considered equal
    bool operator== (const CssMediaQuery& rhs) const;

    // Merges this with [other] and adds a query that matches the intersection
    // of both inputs to [result]. Returns false if the result is unrepresentable
    CssMediaQuery* merge(CssMediaQuery* other);

  };

  /////////////////////////////////////////////////////////////////////////
  // A plain CSS `@media` rule after it has been evaluated.
  /////////////////////////////////////////////////////////////////////////
  class CssMediaRule final : public CssParentNode
  {
  private:

    // The queries for this rule (this is never empty).
    ADD_CONSTREF(Vectorized<CssMediaQuery>, queries);

  public:

    // Value constructor
    CssMediaRule(const SourceSpan& pstate,
      CssParentNode* parent,
      const CssMediaQueryVector& queries,
      CssNodeVector&& children = {});

    // Copy constructor
    CssMediaRule(
      const CssMediaRule* ptr,
      bool childless = false);

    // Check if we or any children are invisible
    bool isInvisibleCss() const override final {
      return queries_.empty() ||
        CssParentNode::isInvisibleCss();
    }

    // Media rules are sometimes transparent, sometimes not
    bool bubbles(bool stopAtMediaRule) const override final {
      return stopAtMediaRule == false;
    }

    // Returns the at-rule name for [node], or `null` if it's not an at-rule.
    const sass::string& getAtRuleName() const override final { return Strings::media; }

    // Css visitor and rendering entry function
    void accept(CssVisitor<void>* visitor) override final {
      return visitor->visitCssMediaRule(this);
    }

    // Check if two instances are considered equal
    // Used by Extension::assertCompatibleMediaContext
    bool operator== (const CssMediaRule& rhs) const;

    // Declare via macro to allow line/col debugging
    CssMediaRule* copy(SASS_MEMORY_ARGS bool childless) const override final {
      return SASS_MEMORY_NEW_DBG(CssMediaRule, this, childless);
    }

    IMPLEMENT_ISA_CASTER(CssMediaRule);
  };
  // EO CssMediaRule

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}


#endif
