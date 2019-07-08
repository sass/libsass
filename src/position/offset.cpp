// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "sass.hpp"

#include <iostream>
#include "offset.hpp"
#include "charcode.hpp"
#include "character.hpp"
#include "util_string.hpp"

namespace Sass {

  using namespace Charcode;
  using namespace Character;

  // Empty constructor
  Offset::Offset() :
    line(0),
    column(0)
  {}

  // Create an `Offset` from the given string
  // Will use `append` internally on all chars
  Offset::Offset(uint8_t character)
    : line(0), column(0)
  {
    plus(character);
  }

  // Create an `Offset` from the given string
  // Will use `append` internally on all chars
  Offset::Offset(const sass::string& text)
    : line(0), column(0)
  {
    for (uint8_t character : text) {
      plus(character);
    }
  }

  // Create an `Offset` from the given char star
  // Will use `append` internally on all chars
  Offset::Offset(const char* text, const char* end)
    : line(0), column(0)
  {
    if (end == nullptr) {
      while (*text != 0) {
        plus(*text++);
      }
    }
    else {
      while (text < end) {
        plus(*text++);
      }
    }
  }

  // Append `character` to increment offset
  void Offset::plus(uint8_t character)
  {
    switch (character) {
    case $lf:
      line += 1;
      column = 0;
      break;
    case $space:
    case $tab:
    case $vt:
    case $ff:
    case $cr:
      column += 1;
      break;
    default:
      // skip over 10xxxxxx and 01xxxxxx
      // count ASCII and initial utf8 bytes
      if (Character::isCharacter(character)) {
        // 64 => initial utf8 byte
        // 128 => regular ASCII char
        column += 1;
      }
      break;
    }
  }

  // Append `character` to increment offset
  void Offset::plus(const sass::string& text)
  {
    for (uint8_t character : text) {
      plus(character);
    }
  }

  // Create offset with given `line` and `column`
  // Needs static constructor to avoid ambiguity
  Offset Offset::init(uint32_t line, uint32_t column)
  {
    Offset offset;
    offset.line = line;
    offset.column = column;
    return offset;
  }

  // Calculate the distance between start and end
  Offset Offset::distance(const Offset& start, const Offset& end)
  {
    Offset rv(end);
    // Both are on the same line
    if (start.line == end.line) {
      // Get distance between columns
      rv.column -= start.column;
      // Line distance is zero
      rv.line = 0;
    }
    else {
      // Get distance between lines
      rv.line -= start.line;
      // Columns don't need to be changed
      // Since we land on another line, we
      // need to reach the same end column
    }
    return rv;
  }

  // Implement comparison operators
  bool Offset::operator<(const Offset& rhs) const
  {
    if (line == rhs.line) {
      return column < rhs.column;
    }
    return line < rhs.line;
  }

  // Implement equality operators
  bool Offset::operator==(const Offset& rhs) const
  {
    return line == rhs.line
      && column == rhs.column;
  }

  // Implement assign and addition operator
  void Offset::operator+= (const Offset& rhs)
  {
    // lines are always summed up
    line += rhs.line;
    // columns may need to be reset
    if (rhs.line == 0) {
      column += rhs.column;
    }
    else {
      column = rhs.column;
    }
  }

  // Implement addition operator
  Offset Offset::operator+ (const Offset& rhs) const
  {
    Offset rv(*this);
    rv += rhs;
    return rv;
  }

  // Implement multiply operator
  Offset Offset::operator* (uint32_t mul) const
  {
    Offset rv(*this);
    if (rv.line == 0) {
      rv.column *= mul;
    }
    else {
      rv.line *= mul;
    }
    return rv;
  }

  // Move/increment char star `text` by `offset`
  const char* Offset::move(const char* text, Offset offset)
  {
    while (offset.line > 0) {
      while (text[0] != '\n') {
        if (text[0] == '\0') {
          return nullptr;
        }
        text += 1;
      }
      if (text[0] == '\0') {
        return nullptr;
      }
      offset.line -= 1;
      text += 1;
    }
    while (offset.column > 0) {
      while (!Character::isCharacter(text[0])) {
        if (text[0] == '\0') {
          return nullptr;
        }
        if (text[0] == '\n') {
          return nullptr;
        }
        text += 1;
      }
      if (text[0] == '\0') {
        return nullptr;
      }
      if (text[0] == '\n') {
        return nullptr;
      }
      offset.column -= 1;
      text += 1;
    }
    if (text[0] == '\n') {
      return nullptr;
    }
    return text;
  }
  // EO Offset::move

}
