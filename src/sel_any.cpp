#include "sel_any.hpp"

#include "ast_selectors.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  bool AnySelectorVisitor::visitAttributeSelector(AttributeSelector* attribute)
  {
    return false;
  }

  bool AnySelectorVisitor::visitClassSelector(ClassSelector* klass)
  {
    return false;
  }

  bool AnySelectorVisitor::visitComplexSelector(ComplexSelector* complex)
  {
    for (auto cmpd : complex->elements()) {
      if (cmpd->selector() == nullptr) continue;
      if (cmpd->selector()->accept(this)) return true;
    }
    return false;
  }

  bool AnySelectorVisitor::visitCompoundSelector(CompoundSelector* compound)
  {
    for (auto comp : compound->elements())
      if (comp->accept(this)) return true;
    return false;
  }

  bool AnySelectorVisitor::visitIDSelector(IDSelector* id)
  {
    return false;
  }

  bool AnySelectorVisitor::visitPlaceholderSelector(PlaceholderSelector* placeholder)
  {
    return false;
  }

  bool AnySelectorVisitor::visitPseudoSelector(PseudoSelector* pseudo)
  {
    if (pseudo->selector().isNull()) return false;
    return pseudo->selector()->accept(this);
  }

  bool AnySelectorVisitor::visitSelectorList(SelectorList* list)
  {
    for (auto cplx : list->elements())
      if (cplx->accept(this)) return true;
    return false;
  }

  bool AnySelectorVisitor::visitTypeSelector(TypeSelector* type)
  {
    return false;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

