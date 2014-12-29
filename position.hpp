#ifndef SASS_POSITION
#define SASS_POSITION

#include <string>
#include <cstdlib>
#include <iostream>

namespace Sass {

  using namespace std;

  class Position {

    public: // c-tor
      Position();
      Position(const size_t file);
      Position(const string& data);
      Position(const size_t line, const size_t column);
      Position(const size_t file, const size_t line, const size_t column);

      // return new position, incremented by the given string
      Position inc(const char* begin, const char* end) const;

    public: // overload operators for position
      bool operator== (const Position &pos) const;
      bool operator!= (const Position &pos) const;
      const Position operator+ (const Position &pos) const;

    public: // overload output stream operator
      friend ostream& operator<<(ostream& strm, const Position& pos);

    public:
      size_t file;
      size_t line;
      size_t column;

  };

  class Selection {
    public:
      string path;
      Position position;
    public:
      Selection();
      Selection(string path, Position position);
  };

}

#endif