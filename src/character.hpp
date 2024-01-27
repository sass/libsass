/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CHARACTER_HPP
#define SASS_CHARACTER_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <bitset>
#include "charcode.hpp"

namespace Sass {

  namespace Character {

    using namespace Charcode;

    // The difference between upper- and lowercase ASCII letters.
    // `0b100000` can be bitwise-ORed with uppercase ASCII letters
    // to get their lowercase equivalents.
    const uint8_t asciiCaseBit = 0x20;

    // The ASCII lookup tables
    extern const std::bitset<256> tblNewline;
    extern const std::bitset<256> tblSpaceOrTab;
    extern const std::bitset<256> tblWhitespace;
    extern const std::bitset<256> tblAlphabetic;
    extern const std::bitset<256> tblDigit;
    extern const std::bitset<256> tblAlphanumeric;
    extern const std::bitset<256> tblHex;

    // skip over 10xxxxxx and 01xxxxxx
    // count ASCII and initial utf8 bytes
    inline bool isCharacter(uint8_t character) {
      // Ignore all `10xxxxxx` chars
      // '0xxxxxxx' are ASCII chars
      // '11xxxxxx' are utf8 starts
      // 64 => initial utf8 byte
      // 128 => regular ASCII char
      return (character & 192) != 128;
      // return (character & (128|64)) != 0;
    }

    // Returns whether [character] is
    // starting a utf8 multi-byte sequence.
    inline bool isUtf8StartByte(uint8_t character) {
      return (character & 192) == 192;
    }

    // Returns whether [character] is
    // part of a utf8 multi-byte sequence.
    inline bool isUtf8Continuation(uint8_t character) {
      return (character & 192) == 128;
    }

    // Returns whether [character] is the
    // beginning of a UTF-16 surrogate pair.
    // bool isUtf8HighSurrogate(uint16_t character) {
    //   return character >= 0xD800 && character <= 0xDBFF;
    // }

    // Returns whether [character] is an ASCII newline.
    inline bool isNewline(uint8_t character) {
      return tblNewline[character];
    }

    // Returns whether [character] is a space or a tab character.
    inline bool isSpaceOrTab(uint8_t character) {
      return tblSpaceOrTab[character];
    }

    // Returns whether [character] is an ASCII whitespace character.
    inline bool isWhitespace(uint8_t character) {
      return tblWhitespace[character];
    }

    // Returns whether [character] is a letter.
    inline bool isAlphabetic(uint8_t character) {
      return tblAlphabetic[character];
    }

    // Returns whether [character] is a number.
    inline bool isDigit(uint8_t character) {
      return tblDigit[character];
    }

    // Returns whether [character] is a letter or number.
    inline bool isAlphanumeric(uint8_t character) {
      return tblAlphanumeric[character];
    }

    // Returns whether [character] is legal as the start of a Sass identifier.
    inline bool isNameStart(uint32_t character) {
      return character == $_
        || character >= 0x0080 // 127
        || tblAlphabetic[character];
    }

    // Returns whether [character] is legal as the start of a Sass identifier.
    inline bool isNameStart(uint8_t character) {
      return character == $_
        || character >= 0x0080 // 127
        || tblAlphabetic[character];
    }

    // Returns whether [character] is legal in the body of a Sass identifier.
    inline bool isName(uint32_t character) {
      return character == $_
        || character == $minus
        || character >= 0x0080 // 127
        || tblAlphanumeric[character];
    }

    // Returns whether [character] is legal in the body of a Sass identifier.
    inline bool isName(uint8_t ascii) {
      return ascii == $_
        || ascii == $minus
        || ascii >= 0x0080 // 127
        || tblAlphanumeric[ascii];
    }

    // Returns whether [character] is a hexadecimal digit.
    inline bool isHex(uint8_t ascii) {
      return tblHex[ascii];
    }

    // Returns whether [character] can start a simple
    // selector other than a type selector.
    inline bool isSimpleSelectorStart(uint8_t character)
    {
      return character == $asterisk
        || character == $lbracket
        || character == $dot
        || character == $hash
        || character == $percent
        || character == $colon;
    }

    // Returns the value of [character] as a hex digit.
    // Assumes that [character] is a hex digit.
    inline uint8_t asHex(uint8_t character)
    {
      // assert(isHex(character));
      if (character <= $9) return character - $0;
      if (character <= $F) return 10 + character - $A;
      return 10 + character - $a;
    }

    // Returns the hexadecimal digit for [number].
    // Assumes that [number] is less than 16.
    inline uint8_t hexCharFor(uint8_t number)
    {
      // assert(number < 0x10);
      return number < 0xA ? $0 + number
        : $a - 0xA + number;
    }

    // Returns the value of [character] as a decimal digit.
    // Assumes that [character] is a decimal digit.
    inline double asDecimal(uint8_t character)
    {
      // assert(character >= $0 && character <= $9);
      return character - $0;
    }

    // Returns the decimal digit for [number].
    // Assumes that [number] is less than 10.
    inline uint8_t decimalCharFor(uint8_t number)
    {
      // assert(number < 10);
      return $0 + number;
    }

    // Assumes that [character] is a left-hand brace-like
    // character, and returns the right-hand version.
    inline uint8_t opposite(uint8_t character)
    {
      switch (character) {
      case $lparen:
        return $rparen;
      case $lbrace:
        return $rbrace;
      case $lbracket:
        return $rbracket;
      default:
        return 0;
      }
    }

    // Returns [character], converted to upper-
    // case if it's an ASCII lowercase letter.
    inline uint8_t toUpperCase(uint8_t character)
    {
      return (character >= $a && character <= $z)
        ? character & ~asciiCaseBit : character;
    }

    // Returns [character], converted to lower-
    // case if it's an ASCII uppercase letter.
    inline uint8_t toLowerCase(uint8_t character)
    {
      return (character >= $A && character <= $Z)
        ? character | asciiCaseBit : character;
    }

    // Returns whether [character1] and [character2] are the same, modulo ASCII case.
    inline bool characterEqualsIgnoreCase(uint8_t character1, uint8_t character2)
    {
      if (character1 == character2) return true;

      // If this check fails, the characters are definitely different. If it
      // succeeds *and* either character is an ASCII letter, they're equivalent.
      if ((character1 ^ character2) != asciiCaseBit) return false;

      // Now we just need to verify that one of the characters is an ASCII letter.
      uint8_t upperCase1 = character1 & ~asciiCaseBit;
      return upperCase1 >= $A && upperCase1 <= $Z;
    }

    // Like [characterEqualsIgnoreCase], but optimized for the
    // fact that [letter] is known to be a lowercase ASCII letter.
    inline bool equalsLetterIgnoreCase(uint8_t letter, uint8_t actual)
    {
      // assert(letter >= $a && letter <= $z);
      return (actual | asciiCaseBit) == letter;
    }

  }

}

#endif
