/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
#include "parser_at_root_query.hpp"

#include "character.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  AtRootQuery* AtRootQueryParser::parse()
  {
    Offset start(scanner.offset);
    scanner.expectChar($lparen);
    scanWhitespace();

    bool include = scanIdentifier("with");
    if (!include) expectIdentifier("without",
      "\"with\" or \"without\"");

    scanWhitespace();
    scanner.expectChar($colon);
    scanWhitespace();

    StringSet atRules;
    do {
      sass::string ident(readIdentifier());
      StringUtils::makeLowerCase(ident);
      atRules.insert(ident);
      scanWhitespace();
    }
    while (lookingAtIdentifier());

    scanner.expectChar($rparen);
    scanner.expectDone();

    return SASS_MEMORY_NEW(AtRootQuery,
      scanner.rawSpanFrom(start),
      std::move(atRules),
      include);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
