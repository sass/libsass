#include "parser_media_query.hpp"

#include "ast_css.hpp"
#include "charcode.hpp"
#include "character.hpp"

namespace Sass {

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  // Consume multiple media queries delimited by commas.
  CssMediaQueryVector MediaQueryParser::parse()
  {
    CssMediaQueryVector queries;
    do {
      scanWhitespace();
      queries.emplace_back(readMediaQuery());
    } while (scanner.scanChar($comma));
    scanner.expectDone();
    return queries;
  }

  // Consumes a single media query.
  CssMediaQuery* MediaQueryParser::readMediaQuery()
  {
    // This is somewhat duplicated in StylesheetParser.readMediaQuery.
    sass::string type;
    sass::string modifier;
    Offset start(scanner.offset);
    if (scanner.peekChar() != $lparen) {
      sass::string identifier1 = readIdentifier();
      scanWhitespace();

      if (!lookingAtIdentifier()) {
        // For example, "@media screen {"
        return SASS_MEMORY_NEW(CssMediaQuery,
          scanner.rawSpanFrom(start),
          std::move(identifier1));
      }

      sass::string identifier2 = readIdentifier();
      scanWhitespace();

      if (StringUtils::equalsIgnoreCase(identifier2, "and", 3)) {
        // For example, "@media screen and ..."
        type = identifier1;
      }
      else {
        modifier = identifier1;
        type = identifier2;
        if (scanIdentifier("and")) {
          // For example, "@media only screen and ..."
          scanWhitespace();
        }
        else {
          // For example, "@media only screen {"
          return SASS_MEMORY_NEW(CssMediaQuery,
            scanner.rawSpanFrom(start),
            std::move(type),
            std::move(modifier));
        }
      }

    }

    // We've consumed either `IDENTIFIER "and"`, `IDENTIFIER IDENTIFIER "and"`,
    // or no text.

    StringVector features;
    do {
      scanWhitespace();
      scanner.expectChar($lparen);
      auto decl = declarationValue();
      features.emplace_back("(" + decl + ")");
      scanner.expectChar($rparen);
      scanWhitespace();
    } while (scanIdentifier("and"));

    if (type.empty()) {
      return SASS_MEMORY_NEW(CssMediaQuery,
        scanner.rawSpanFrom(start),
        "", "", std::move(features));
    }
    else {
      return SASS_MEMORY_NEW(CssMediaQuery,
        scanner.rawSpanFrom(start),
        std::move(type),
        std::move(modifier),
        std::move(features));
    }

  }
  // EO readMediaQuery

}
