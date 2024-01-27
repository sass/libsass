/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_KEYFRAME_SELECTOR_HPP
#define SASS_PARSER_KEYFRAME_SELECTOR_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // A parser for `@keyframes` block selectors.
  class KeyframeSelectorParser final : public Parser
  {
  public:

    KeyframeSelectorParser(
      Compiler& context,
      SourceDataObj source) :
      Parser(context, source)
    {}

    StringVector parse();

    sass::string readPercentage();

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
