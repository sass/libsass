#include "parser_keyframe_selector.hpp"

#include "character.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Import some namespaces
  using namespace Charcode;
  using namespace Character;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  StringVector KeyframeSelectorParser::parse() {

    StringVector selectors;
    do {
      scanWhitespace();
      if (lookingAtIdentifier()) {
        if (scanIdentifier("from")) {
          selectors.emplace_back("from");
        }
        else {
          expectIdentifier("to",
            "\"to\" or \"from\"");
          selectors.emplace_back("to");
        }
      }
      else {
        selectors.emplace_back(readPercentage());
      }
      scanWhitespace();
    }
    while (scanner.scanChar($comma));

    scanner.expectDone();
    return selectors;
  }

  sass::string KeyframeSelectorParser::readPercentage()
  {

    StringBuffer buffer;
    if (scanner.scanChar($plus)) {
      buffer.writeCharCode($plus);
    }

    uint8_t second = scanner.peekChar();
    if (!isDigit(second) && second != $dot) {
      error("Expected number.",
        scanner.rawSpan());
    }

    while (isDigit(scanner.peekChar())) {
      buffer.writeCharCode(scanner.readChar());
    }

    if (scanner.peekChar() == $dot) {
      buffer.writeCharCode(scanner.readChar());

      while (isDigit(scanner.peekChar())) {
        buffer.writeCharCode(scanner.readChar());
      }
    }

    if (scanIdentifier("e")) {
      buffer.write(scanner.readChar());
      uint8_t next = scanner.peekChar();
      if (next == $plus || next == $minus) buffer.write(scanner.readChar());
      if (!isDigit(scanner.peekChar())) {
        error("Expected digit.",
          scanner.rawSpan());
      }

      while (isDigit(scanner.peekChar())) {
        buffer.writeCharCode(scanner.readChar());
      }
    }

    scanner.expectChar($percent);
    buffer.writeCharCode($percent);
    return buffer.buffer;

  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
