#ifndef SASS_QUOTE
#define SASS_QUOTE

#include <string>

namespace Sass {
  using namespace std;

  string unquote(const string&);
  string quote(const string&, char);

}

#endif
