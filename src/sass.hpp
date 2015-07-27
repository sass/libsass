#ifndef SASS_H
#define SASS_H

// #include <stack>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace Sass {

  // export only some symbols from std
  // avoids to expose full std namespace
  using std::cerr;
  using std::endl;
  // using std::stack;
  using std::vector;
  using std::string;
  using std::stringstream;
  using std::runtime_error;

}


#endif
