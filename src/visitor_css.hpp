/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VISITOR_CSS_HPP
#define SASS_VISITOR_CSS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"

namespace Sass {

  // An interface for [visitors][] that traverse CSS statements.
  // [visitors]: https://en.wikipedia.org/wiki/Visitor_pattern

  template <typename T>
  class CssVisitor {
  public:

    virtual T visitCssAtRule(CssAtRule* css) = 0;
    virtual T visitCssComment(CssComment* css) = 0;
    virtual T visitCssDeclaration(CssDeclaration* css) = 0;
    virtual T visitCssImport(CssImport* css) = 0;
    virtual T visitCssKeyframeBlock(CssKeyframeBlock* css) = 0;
    virtual T visitCssMediaRule(CssMediaRule* css) = 0;
    virtual T visitCssRoot(CssRoot* css) = 0; // LibSass only
    virtual T visitCssStyleRule(CssStyleRule* css) = 0;
    virtual T visitCssSupportsRule(CssSupportsRule* css) = 0;

  };

  template <typename T>
  class CssVisitable {
  public:
    virtual T accept(CssVisitor<T>* visitor) = 0;
  };

}

#define DECLARE_CSS_ACCEPT(T, name)\
  T accept(CssVisitor<T>* visitor) override final {\
    return visitor->visit##name(this);\
  }\

#endif
