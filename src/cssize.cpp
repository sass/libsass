#include "cssize.hpp"

#include "charcode.hpp"
#include "character.hpp"
#include "ast_values.hpp"
#include "exceptions.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  void Cssize::visitFunction(Function* f)
  {
    throw Exception::InvalidCssValue({}, *f);
  }

  void Cssize::visitMap(Map* value)
  {
    if (output_style() == SASS_STYLE_TO_CSS) {
      // should be handle in check_expression
      throw Exception::InvalidCssValue({}, *value);
    }
    if (value->empty()) return;

    Inspect::visitMap(value);
  }

  void Cssize::visitList(List* list)
  {
    if (list->empty() && !list->hasBrackets()) {
      throw Exception::InvalidCssValue({}, *list);
    }
    Inspect::visitList(list);
  }

  void Cssize::visitNumber(Number* n)
  {

    if (n->lhsAsSlash() && n->rhsAsSlash()) {
      n->lhsAsSlash()->accept(this);
      append_string("/");
      n->rhsAsSlash()->accept(this);
      return;
    }

    // reduce units
    n->reduce();

    if (opt.output_style == SASS_STYLE_TO_CSS && !n->isValidCssUnit()) {
      // traces.push_back(BackTrace(nr->pstate()));
      // issue_1804
      throw Exception::InvalidCssValue({}, *n);
    }

    Inspect::visitNumber(n);

  }

}

