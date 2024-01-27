/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "visitor_is_calc_safe.hpp"

#include "ast_expressions.hpp"

#include <sass/values.h>

namespace Sass {

  bool isMathOperator(SassOperator op)
  {
    if (op == MUL) return true;
    if (op == DIV) return true;
    if (op == ADD) return true;
    if (op == SUB) return true;
    return false;
  }

  bool SelectorExpression::isCalcSafe() {
    return false;
  }

  bool BinaryOpExpression::isCalcSafe()
  {
    if (isMathOperator(operand())) {
      return left()->isCalcSafe()
        || right()->isCalcSafe();
    }
    return false;
  }

  bool ListExpression::isCalcSafe()
  {
    if (separator() != SASS_SPACE) return false;
    if (hasBrackets() == true) return false;
    if (size() < 2) return false;
    for(auto asd : items()) {
      if (!asd->isCalcSafe())
        return false;
    }
    return true;
  }

  bool ParenthesizedExpression::isCalcSafe()
  {
    return expression()->isCalcSafe();
  }

  bool StringExpression::isCalcSafe()
  {
    // auto str = text()->getInitialPlain();
    // Requires a bit more testings
    return true;
  }

}
