#ifndef SASS_INSPECT_HPP
#define SASS_INSPECT_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "emitter.hpp"
#include "color_maps.hpp"
#include "visitor_css.hpp"
#include "visitor_value.hpp"
#include "visitor_selector.hpp"

namespace Sass {
  class Context;

  // public SelectorVisitor<void>

  // Inspect does roughly the same as serialize.dart
  class Inspect :
    public SelectorVisitor<void>,
    public ValueVisitor<void>,
    public CssVisitor<void>,
    public Emitter {

  public:

    // Whether quoted strings should be emitted with quotes.
    bool quotes;

    // So far this only influences how list are rendered.
    // If the separator is known to be comma, we append
    // a trailing comma for lists with a single item.
    bool inspect;

    // We should probably pass an emitter, so we can switch implementation?
    Inspect(struct SassOutputOptionsCpp& opt, bool srcmap_enabled);
    Inspect(Logger& logger, struct SassOutputOptionsCpp& opt, bool srcmap_enabled);

    void visitBlockStatements(CssNodeVector children);
    
    void renderQuotedString(const sass::string& text, uint8_t quotes = 0);
    void renderUnquotedString(const sass::string& text);

    /////////////////////////////////////////////////////////////////////////
    // Implement Selector Visitors
    /////////////////////////////////////////////////////////////////////////

    virtual void visitAttributeSelector(AttributeSelector* sel) override;
    virtual void visitClassSelector(ClassSelector* sel) override;
    virtual void visitComplexSelector(ComplexSelector* sel) override;
    virtual void visitCompoundSelector(CompoundSelector* sel) override;
    virtual void visitIDSelector(IDSelector* sel) override;
    virtual void visitPlaceholderSelector(PlaceholderSelector* sel) override;
    virtual void visitPseudoSelector(PseudoSelector* sel) override;
    virtual void visitSelectorCombinator(SelectorCombinator* sel) override; // LibSass only
    virtual void visitSelectorList(SelectorList* sel) override;
    virtual void visitTypeSelector(TypeSelector* sel) override;

    /////////////////////////////////////////////////////////////////////////
    // Implement Value Visitors
    /////////////////////////////////////////////////////////////////////////

    virtual void visitBoolean(Boolean* value) override;
    virtual void visitColor(Color* value) override;
    virtual void visitFunction(Function* value) override;
    virtual void visitList(List* value) override;
    virtual void visitMap(Map* value) override;
    virtual void visitNull(Null* value) override;
    virtual void visitNumber(Number* value) override;
    virtual void visitString(String* value) override;

    /////////////////////////////////////////////////////////////////////////
    // Implement CSS Visitors
    /////////////////////////////////////////////////////////////////////////

    virtual void visitCssAtRule(CssAtRule* css) override;
    virtual void visitCssComment(CssComment* css) override;
    virtual void visitCssDeclaration(CssDeclaration* css) override;
    virtual void visitCssImport(CssImport* css) override;
    virtual void visitCssKeyframeBlock(CssKeyframeBlock* css) override;
    virtual void visitCssMediaRule(CssMediaRule* css) override;
    virtual void visitCssRoot(CssRoot* css) override; // LibSass only
    virtual void visitCssStyleRule(CssStyleRule* css) override;
    virtual void visitCssSupportsRule(CssSupportsRule* css) override;

    /////////////////////////////////////////////////////////////////////////
    // Not part of visitors (used internally as entry points)
    /////////////////////////////////////////////////////////////////////////

    virtual void acceptCssString(CssString*);
    virtual void acceptCssMediaQuery(CssMediaQuery*);
    virtual void acceptInterpolation(Interpolation*);
    virtual void acceptNameSpaceSelector(NameSpaceSelector*);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    void _writeMapElement(Interpolant* value);

  };

}
#endif
