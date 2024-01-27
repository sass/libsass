#include "css_every.hpp"

#include "ast_css.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool Sass::EveryCssVisitor::visitCssAtRule(CssAtRule* css)
  {
    for (auto& child : css->elements()) {
      if (!child->accept(this)) return false;
    }
    return true;
  }

  bool EveryCssVisitor::visitCssComment(CssComment* css)
  {
    return false;
  }

  bool EveryCssVisitor::visitCssDeclaration(CssDeclaration* css)
  {
    return false;
  }

  bool EveryCssVisitor::visitCssImport(CssImport* css)
  {
    return false;
  }

  bool EveryCssVisitor::visitCssKeyframeBlock(CssKeyframeBlock* css)
  {
    for (auto& child : css->elements()) {
      if (!child->accept(this)) return false;
    }
    return true;
  }

  bool EveryCssVisitor::visitCssMediaRule(CssMediaRule* css)
  {
    for (auto& child : css->elements()) {
      if (!child->accept(this)) return false;
    }
    return true;
  }

  bool EveryCssVisitor::visitCssRoot(CssRoot* css)
  {
    for (auto& child : css->elements()) {
      if (!child->accept(this)) return false;
    }
    return true;
  }

  bool EveryCssVisitor::visitCssStyleRule(CssStyleRule* css)
  {
    for (auto& child : css->elements()) {
      if (!child->accept(this)) return false;
    }
    return true;
  }

  bool EveryCssVisitor::visitCssSupportsRule(CssSupportsRule* css)
  {
    for (auto& child : css->elements()) {
      if (!child->accept(this)) return false;
    }
    return true;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

