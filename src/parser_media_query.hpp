/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_MEDIA_QUERY_HPP
#define SASS_PARSER_MEDIA_QUERY_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class MediaQueryParser final : public Parser
  {
  public:

    // Value constructor
    MediaQueryParser(
      Compiler& context,
      SourceDataObj source) :
      Parser(context, source)
    {}

    // Consume multiple media queries delimited by commas.
    CssMediaQueryVector parse();

  private:

    // Consumes a single media query.
    CssMediaQuery* readMediaQuery();

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
