/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_STYLESHEET_HPP
#define SASS_STYLESHEET_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "ast_css.hpp"
#include "modules.hpp"
#include "import.hpp"

namespace Sass {

  // parsed stylesheet from loaded resource
  // this should be a `Module` for sass 4.0
  class Root final : public AstNode,
    public Vectorized<Statement>,
    public Module
  {
  public:

    // sass::vector<Root*> upstreams;

    // Import object through which this module was loaded.
    // It also has the input type (css vs sass) attached
    ImportObj import;

    Root(const SourceSpan& pstate, size_t reserve = 0);

    Root(const SourceSpan& pstate, StatementVector&& vec);

    void addExtension(
      const SelectorListObj& extender,
      const SimpleSelectorObj& target,
      const CssMediaRuleObj& mediaQueryContext,
      const ExtendRuleObj& extend,
      bool is_optional);


  };

}

#endif
