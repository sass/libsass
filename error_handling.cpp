#ifndef SASS_ERROR_HANDLING
#include "error_handling.hpp"
#endif

#include "backtrace.hpp"
#include "prelexer.hpp"

namespace Sass {

  Sass_Error::Sass_Error(Type type, Selection slct, string message)
  : type(type), slct(slct), message(message)
  { }

  void error(string msg, Selection slct)
  { throw Sass_Error(Sass_Error::syntax, slct, msg); }

  void error(string msg, Selection slct, Backtrace* bt)
  {
    if (!slct.path.empty() && Prelexer::string_constant(slct.path.c_str()))
      slct.path = slct.path.substr(1, slct.path.size() - 1);

    Backtrace top(bt, slct.path, slct.position, "");
    msg += top.to_string();

    throw Sass_Error(Sass_Error::syntax, slct, msg);
  }

}
