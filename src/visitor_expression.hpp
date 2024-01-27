/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VISITOR_EXPRESSION_HPP
#define SASS_VISITOR_EXPRESSION_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"

namespace Sass {

  // An interface for [visitors][] that traverse SassScript expressions.
  // [visitors]: https://en.wikipedia.org/wiki/Visitor_pattern

  template <typename T>
  class ExpressionVisitor {
  public:

    virtual T visitBinaryOpExpression(BinaryOpExpression*) = 0;
    virtual T visitBooleanExpression(BooleanExpression*) = 0;
    virtual T visitColorExpression(ColorExpression*) = 0;
    virtual T visitItplFnExpression(ItplFnExpression*) = 0;
    virtual T visitFunctionExpression(FunctionExpression*) = 0;
    virtual T visitIfExpression(IfExpression*) = 0;
    virtual T visitListExpression(ListExpression*) = 0;
    virtual T visitMapExpression(MapExpression*) = 0;
    virtual T visitNullExpression(NullExpression*) = 0;
    virtual T visitNumberExpression(NumberExpression*) = 0;
    virtual T visitParenthesizedExpression(ParenthesizedExpression*) = 0;
    virtual T visitSelectorExpression(SelectorExpression*) = 0;
    virtual T visitStringExpression(StringExpression*) = 0;
    virtual T visitSupportsExpression(SupportsExpression*) = 0;
    virtual T visitUnaryOpExpression(UnaryOpExpression*) = 0;
    virtual T visitValueExpression(ValueExpression*) = 0;
    virtual T visitVariableExpression(VariableExpression*) = 0;

  };

  template <typename T>
  class ExpressionVisitable {
  public:
    virtual T accept(ExpressionVisitor<T>* visitor) = 0;
  };

}

#endif
