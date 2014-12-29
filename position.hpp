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

    public: // getters
      size_t getCol() const;
      size_t getLine() const;
      size_t getFile() const;
    public: // setters
      void setPos(const Position pos);
      void setCol(const size_t col);
      void setLine(const size_t line);
      void setFile(const size_t file);

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

  class Selection : public Position{

    public:
      Selection();
      Selection(string path, Position position);

    public: // getters
      const char* getPath() const;
    public: // setters
      void setPath(const char* path);

    public:
      string path;

  };

}

#endif