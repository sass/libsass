/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "offset.hpp"

#include "charcode.hpp"
#include "character.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Charcode;
  using namespace Character;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Default constructor
  Offset::Offset() {}

  // Create an Offset from the given string
  // Will use `plus` internally on all chars
  Offset::Offset(uint8_t character)
  {
    plus(character);
  }

  // Create an Offset from the given string
  // Will use `plus` internally on all chars
  Offset::Offset(const sass::string& text)
  {
    for (uint8_t character : text) {
      plus(character);
    }
  }

  // Create an Offset from the given char star
  // Will use `plus` internally on all chars
  Offset::Offset(const char* text, const char* end)
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

  // Append [character] to increment offset
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

  // Append [text] to increment offset
  void Offset::plus(const sass::string& text)
  {
    for (uint8_t character : text) {
      plus(character);
    }
  }

  // Create offset with given [line] and [column]
  // Needs static constructor to avoid ambiguity
  Offset Offset::init(size_t line, size_t column)
  {
    Offset offset;
    offset.line = line == NPOS ? -1 : (uint32_t) line;
    offset.column = column == NPOS ? -1 : (uint32_t) column;
    return offset;
  }
  // EO Offset::init

  // Return the `distance` between [start[ and [end]
  // Gives the solution to the equation `end = start + x`
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
      // will reach the same end column
    }
    return rv;
  }
  // EO Offset::distance

  // Implement equality operators
  bool Offset::operator== (const Offset& rhs) const
  {
    return line == rhs.line
      && column == rhs.column;
  }
  // EO Offset::operator==

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
  // EO Offset::operator+=

  // Implement addition operator (returns new Offset)
  Offset Offset::operator+ (const Offset& rhs) const
  {
    Offset rv(*this);
    rv += rhs;
    return rv;
  }
  // EO Offset::operator+

  // Implement multiply operator (returns new Offset)
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
  // EO Offset::operator*

}
