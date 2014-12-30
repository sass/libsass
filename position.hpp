#ifndef SASS_POSITION
#define SASS_POSITION

#include <string>
#include <cstdlib>
#include <iostream>

namespace Sass {

  using namespace std;

  class Offset {

    public: // c-tor
      Offset();
      Offset(const size_t line, const size_t column);

      // return new position, incremented by the given string
      Offset inc(const char* begin, const char* end) const;

    public: // overload operators for position
      bool operator== (const Offset &pos) const;
      bool operator!= (const Offset &pos) const;
      const Offset operator+ (const Offset &off) const;

    public: // overload output stream operator
      friend ostream& operator<<(ostream& strm, const Offset& off);

    public:
      size_t line;
      size_t column;

  };

  class Position : public Offset {

    public: // c-tor
      Position();
      Position(const size_t file);
      Position(const size_t line, const size_t column);
      Position(const size_t file, const size_t line, const size_t column);

    public: // overload operators for position
      bool operator== (const Position &pos) const;
      bool operator!= (const Position &pos) const;
      const Position operator+ (const Offset &off) const;
      // return new position, incremented by the given string
      Position inc(const char* begin, const char* end) const;

    public: // overload output stream operator
      friend ostream& operator<<(ostream& strm, const Position& pos);

    public:
      size_t file;

  };

  class Selection : public Position{

    public:
      Selection(string path);
      Selection(string path, const size_t file);
      Selection(string path, Position position, Offset offset);

    public: // getters
      const char* getPath() const;
    public: // setters
      void setPath(const char* path);

    public:
      string path;
      Offset offset;

  };

}

#endif