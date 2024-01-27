/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "stylesheet.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  Root::Root(const SourceSpan& pstate, size_t reserve)
    : AstNode(pstate), Vectorized<Statement>(reserve), Module(nullptr)
  {}

  Root::Root(const SourceSpan& pstate, StatementVector&& vec)
    : AstNode(pstate), Vectorized<Statement>(std::move(vec)), Module(nullptr)
  {}

  void Root::addExtension(
    const SelectorListObj& extender3,
    const SimpleSelectorObj& target,
    const CssMediaRuleObj& mediaQueryContext,
    const ExtendRuleObj& extend,
    bool is_optional)
  {
//UUU    for (Root* mod : upstream) {
//UUU      mod->addExtension(extend, target, mediaQueryContext, is_optional);
//UUU    }
    if (extender) extender->addExtension(extender3, target, mediaQueryContext, extend, is_optional);
  }


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
