#define SASS_POSITION

#include <cstdlib>
#include <iostream>

namespace Sass {

  class Position {
    public:
      size_t file;
      size_t line;
      size_t column;

      Position()
      : file(-1), line(0), column(0) { }

      Position(const size_t line, const size_t column)
      : file(-1), line(line), column(column) { }

      Position(const size_t file, const size_t line, const size_t column)
      : file(file), line(line), column(column) { }

      friend std::ostream& operator<<(std::ostream& strm, const Position& pos) {
        return strm << pos.line << ":" << pos.column;
      }

  };

}
