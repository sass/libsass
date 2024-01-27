#include "sel_useless.hpp"

#include "ast_selectors.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool IsUselessVisitor::visitComplexSelector(ComplexSelector* complex)
  {
    if (complex->leadingCombinators().size() > 1) return true;
    for (auto& component : complex->elements()) {
      if (component->combinators().size() > 1) return true;
      if (component->selector()->accept(this)) return true;
    }
    return false;
  }

  bool IsUselessVisitor::visitPseudoSelector(PseudoSelector* pseudo)
  {
    return pseudo->isBogusLenient();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

