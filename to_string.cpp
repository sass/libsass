#include <cmath>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "ast.hpp"
#include "inspect.hpp"
#include "context.hpp"
#include "to_string.hpp"

namespace Sass {
  using namespace std;

  To_String::To_String(Context* ctx) : ctx(ctx) { }
  To_String::~To_String() { }

  inline string To_String::fallback_impl(AST_Node* n)
  {

    OutputBuffer buffer;
    Emitter emitter(buffer, ctx, NESTED);
    Inspect i(emitter);
    n->perform(&i);
    return buffer.buffer;
  }

  inline string To_String::operator()(Null* n)
  { return ""; }
}
