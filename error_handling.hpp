#define SASS_ERROR_HANDLING
#include <string>

#ifndef SASS_POSITION
#include "position.hpp"
#endif

namespace Sass {
  using namespace std;

  struct Backtrace;

  struct Sass_Error {
    enum Type { read, write, syntax, evaluation };

    Type type;
    string path;
    Selection slct;
    string message;

    Sass_Error(Type type, Selection slct, string message);

  };

  void error(string msg, Selection slct);
  void error(string msg, Selection slct, Backtrace* bt);

}
