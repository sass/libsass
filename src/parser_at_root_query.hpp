/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_PARSER_AT_ROOT_QUERY_HPP
#define SASS_PARSER_AT_ROOT_QUERY_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "parser.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  class AtRootQueryParser : public Parser
  {
  public:

    // Value constructor
    AtRootQueryParser(
      Compiler& context,
      SourceDataObj source) :
      Parser(context, source)
    {}

    // Main entry function
    AtRootQuery* parse();

  };

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}

#endif
