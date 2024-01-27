/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_SEL_ANY_HPP
#define SASS_SEL_ANY_HPP

#include "visitor_selector.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class AnySelectorVisitor : public SelectorVisitor<bool> {

  public:

    virtual bool visitAttributeSelector(AttributeSelector* attribute) override final;
    virtual bool visitClassSelector(ClassSelector* klass) override final;
    virtual bool visitComplexSelector(ComplexSelector* complex) override;
    virtual bool visitCompoundSelector(CompoundSelector* compound) override final;
    virtual bool visitIDSelector(IDSelector* id) override final;
    virtual bool visitPlaceholderSelector(PlaceholderSelector* placeholder) override;
    virtual bool visitPseudoSelector(PseudoSelector* pseudo) override;
    virtual bool visitSelectorList(SelectorList* list) override;
    virtual bool visitTypeSelector(TypeSelector* type) override final;
    // virtual bool visitSelectorCombinator(SelectorCombinator* combinator) override final;

  };

}

#endif
