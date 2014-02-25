#include "../src/sass/ast.hpp"
#include "../src/sass/context.hpp"
#include "../src/sass/parser.hpp"
#include "../src/sass/to_string.hpp"
#include <string>
#include <iostream>

using namespace std;
using namespace Sass;

Context ctx = Context::Data();
To_String sass_to_string;

Compound_Selector* selector(string src)
{ return Parser::from_c_str(src.c_str(), ctx, "", Position()).parse_simple_selector_sequence(); }

void diff(string s, string t)
{
  cout << s << " - " << t << " = " << selector(s + ";")->minus(selector(t + ";"), ctx)->perform(&sass_to_string) << endl;
}

int main()
{
  diff(".a.b.c", ".c.b");
  diff(".a.b.c", ".fludge.b");

  return 0;
}
