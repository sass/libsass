#include "../src/sass/ast.hpp"
#include "../src/sass/context.hpp"
#include "../src/sass/parser.hpp"
#include "../src/sass/to_string.hpp"
#include <string>

Sass::Context ctx = Sass::Context(Sass::Context::Data());
Sass::To_String sass_to_string;

Sass::Compound_Selector* selector(string src)
{ return Sass::Parser::from_c_str(src.c_str(), ctx, "", Sass::Position()).parse_simple_selector_sequence(); }

void unify(string lhs, string rhs)
{
  Sass::Compound_Selector* unified = selector(lhs + ";")->unify_with(selector(rhs + ";"), ctx);
  cout << lhs << " UNIFIED WITH " << rhs << " =\t" << (unified ? unified->perform(&sass_to_string) : "NOTHING") << endl;
}

int main()
{
  unify(".foo", ".foo.bar");
  unify("div:nth-of-type(odd)", "div:first-child");
  unify("div", "span:whatever");
  unify("div", "span");
  unify("foo:bar::after", "foo:bar::first-letter");
  unify(".foo#bar.hux", ".hux.foo#bar");
  unify(".foo#bar.hux", ".hux.foo#baz");
  unify("*:blah:fudge", "p:fudge:blah");

  return 0;
}
