#include <exception>

#include "catch/single_include/catch.hpp"
#include "../../quote.hpp"

TEST_CASE( "tests run", "[meta]" ) {
  REQUIRE( 1 == 1 );
}

TEST_CASE( "unquote empty", "[unquote]" ) {
  REQUIRE( std::string("") == Sass::unquote("") );
}

TEST_CASE( "unquote one quote", "[unquote][ill-formed]" ) {
  REQUIRE( std::string("") == Sass::unquote("\"") );
  REQUIRE( std::string("") == Sass::unquote("\'") );
}

TEST_CASE( "only quoted one side (doublequote)", "[unquote][ill-formed][doublequote]" ) {
  REQUIRE( std::string("\"foo") == Sass::unquote("\"foo") );
  REQUIRE( std::string("foo\"") == Sass::unquote("foo\"") );
}

TEST_CASE( "only quoted one side (singlequote)", "[unquote][ill-formed][singlequote]" ) {
  REQUIRE( std::string("'foo") == Sass::unquote("'foo") );
  REQUIRE( std::string("foo'") == Sass::unquote("foo'") );
}

TEST_CASE( "mismatched quotes", "[unquote][ill-formed]" ) {
  REQUIRE( std::string("'foo\"") == Sass::unquote("'foo\"") );
  REQUIRE( std::string("\"foo'") == Sass::unquote("\"foo'") );
}

TEST_CASE( "valid unquote (doublequote)", "[unquote][doublequote]" ) {
  REQUIRE( std::string("foo") == Sass::unquote("\"foo\"") );
  REQUIRE( std::string("f'o'o") == Sass::unquote("\"f'o'o\"") );  
}

TEST_CASE( "valid unquote (singlequote)", "[unquote][singlequote]" ) {
  REQUIRE( std::string("foo") == Sass::unquote("'foo'") );
  REQUIRE( std::string("f\"o\"o") == Sass::unquote("'f\"o\"o'") );
}

TEST_CASE( "valid unquote (doublequotes)(escaped quotes)", "[unquote][doublequote]" ) {
  REQUIRE( std::string("I said, \"Hello,\" to them.") ==
           Sass::unquote("\"I said, \\\"Hello,\\\" to them.\"") );
}

TEST_CASE( "valid unquote (singlequote)(escaped quotes)", "[unquote][singlequote]" ) {
  REQUIRE( std::string("I said, 'Hello,' to them.") ==
           Sass::unquote("\'I said, \\'Hello,\\' to them.'") );
}

TEST_CASE( "unquote handles string-final escaped quote", "[unquote]" ) {
  REQUIRE( std::string("I said, 'Hello.'") ==
           Sass::unquote("'I said, \\'Hello.\\''") );
  REQUIRE( std::string("I said, \"Hello.\"") ==
           Sass::unquote("\"I said, \\\"Hello.\\\"\"") );
}

TEST_CASE( "unquote handles string-initial escaped quote", "[unquote]" ) {
  REQUIRE( std::string("'Hello,' I said.") ==
           Sass::unquote("'\\'Hello,\\' I said.'") );
  REQUIRE( std::string("\"Hello,\" I said.") ==
           Sass::unquote("\"\\\"Hello,\\\" I said.\"") );
}

TEST_CASE( "unquote no longer throws on string-initial unescaped quote", "[unquote][ill-formed]" ) {
  REQUIRE( std::string("'Hello,' I said.") ==
           Sass::unquote("''Hello,\\' I said.'") );
  REQUIRE( std::string("\"Hello,\" I said.") ==
           Sass::unquote("\"\"Hello,\\\" I said.\"") );
}

TEST_CASE( "unquote no longer eats previous char on string-final unescaped quote", "[unquote][ill-formed]" ) {
  REQUIRE( std::string("I said, 'Hello.'") ==
           Sass::unquote("'I said, \\'Hello.''") );
  REQUIRE( std::string("I said, \"Hello.\"") ==
           Sass::unquote("\"I said, \\\"Hello.\"\"") );
}

TEST_CASE( "unquote always honors backslash", "[unquote]" ) {
  REQUIRE( std::string("quoted ' ignored \\ end") ==
           Sass::unquote("'quoted \\' ignored \\\\ end'") );
  REQUIRE( std::string("quoted \" ignored \\ end") ==
           Sass::unquote("\"quoted \\\" ignored \\\\ end\"") );
}

/// test quote

TEST_CASE( "quote empty string returns pair of quotes", "[quote]" ) {
  REQUIRE( std::string("''") == Sass::quote("", '\'') );
  REQUIRE( std::string("\"\"") == Sass::quote("", '"') );
}

TEST_CASE( "any char can be quote char", "[quote]" ) {
  // arguably bug
  REQUIRE( std::string("ZZ") == Sass::quote("", 'Z') );
}

TEST_CASE( "quote non-empty with NUL is no-op", "[quote]" ) {
  REQUIRE( std::string("'") == Sass::quote("'", '\0') );
  REQUIRE( std::string("Z") == Sass::quote("Z", '\0') );
  REQUIRE( std::string("\"") == Sass::quote("\"", '\0') );
}

TEST_CASE( "quote empty with NUL is weird", "[quote][corner-case]") {
  std::string sTwoNULs;

  sTwoNULs.push_back('\0');
  sTwoNULs.push_back('\0');
  
  REQUIRE( sTwoNULs == Sass::quote("", '\0') );
  REQUIRE( std::string("") != Sass::quote("", '\0') );
}

TEST_CASE( "quote quoted string is no-op", "[quote]") {
  REQUIRE( std::string("\"foo\"") == Sass::quote("\"foo\"", '"') );
  REQUIRE( std::string("\"foo\"") == Sass::quote("\"foo\"", '\'') );

  REQUIRE( std::string("'foo'") == Sass::quote("'foo'", '"') );
  REQUIRE( std::string("'foo'") == Sass::quote("'foo'", '\'') );
}

TEST_CASE( "only leading quote is checked", "[quote][ill-formed][edge-case]") {
  REQUIRE( std::string("\"foo\\\"\"") == Sass::quote("foo\"", '"') );
  REQUIRE( std::string("'foo\"'") == Sass::quote("foo\"", '\'') );

  REQUIRE( std::string("\"foo'\"") == Sass::quote("foo'", '"') );
  REQUIRE( std::string("'foo\\''") == Sass::quote("foo'", '\'') );
}

TEST_CASE( "backslash is not quoted", "[quote][edge-case][bug]") {
  REQUIRE( std::string("\"foo\\\\\\\"\"") == Sass::quote("foo\\\"", '"') );
  REQUIRE( std::string("'foo\\\\\"'") == Sass::quote("foo\\\"", '\'') );

  REQUIRE( std::string("\"foo\\\\'\"") == Sass::quote("foo\\'", '"') );
  REQUIRE( std::string("'foo\\\\\\''") == Sass::quote("foo\\'", '\'') );
}

TEST_CASE( "unquote with final backslash does not throw", "[unquote][ill-formed]" ) {
  REQUIRE( std::string("foo\'") == Sass::unquote("'foo\\'") );
  REQUIRE( std::string("foo\"") == Sass::unquote("\"foo\\\"") );
}

TEST_CASE( "unquote quoted string is same string", "[quote][unquote]" ) {
  const char * strings[] = {
    "",
    "a",
    "abc",
    "abc.def",
    "foo;bar",
    ".",
    "123",
    "10.34",
    ".34",
    "foo\\bar",
    "foo\\",
    NULL
  };

  int i = 0;
  const char * s = NULL;
  for (i = 0, s = strings[i]; s != NULL && i < 10; i += 1) {
    std::string orig(s);

    REQUIRE( orig == Sass::unquote(Sass::quote(s, '"')) );
    REQUIRE( orig == Sass::unquote(Sass::quote(s, '\'')) );
  }

}
