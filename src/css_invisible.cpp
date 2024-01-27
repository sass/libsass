#include "css_invisible.hpp"

#include "ast_css.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  IsCssInvisibleVisitor::IsCssInvisibleVisitor(
    bool includeBogus, bool includeComments) :
    includeBogus(includeBogus),
    includeComments(includeComments)
  {}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool IsCssInvisibleVisitor::visitCssAtRule(CssAtRule * rule)
  {
    return false;
  }

  bool IsCssInvisibleVisitor::visitCssComment(CssComment* comment)
  {
    return includeComments && !comment->isPreserved();
  }

  bool IsCssInvisibleVisitor::visitCssStyleRule(CssStyleRule* rule)
  {
    //std::cerr << "visit css style rule " << includeBogus << "\n";
    if (includeBogus && rule->selector()->isInvisible()) {
      //std::cerr << "has bogus\n";
      return true;
    }
    if (rule->selector()->isInvisibleOtherThanBogusCombinators()) {
      //std::cerr << "has bogus 2\n";
      return true;
    }
    return EveryCssVisitor::visitCssStyleRule(rule);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

