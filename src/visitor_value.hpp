/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_VISITOR_VALUE_HPP
#define SASS_VISITOR_VALUE_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_fwd_decl.hpp"

namespace Sass {

  // An interface for [visitors][] that traverse SassScript values.
  // [visitors]: https://en.wikipedia.org/wiki/Visitor_pattern

  template <typename T>
  class ValueVisitor {
  public:

    virtual T visitBoolean(Boolean* value) = 0;
    virtual T visitColor(Color* value) = 0;
    virtual T visitFunction(Function* value) = 0;
    virtual T visitCalculation(Calculation* value) = 0;
    virtual T visitCalcOperation(CalcOperation* value) = 0;
    virtual T visitMixin(Mixin* value) = 0;
    virtual T visitList(List* value) = 0;
    virtual T visitMap(Map* value) = 0;
    virtual T visitNull(Null* value) = 0;
    virtual T visitNumber(Number* value) = 0;
    virtual T visitString(String* value) = 0;

  };

  template <typename T>
  class ValueVisitable {
  public:
    virtual T accept(ValueVisitor<T>* visitor) = 0;
  };

}

#define DECLARE_VALUE_ACCEPT(T, name)\
  T accept(ValueVisitor<T>* visitor) override final {\
    return visitor->visit##name(this);\
  }\

#endif
