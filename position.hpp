#define SASS_POSITION

#include <cstdlib>

namespace Sass {

  struct Position {
    size_t file;
    size_t line;
    size_t column;
    size_t item;

    Position()
    : file(0), line(0), column(0), item(0) { }

    Position(const size_t file, const size_t line, const size_t column, const size_t item)
    : file(file), line(line), column(column), item(item) { }

    Position(const size_t file, const size_t line, const size_t column)
    : file(file), line(line), column(column), item(0) { }

    Position(const size_t line, const size_t column)
    : file(0), line(line), column(column), item(0) { }

  };

}
