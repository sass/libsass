/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
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
    Inspect(const OutputOptions& opt);
    Inspect(Logger& logger, const OutputOptions& opt);

    void visitBlockStatements(CssNodeVector children);
    
    void renderQuotedString(const sass::string& text, uint8_t quotes = 0);
    void renderUnquotedString(const sass::string& text);

    bool _tryPrivateUseCharacter(uint8_t chr);

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
    // virtual void visitSelectorCombinator(SelectorCombinator* sel) override; // LibSass only
    virtual void visitSelectorList(SelectorList* sel) override;
    virtual void visitTypeSelector(TypeSelector* sel) override;

    virtual void visitSelectorComponent(CplxSelComponent* sel); // LibSass only
    virtual void visitSelectorCombinator(SelectorCombinator* sel); // LibSass only

    /////////////////////////////////////////////////////////////////////////
    // Implement Value Visitors
    /////////////////////////////////////////////////////////////////////////

    virtual void visitBoolean(Boolean* value) override;
    virtual void visitColor(Color* value) override;
    virtual void visitFunction(Function* value) override;
    virtual void visitCalculation(Calculation* value) override;
    virtual void visitCalcOperation(CalcOperation* value) override;
    virtual void visitMixin(Mixin* value) override;
    virtual void visitList(List* value) override;
    virtual void visitMap(Map* value) override;
    virtual void visitNull(Null* value) override;
    virtual void visitNumber(Number* value) override;
    virtual void visitString(String* value) override;
    // Private helper for "messy" calc values
    void _writeCalculationValue(AstNode* node, bool wrap = true);
    void _writeCalculationUnits(Units* units);

    /////////////////////////////////////////////////////////////////////////
    // Implement CSS Visitors
    /////////////////////////////////////////////////////////////////////////

    virtual void visitCssAtRule(CssAtRule* css) override;
    virtual void visitCssComment(CssComment* css) override;
    virtual void visitCssDeclaration(CssDeclaration* css) override;
    bool _IsInvisible(CssNode* node);
    virtual void visitCssImport(CssImport* css) override;
    virtual void visitCssKeyframeBlock(CssKeyframeBlock* css) override;
    virtual void visitCssMediaRule(CssMediaRule* css) override;
    virtual void visitCssRoot(CssRoot* css) override; // LibSass only
    virtual void visitCssStyleRule(CssStyleRule* css) override;
    virtual void visitCssSupportsRule(CssSupportsRule* css) override;

    /////////////////////////////////////////////////////////////////////////
    // Not part of visitors (used internally as entry points)
    /////////////////////////////////////////////////////////////////////////

    virtual void acceptCssString(const CssString*);
    virtual void acceptCssMediaQuery(CssMediaQuery*);
    virtual void acceptInterpolation(Interpolation*);
    virtual void acceptNameSpaceSelector(SelectorNS*);

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    void _writeMapElement(Interpolant* value);

    template<typename octet_iterator>
    bool _tryPrivateUseCharacter(const octet_iterator& it, const octet_iterator& end, size_t& offset);

  };

}
#endif
