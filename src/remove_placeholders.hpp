/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_REMOVE_PLACEHOLDERS_HPP
#define SASS_REMOVE_PLACEHOLDERS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_css.hpp"
#include "ast_fwd_decl.hpp"
#include "visitor_css.hpp"

namespace Sass {

  class RemovePlaceholders :
    public CssVisitor<void> {

  private:

    // Helper to traverse the tree recursively
    void acceptCssParentNode(CssParentNode*);

    // The main functions to do the cleanup
    void remove_placeholders(SelectorList*);
    void remove_placeholders(SimpleSelector*);
    void remove_placeholders(CompoundSelector*);
    void remove_placeholders(ComplexSelector*);

  public:

    // Do not implement anything for these visitors
    void visitCssComment(CssComment* css) override final {};
    void visitCssDeclaration(CssDeclaration* css) override final {};
    void visitCssImport(CssImport* css) override final {};

    // Move further down into children to remove recursively
    void visitCssAtRule(CssAtRule* css) override final { acceptCssParentNode(css); };
    void visitCssKeyframeBlock(CssKeyframeBlock* css) override final { acceptCssParentNode(css); };
    void visitCssMediaRule(CssMediaRule* css) override final { acceptCssParentNode(css); };
    void visitCssSupportsRule(CssSupportsRule* css) override final { acceptCssParentNode(css); };

    // Cleaning only makes sense on those nodes
    void visitCssRoot(CssRoot*) override final;
    void visitCssStyleRule(CssStyleRule*) override final;

  };

}

#endif
