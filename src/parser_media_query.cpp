/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* Implementations are mostly a direct code-port from dart-sass.             */
/*****************************************************************************/
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

  sass::string MediaQueryParser::readMediaInParens() {
    scanner.expectChar($lparen, "media condition in parentheses");
    sass::string result = "(" + declarationValue() +")";
    scanner.expectChar($rparen);
    return result;
  }


  /// Consumes one or more `<media-in-parens>` expressions separated by
  /// [operator] and returns them.
  sass::vector<sass::string> MediaQueryParser::readMediaLogicSequence2(sass::string op) {
    sass::vector<sass::string> result;
    while (true) {
      result.push_back(readMediaInParens());
      scanWhitespace();
      if (!scanIdentifier(op)) return result;
      expectWhitespace();
    }
  }


  // Consumes a single media query.
  CssMediaQuery* MediaQueryParser::readMediaQuery()
  {

    // This is somewhat duplicated in StylesheetParser.readMediaQuery.

    Offset start(scanner.offset);

    if (scanner.peekChar() == $lparen) {
      StringVector conditions;
      conditions.push_back(readMediaInParens());
      scanWhitespace();

      bool conjunction = true;
      if (scanIdentifier("and")) {
        expectWhitespace();
        auto ands = readMediaLogicSequence2("and");
        std::copy(std::begin(ands), std::end(ands),
          std::back_inserter(conditions));
      }
      else if (scanIdentifier("or")) {
        expectWhitespace();
        conjunction = false;
        auto ors = readMediaLogicSequence2("or");
        std::copy(std::begin(ors), std::end(ors),
          std::back_inserter(conditions));
      }

      return SASS_MEMORY_NEW(CssMediaQuery,
        scanner.rawSpanFrom(start),
        std::move(conditions),
        conjunction);
    }


    sass::string type;
    sass::string modifier;

    auto identifier1 = readIdentifier();
    if (StringUtils::equalsIgnoreCase(identifier1, "not", 3)) {
      expectWhitespace();
      if (!lookingAtIdentifier()) {
        // For example, "@media not (...) {"
        // For example, "@media screen {"

        // return CssMediaQuery.condition(["(not ${_mediaInParens()})"]);
        sass::string str("(not " + readMediaInParens() + ")");
        return SASS_MEMORY_NEW(CssMediaQuery,
          scanner.rawSpanFrom(start), { str });
      }
    }

    scanWhitespace();
    if (!lookingAtIdentifier()) {
      // For example, "@media screen {"
      // return CssMediaQuery.type(identifier1);
      return SASS_MEMORY_NEW(CssMediaQuery,
        scanner.rawSpanFrom(start),
        std::move(identifier1));
    }

    auto identifier2 = readIdentifier();

    if (StringUtils::equalsIgnoreCase(identifier2, "and", 3)) {
      expectWhitespace();
      // For example, "@media screen and ..."
      type = identifier1;
    }
    else {
      scanWhitespace();
      modifier = identifier1;
      type = identifier2;
      if (scanIdentifier("and")) {
        // For example, "@media only screen and ..."
        expectWhitespace();
      }
      else {
        // For example, "@media only screen {"
        // return CssMediaQuery.type(type, modifier: modifier);
        return SASS_MEMORY_NEW(CssMediaQuery,
          scanner.rawSpanFrom(start),
          std::move(type),
          std::move(modifier));

      }
    }

    // We've consumed either `IDENTIFIER "and"` or
    // `IDENTIFIER IDENTIFIER "and"`.

    if (scanIdentifier("not")) {
      // For example, "@media screen and not (...) {"
      expectWhitespace();
      return SASS_MEMORY_NEW(CssMediaQuery,
        scanner.rawSpanFrom(start),
        std::move(type), std::move(modifier),
        { "(not " + readMediaInParens() + ")" });

      //return SASS_MEMORY_NEW(CssMediaQuery,
      //  scanner.rawSpanFrom(start),
      //  std::move(type),
      //  std::move(modifier));

      // return CssMediaQuery.type(type,
      //   modifier: modifier, conditions : ["(not ${_mediaInParens()})"] );
    }

    auto qwe = SASS_MEMORY_NEW(CssMediaQuery,
      scanner.rawSpanFrom(start),
      std::move(type), std::move(modifier),
      std::move(readMediaLogicSequence2("and")));

    return qwe;

    //return CssMediaQuery.type(type,
    //  modifier: modifier, conditions : _mediaLogicSequence("and"));

/*
    // This is somewhat duplicated in StylesheetParser.readMediaQuery.
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
          */

  }
  // EO readMediaQuery

}
