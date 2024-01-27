/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CSS_EVERY_HPP
#define SASS_CSS_EVERY_HPP

#include "visitor_css.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class EveryCssVisitor : public CssVisitor<bool> {

  public:

    virtual bool visitCssAtRule(CssAtRule* css);
    virtual bool visitCssComment(CssComment* css);
    virtual bool visitCssDeclaration(CssDeclaration* css);
    virtual bool visitCssImport(CssImport* css);
    virtual bool visitCssKeyframeBlock(CssKeyframeBlock* css);
    virtual bool visitCssMediaRule(CssMediaRule* css);
    virtual bool visitCssRoot(CssRoot* css);
    virtual bool visitCssStyleRule(CssStyleRule* css);
    virtual bool visitCssSupportsRule(CssSupportsRule* css);

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
