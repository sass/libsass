/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CSS_INVISIBLE_HPP
#define SASS_CSS_INVISIBLE_HPP

#include "css_every.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class IsCssInvisibleVisitor : public EveryCssVisitor {

    /// Whether to consider selectors with bogus combinators invisible.
    bool includeBogus;

    /// Whether to consider comments invisible.
    bool includeComments;

  public:

    IsCssInvisibleVisitor(bool includeBogus, bool includeComments);

    virtual bool visitCssAtRule(CssAtRule* rule) override final;
    virtual bool visitCssComment(CssComment* rule) override final;
    virtual bool visitCssStyleRule(CssStyleRule* rule) override final;

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
