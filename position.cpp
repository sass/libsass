#include "position.hpp"

namespace Sass {

  using namespace std;

  Offset::Offset()
  : line(0), column(0) { }

  Offset::Offset(const size_t line, const size_t column)
  : line(line), column(column) { }

  Offset Offset::inc(const char* begin, const char* end) const
  {
    Offset offset(line, column);
    while (begin < end && *begin) {
      if (*begin == '\n') {
        ++ offset.line;
        offset.column = 0;
      } else {
        ++ offset.column;
      }
      ++begin;
    }
    return offset;
  }

  bool Offset::operator== (const Offset &pos) const
  {
    return line == pos.line && column == pos.column;
  }

  bool Offset::operator!= (const Offset &pos) const
  {
    return line != pos.line || column != pos.column;
  }

  const Offset Offset::operator+ (const Offset &off) const
  {
    return Offset(line + off.line, off.line > 0 ? off.column : off.column + column);
  }

  Selection::Selection(string path)
  : Position(-1, -1, -1), path(path) { }

  Selection::Selection(string path, Position position, Offset offset)
  : Position(position), path(path), offset(offset) { }

  Position::Position()
  : Offset(-1, -1), file(-1) { }

  Position::Position(const size_t file)
  : Offset(0, 0), file(file) { }

  Position::Position(const size_t line, const size_t column)
  : Offset(line, column), file(-1) { }

  Position::Position(const size_t file, const size_t line, const size_t column)
  : Offset(line, column), file(file) { }

  /*
  Position::Position(const char* begin, const char* end)
  : file(-1), line(0), column(0)
  {
  }
  */

  Position Position::inc(const char* begin, const char* end) const
  {
    Position pos(line, column);
    while (begin < end && *begin) {
      if (*begin == '\n') {
        ++ pos.line;
        pos.column = 0;
      } else {
        ++ pos.column;
      }
      ++begin;
    }
    return pos;
  }

  bool Position::operator== (const Position &pos) const
  {
    return file == pos.file && line == pos.line && column == pos.column;
  }

  bool Position::operator!= (const Position &pos) const
  {
    return file == pos.file || line != pos.line || column != pos.column;
  }

  const Position Position::operator+ (const Offset &off) const
  {
    return Position(file, line + off.line, off.line > 0 ? off.column : off.column + column);
  }

  std::ostream& operator<<(std::ostream& strm, const Position& pos)
  {
    if (pos.file != string::npos) strm << pos.file << ":";
    if (pos.line == string::npos) strm << "-1:"; else strm << pos.line << ":";
    if (pos.column == string::npos) strm << "-1"; else strm << pos.column;
    return strm;
  }

}