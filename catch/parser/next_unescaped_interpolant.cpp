#include <exception>

#include "catch/single_include/catch.hpp"
#include "../../parser.hpp"


TEST_CASE( "tests run", "[meta]" ) {
  REQUIRE( 1 == 1 );
}

TEST_CASE( "create a parser", "[parser]" ) {
  Sass::Context::Data initializers;
  initializers.source_c_str("")
    .output_path("")
    .output_style((Sass::Output_Style) Sass::NESTED)
    .source_map_file("")
    .source_map_embed("")
    .source_map_contents("")
    .image_path("")
    .include_paths_c_str("")
    .include_paths_array(0)
    .include_paths(vector<string>())
    .precision(5)
    .importer(0); 

  Sass::Context ctx(initializers);
  Sass::Position pos;
  Sass::Parser p(ctx, "", pos);
}

TEST_CASE( "next_unescaped_interpolant (no interpolant)", "[parser][unit]" ) {
  const char * t = "no interpolant here";

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (one interpolant)", "[parser][unit]" ) {
  //                0         1         2
  //                012345678901234567890123
  const char * t = "one #{interpolant} here";

  REQUIRE( (t+4) ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+5, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (two interpolants)", "[parser][unit]" ) {
  //                0         1         2        3
  //                0123456789012345678901234578901234567
  const char * t = "two #{interpolants} #{in} this string";

  REQUIRE( (t+4) ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( (t+20) ==
           Sass::Parser::next_unescaped_interpolant(t+5, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+21, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (escaped interpolant)", "[parser][unit]" ) {
  //                0         1         2        3         4
  //                01234567890123456789012 34578901234567890123456789
  const char * t = "two #{interpolants}, 1 \\#{escaped} #{in} this string";

  REQUIRE( (t+4) ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( (t+35) ==
           Sass::Parser::next_unescaped_interpolant(t+5, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+37, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (string-initial interpolant)", "[parser][unit]" ) {
  //                0         1         2        3         4
  //                01234567890123456789012 34578901234567890123456789
  const char * t = "#{interpolant}";

  REQUIRE( (t) ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+1, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (string-final interpolant)", "[parser][unit]" ) {
  //                0         1         2        3         4
  //                01234567890123456789012 34578901234567890123456789
  const char * t = "ends with #{interpolant}";

  REQUIRE( (t+10) ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+11, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (string-initial escaped interpolant)", "[parser][unit]" ) {
  //                 0         1         2        3         4
  //                 01234567890123456789012 34578901234567890123456789
  const char * t = "\\#{ignore-me} #{interpolant}";

  REQUIRE( (t+14) ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+15, t+strlen(t)) );
}


TEST_CASE( "next_unescaped_interpolant (string-final escaped interpolant)", "[parser][unit]" ) {
  //                 0         1         2        3         4
  //                 01234567890123456789012 34578901234567890123456789
  const char * t = "#{find-me} \\#{ignore-me}";

  REQUIRE( t ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t+1, t+strlen(t)) );
}

TEST_CASE( "next_unescaped_interpolant (escaped interpolant alone in string)", "[parser][unit]" ) {
  //                 0         1         2        3         4
  //                 01234567890123456789012 34578901234567890123456789
  const char * t = "\\#{ignore-me}";

  REQUIRE( NULL ==
           Sass::Parser::next_unescaped_interpolant(t, t+strlen(t)) );
}
