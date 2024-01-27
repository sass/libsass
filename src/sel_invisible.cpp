#include "sel_invisible.hpp"

#include "ast_selectors.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  IsInvisibleVisitor::IsInvisibleVisitor(
    bool includeBogus):
    includeBogus(includeBogus)
  { }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool IsInvisibleVisitor::visitSelectorList(SelectorList* list)
  {
    for (const auto& complex : list->elements()) {
      if (!visitComplexSelector(complex)) return false;
    }
    return true;
  }

  bool IsInvisibleVisitor::visitComplexSelector(ComplexSelector* complex)
  {
    bool asd = AnySelectorVisitor::visitComplexSelector(complex);
    if (asd) {
      //std::cerr << "base is invisible\n";
    }
    return asd ||
      (includeBogus && complex->isBogusOtherThanLeadingCombinator());
  }

  bool IsInvisibleVisitor::visitPlaceholderSelector(PlaceholderSelector* placeholder)
  {
    return true;
  }

  bool IsInvisibleVisitor::visitPseudoSelector(PseudoSelector* pseudo)
  {
    //std::cerr << "Visit Rule " << includeBogus << "\n";
    if (const auto& selector = pseudo->selector()) {
      if (pseudo->name() != "not") return selector->accept(this); 
      else return includeBogus && selector->isBogusLenient();
    }
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

