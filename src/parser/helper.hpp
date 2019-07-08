#ifndef SASS_PARSER_HELPER_H
#define SASS_PARSER_HELPER_H

namespace Sass {

  namespace ParserHelper {


    // Returns whether [character] is an ASCII newline.
    inline bool isNewline(unsigned char character)
    {
      return character == 0x0A
        || character == 0x0C
        || character == 0x0D;
    }

    // Returns whether [character] is a space or a tab character.
    inline bool isSpaceOrTab(unsigned char character) {
      return character == 0x20
        || character == 0x09;
    }

    // Returns whether [character] is an ASCII whitespace character.
    inline bool isWhitespace(unsigned char character) {
      return isSpaceOrTab(character)
        || isNewline(character);
    }

    // Returns whether [character] is a letter or number.
    inline bool isAlphanumeric(unsigned char character) {
      return Util::ascii_isalnum(character);
    }

    // Returns whether [character] is a letter.
    inline bool isAlphabetic(unsigned char character) {
      return Util::ascii_isalpha(character);
    }

    // Returns whether [character] is a number.
    inline bool isDigit(unsigned char character) {
      return Util::ascii_isdigit(character);
    }

    // Returns whether [character] is legal as the start of a Sass identifier.
    inline bool isNameStart(uint32_t character) {
      return character == '_'
        || isAlphabetic(character)
        || character >= 0x0080;
    }

    // Returns whether [character] is legal in the body of a Sass identifier.
    inline bool isName(unsigned char character) {
      return isNameStart(character)
        || isDigit(character)
        || character == '-';
    }

    // Returns whether [character] is a hexadeicmal digit.
    inline bool isHex(unsigned char character) {
      return Util::ascii_isxdigit(character);
    }

  }

}

#endif
