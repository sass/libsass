#include "sel_bogus.hpp"

#include "ast_selectors.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool IsBogusVisitor::visitComplexSelector(ComplexSelector* complex)
  {
    //std::cerr << "visit [" << complex->inspect() << "] " << includeLeadingCombinator << "\n";
    const auto& elements = complex->elements();
    if (elements.empty()) {
      //std::cerr << "Case 1\n";
      return !complex->leadingCombinators().empty();
    }
    else if (complex->leadingCombinators().size()
      > (includeLeadingCombinator ? 0UL : 1UL))
    {
      //std::cerr << "Case 2 " << includeLeadingCombinator << "\n";
      return true;
    }
    else if (!elements.back()->combinators().empty()) {
      //std::cerr << "Case 3\n";
      return true;
    }
    else {
      //std::cerr << "Case 4 " << includeLeadingCombinator << "\n";
      for (auto component : elements) {
        if (component->combinators().size() > 1) return true;
        return component->selector()->accept(this);
      }
    }
    return false;
  }

  bool IsBogusVisitor::visitPseudoSelector(PseudoSelector* pseudo)
  {
    auto& selector = pseudo->selector();
    if (selector.isNull()) return false;

    if (pseudo->name() != "has") return selector->isBogusStrict();
    else return selector->isBogusOtherThanLeadingCombinator();
    // return pseudo->selector()->accept(this);
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  const IsBogusVisitor IsBogusVisitorStrict(false);
  const IsBogusVisitor IsBogusVisitorLenient(true);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

