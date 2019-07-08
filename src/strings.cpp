#include "strings.hpp"

namespace Sass
{

	namespace Strings
	{

		const sass::string empty("");

    const sass::string plus("+");
		const sass::string minus("-");
		const sass::string percent("%");

		const sass::string rgb("rgb");
    const sass::string hsl("hsl");
    const sass::string hwb("hwb");
    const sass::string rgba("rgba");
    const sass::string hsla("hsla");
    const sass::string hwba("hwba");

    const sass::string deg("deg");
    const sass::string red("red");
		const sass::string hue("hue");
		const sass::string blue("blue");
		const sass::string green("green");
		const sass::string alpha("alpha");
    const sass::string color("color");
    const sass::string weight("weight");
    const sass::string number("number");
    const sass::string amount("amount");
    const sass::string invert("invert");
    const sass::string degrees("degrees");
    const sass::string saturate("saturate");
    const sass::string grayscale("grayscale");
    const sass::string whiteness("whiteness");
    const sass::string blackness("blackness");

    const sass::string $whiteness("$whiteness");
    const sass::string $blackness("$blackness");
    
    
    

		const sass::string lightness("lightness");
		const sass::string saturation("saturation");

		const sass::string key("key");
		const sass::string map("map");
		const sass::string map1("map1");
		const sass::string map2("map2");
		const sass::string args("args");
    const sass::string null("null");
    const sass::string boolean("bool");
    const sass::string error("error");
    const sass::string warning("warning");
    const sass::string string("string");
    const sass::string function("function");
    const sass::string arglist("arglist");

    const sass::string list("list");
    const sass::string name("name");
		const sass::string media("media");
		const sass::string module("module");
    const sass::string supports("supports");
    const sass::string keyframes("keyframes");



		const sass::string forRule("@for");
		const sass::string warnRule("@warn");
		const sass::string errorRule("@error");
		const sass::string debugRule("@debug");
		const sass::string extendRule("@extend");
		const sass::string importRule("@import");
		const sass::string contentRule("@content");

		const sass::string scaleColor("scale-color");
		const sass::string colorAdjust("color-adjust");
		const sass::string colorChange("color-change");

		const sass::string condition("condition");
		const sass::string ifFalse("if-false");
		const sass::string ifTrue("if-true");

    const sass::string $red("$red");
    const sass::string $green("$green");
    const sass::string $blue("$blue");

    const sass::string $hue("$hue");
    const sass::string $saturation("$saturation");
    const sass::string $lightness("$lightness");

    const sass::string utf8bom("\xEF\xBB\xBF");

    const sass::string argument("argument");
    const sass::string _and_("and");
    const sass::string _or_("or");

  } // namespace Strings

  namespace Keys {

    const EnvKey red(Strings::red);
		const EnvKey hue(Strings::hue);
		const EnvKey blue(Strings::blue);
		const EnvKey green(Strings::green);
		const EnvKey alpha(Strings::alpha);
    const EnvKey color(Strings::color);
    const EnvKey lightness(Strings::lightness);
    const EnvKey saturation(Strings::saturation);

    const EnvKey whiteness(Strings::whiteness);
    const EnvKey blackness(Strings::blackness);

    
      

		const EnvKey warnRule(Strings::warnRule);
		const EnvKey errorRule(Strings::errorRule);
		const EnvKey debugRule(Strings::debugRule);
		const EnvKey contentRule(Strings::contentRule);

		const EnvKey condition(Strings::condition);
		const EnvKey ifFalse(Strings::ifFalse);
		const EnvKey ifTrue(Strings::ifTrue);

	} // namespace Keys

} // namespace Sass
