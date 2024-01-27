/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_SEL_INVISIBLE_HPP
#define SASS_SEL_INVISIBLE_HPP

#include "visitor_selector.hpp"

#include "sel_any.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class IsInvisibleVisitor : public AnySelectorVisitor {

    /// Whether to consider selectors with bogus combinators invisible.
    bool includeBogus;

  public:

    IsInvisibleVisitor(bool includeBogus);

    virtual bool visitSelectorList(SelectorList* list) override final;
    virtual bool visitComplexSelector(ComplexSelector* complex) override;
    virtual bool visitPlaceholderSelector(PlaceholderSelector* placeholder) override final;
    virtual bool visitPseudoSelector(PseudoSelector* pseudo) override;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
