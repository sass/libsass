/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "strings.hpp"

namespace Sass
{

  const sass::string str_empty("");

  // For list functions
  const sass::string str_length("length");
  const sass::string str_nth("nth");
  const sass::string str_set_nth("set-nth");
  const sass::string str_join("join");
  const sass::string str_append("append");
  const sass::string str_zip("zip");
  const sass::string str_slash("slash");
  const sass::string str_list_separator("list-separator");
  const sass::string str_is_bracketed("is-bracketed");

  // Rounding strategies
  const sass::string str_up = "up";
  const sass::string str_down = "down";
  const sass::string str_nearest = "nearest";
  const sass::string str_to_zero = "to-zero";

  // For map functions
  const sass::string str_set("set");
  const sass::string str_map_set("map-set");
  const sass::string str_get("get");
  const sass::string str_map_get("map-get");
  const sass::string str_merge("merge");
  const sass::string str_map_merge("map-merge");
  const sass::string str_remove("remove");
  const sass::string str_map_remove("map-remove");
  const sass::string str_keys("keys");
  const sass::string str_map_keys("map-keys");
  const sass::string str_base("base");
  const sass::string str_deg("deg");
  const sass::string str_angle("angle");
  const sass::string str_number("number");
  const sass::string str_value("value");
  const sass::string str_values("values");
  const sass::string str_map_values("map-values");
  const sass::string str_has_key("has-key");
  const sass::string str_map_has_key("map-has-key");
  const sass::string str_deep_merge("deep-merge");
  const sass::string str_deep_remove("deep-remove");

  // For text functions
  const sass::string str_unquote("unquote");
  const sass::string str_quote("quote");
  const sass::string str_to_upper_case("to-upper-case");
  const sass::string str_to_lower_case("to-lower-case");
  // const sass::string str_length("length");
  const sass::string str_str_length("str-length");
  const sass::string str_insert("insert");
  const sass::string str_str_insert("str-insert");
  const sass::string str_index("index");
  const sass::string str_str_index("str-index");
  const sass::string str_slice("slice");
  const sass::string str_split("split");
  const sass::string str_str_slice("str-slice");
  const sass::string str_str_split("str-split");
  const sass::string str_unique_id("unique-id");

  // For meta functions
  const sass::string str_load_css("load-css");
  const sass::string str_feature_exists("feature-exists");
  const sass::string str_type_of("type-of");
  const sass::string str_inspect("inspect");
  const sass::string str_keywords("keywords");
  const sass::string str_if("if");
  const sass::string str_apply("apply");
  const sass::string str_calc_name("calc-name");
  const sass::string str_calc_args("calc-args");
  const sass::string str_get_mixin("get-mixin");
  const sass::string str_module_mixins("module-mixins");
  const sass::string str_accepts_content("accepts-content");
  const sass::string str_global_variable_exists("global-variable-exists");
  const sass::string str_variable_exists("variable-exists");
  const sass::string str_function_exists("function-exists");
  const sass::string str_mixin_exists("mixin-exists");
  const sass::string str_content_exists("content-exists");
  const sass::string str_module_variables("module-variables");
  const sass::string str_module_functions("module-functions");
  const sass::string str_get_function("get-function");
  const sass::string str_call("call");

  // For color functions
  const sass::string str_rgb("rgb");
  const sass::string str_rgba("rgba");
  const sass::string str_hsl("hsl");
  const sass::string str_hsla("hsla");
  const sass::string str_hwb("hwb");
  const sass::string str_hwba("hwba");
  const sass::string str_red("red");
  const sass::string str_green("green");
  const sass::string str_blue("blue");
  const sass::string str_hue("hue");
  const sass::string str_from("from");
  const sass::string str_lightness("lightness");
  const sass::string str_saturation("saturation");
  const sass::string str_blackness("blackness");
  const sass::string str_whiteness("whiteness");
  const sass::string str_invert("invert");
  const sass::string str_grayscale("grayscale");
  const sass::string str_complement("complement");
  const sass::string str_desaturate("desaturate");
  const sass::string str_saturate("saturate");
  const sass::string str_lighten("lighten");
  const sass::string str_darken("darken");
  const sass::string str_adjust_hue("adjust-hue");
  const sass::string str_adjust_color("adjust-color");
  const sass::string str_change_color("change-color");
  const sass::string str_scale_color("scale-color");
  const sass::string str_adjust("adjust");
  const sass::string str_change("change");
  const sass::string str_scale("scale");
  const sass::string str_mix("mix");
  const sass::string str_opacify("opacify");
  const sass::string str_fade_in("fade-in");
  const sass::string str_fade_out("fade-out");
  const sass::string str_transparentize("transparentize");
  const sass::string str_ie_hex_str("ie-hex-str");
  const sass::string str_alpha("alpha");
  const sass::string str_opacity("opacity");

  // For math functions
  const sass::string str_e("e");
  const sass::string str_pi("pi");
  const sass::string str_tau("tau");
  const sass::string str_epsilon("epsilon");
  const sass::string str_min_safe_integer("min-safe-integer");
  const sass::string str_max_safe_integer("max-safe-integer");
  const sass::string str_min_number("min-number");
  const sass::string str_max_number("max-number");

  const sass::string str_infinity("infinity");
  const sass::string str_neg_infinity("-infinity");
  const sass::string str_nan("nan");

  const sass::string str_ceil("ceil");
  const sass::string str_calc("calc");
  const sass::string str_clamp("clamp");
  const sass::string str_floor("floor");
  const sass::string str_max("max");
  const sass::string str_min("min");
  const sass::string str_round("round");
  const sass::string str_abs("abs");
  const sass::string str_exp("exp");
  const sass::string str_sign("sign");
  const sass::string str_hypot("hypot");
  const sass::string str_log("log");
  const sass::string str_div("div");
  const sass::string str_pow("pow");
  const sass::string str_sqrt("sqrt");
  const sass::string str_cos("cos");
  const sass::string str_sin("sin");
  const sass::string str_tan("tan");
  const sass::string str_acos("acos");
  const sass::string str_asin("asin");
  const sass::string str_atan("atan");
  const sass::string str_atan2("atan2");
  const sass::string str_mod("mod");
  const sass::string str_rem("rem");
  const sass::string str_random("random");
  const sass::string str_unit("unit");
  const sass::string str_percentage("percentage");
  const sass::string str_unitless("unitless");
  const sass::string str_is_unitless("is-unitless");
  const sass::string str_compatible("compatible");
  const sass::string str_comparable("comparable");
  const sass::string str_rad("rad");

  // For selector functions
  const sass::string str_nest("nest");
  const sass::string str_selector_nest("selector-nest");
  // const sass::string str_append("append");
  const sass::string str_selector_append("selector-append");
  const sass::string str_extend("extend");
  const sass::string str_selector_extend("selector-extend");
  const sass::string str_replace("replace");
  const sass::string str_selector_replace("selector-replace");
  const sass::string str_unify("unify");
  const sass::string str_selector_unify("selector-unify");
  const sass::string str_parse("parse");
  const sass::string str_selector_parse("selector-parse");
  const sass::string str_is_superselector("is-superselector");
  const sass::string str_simple_selectors("simple-selectors");




  const Units unit_rad(str_rad);
  const Units unit_deg(str_deg);
  const Units unit_percent("%");
  const Units unit_none;










  // For list functions
  const EnvKey key_length(str_length);
  const EnvKey key_nth(str_nth);
  const EnvKey key_set_nth(str_set_nth);
  const EnvKey key_join(str_join);
  const EnvKey key_append(str_append);
  const EnvKey key_zip(str_zip);
  const EnvKey key_slash(str_slash);
  const EnvKey key_list_separator(str_list_separator);
  const EnvKey key_is_bracketed(str_is_bracketed);

  // For map functions
  const EnvKey key_set(str_set);
  const EnvKey key_map_set(str_map_set);
  const EnvKey key_get(str_get);
  const EnvKey key_map_get(str_map_get);
  const EnvKey key_merge(str_merge);
  const EnvKey key_map_merge(str_map_merge);
  const EnvKey key_remove(str_remove);
  const EnvKey key_map_remove(str_map_remove);
  const EnvKey key_keys(str_keys);
  const EnvKey key_map_keys(str_map_keys);
  const EnvKey key_values(str_values);
  const EnvKey key_map_values(str_map_values);
  const EnvKey key_has_key(str_has_key);
  const EnvKey key_map_has_key(str_map_has_key);
  const EnvKey key_deep_merge(str_deep_merge);
  const EnvKey key_deep_remove(str_deep_remove);

  // For text functions
  const EnvKey key_unquote(str_unquote);
  const EnvKey key_quote(str_quote);
  const EnvKey key_to_upper_case(str_to_upper_case);
  const EnvKey key_to_lower_case(str_to_lower_case);
  // const EnvKey key_length(str_length);
  const EnvKey key_str_length(str_str_length);
  const EnvKey key_insert(str_insert);
  const EnvKey key_str_insert(str_str_insert);
  const EnvKey key_index(str_index);
  const EnvKey key_str_index(str_str_index);
  const EnvKey key_slice(str_slice);
  const EnvKey key_split(str_split);
  const EnvKey key_str_slice(str_str_slice);
  const EnvKey key_str_split(str_str_split);
  const EnvKey key_unique_id(str_unique_id);

  // For meta functions
  const EnvKey key_load_css(str_load_css);
  const EnvKey key_feature_exists(str_feature_exists);
  const EnvKey key_type_of(str_type_of);
  const EnvKey key_inspect(str_inspect);
  const EnvKey key_keywords(str_keywords);
  const EnvKey key_if(str_if);
  const EnvKey key_apply(str_apply);
  const EnvKey key_calc_name(str_calc_name);
  const EnvKey key_calc_args(str_calc_args);
  const EnvKey key_get_mixin(str_get_mixin);
  const EnvKey key_module_mixins(str_module_mixins);
  const EnvKey key_accepts_content(str_accepts_content);;
  const EnvKey key_global_variable_exists(str_global_variable_exists);
  const EnvKey key_variable_exists(str_variable_exists);
  const EnvKey key_function_exists(str_function_exists);
  const EnvKey key_mixin_exists(str_mixin_exists);
  const EnvKey key_content_exists(str_content_exists);
  const EnvKey key_module_variables(str_module_variables);
  const EnvKey key_module_functions(str_module_functions);
  const EnvKey key_get_function(str_get_function);
  const EnvKey key_call(str_call);

  // For color functions
  const EnvKey key_rgb(str_rgb);
  const EnvKey key_rgba(str_rgba);
  const EnvKey key_hsl(str_hsl);
  const EnvKey key_hsla(str_hsla);
  const EnvKey key_hwb(str_hwb);
  const EnvKey key_hwba(str_hwba);
  const EnvKey key_red(str_red);
  const EnvKey key_green(str_green);
  const EnvKey key_blue(str_blue);
  const EnvKey key_hue(str_hue);
  const EnvKey key_lightness(str_lightness);
  const EnvKey key_saturation(str_saturation);
  const EnvKey key_blackness(str_blackness);
  const EnvKey key_whiteness(str_whiteness);
  const EnvKey key_invert(str_invert);
  const EnvKey key_grayscale(str_grayscale);
  const EnvKey key_complement(str_complement);
  const EnvKey key_desaturate(str_desaturate);
  const EnvKey key_saturate(str_saturate);
  const EnvKey key_lighten(str_lighten);
  const EnvKey key_darken(str_darken);
  const EnvKey key_adjust_hue(str_adjust_hue);
  const EnvKey key_adjust_color(str_adjust_color);
  const EnvKey key_change_color(str_change_color);
  const EnvKey key_scale_color(str_scale_color);
  const EnvKey key_adjust(str_adjust);
  const EnvKey key_change(str_change);
  const EnvKey key_scale(str_scale);
  const EnvKey key_mix(str_mix);
  const EnvKey key_opacify(str_opacify);
  const EnvKey key_fade_in(str_fade_in);
  const EnvKey key_fade_out(str_fade_out);
  const EnvKey key_transparentize(str_transparentize);
  const EnvKey key_ie_hex_str(str_ie_hex_str);
  const EnvKey key_alpha(str_alpha);
  const EnvKey key_opacity(str_opacity);

  // For math functions
  const EnvKey key_e(str_e);
  const EnvKey key_pi(str_pi);
  const EnvKey key_tau(str_tau);
  const EnvKey key_epsilon(str_epsilon);
  const EnvKey key_min_safe_integer(str_min_safe_integer);
  const EnvKey key_max_safe_integer(str_max_safe_integer);
  const EnvKey key_min_number(str_min_number);
  const EnvKey key_max_number(str_max_number);


  const EnvKey key_ceil(str_ceil);
  const EnvKey key_clamp(str_clamp);
  const EnvKey key_floor(str_floor);
  const EnvKey key_max(str_max);
  const EnvKey key_min(str_min);
  const EnvKey key_round(str_round);
  const EnvKey key_abs(str_abs);
  const EnvKey key_hypot(str_hypot);
  const EnvKey key_log(str_log);
  const EnvKey key_div(str_div);
  const EnvKey key_pow(str_pow);
  const EnvKey key_sqrt(str_sqrt);
  const EnvKey key_cos(str_cos);
  const EnvKey key_sin(str_sin);
  const EnvKey key_tan(str_tan);
  const EnvKey key_acos(str_acos);
  const EnvKey key_asin(str_asin);
  const EnvKey key_atan(str_atan);
  const EnvKey key_atan2(str_atan2);
  const EnvKey key_random(str_random);
  const EnvKey key_unit(str_unit);
  const EnvKey key_percentage(str_percentage);
  const EnvKey key_unitless(str_unitless);
  const EnvKey key_is_unitless(str_is_unitless);
  const EnvKey key_compatible(str_compatible);
  const EnvKey key_comparable(str_comparable);

  // For selector functions
  const EnvKey key_nest(str_nest);
  const EnvKey key_selector_nest(str_selector_nest);
  // const EnvKey key_append(str_append);
  const EnvKey key_selector_append(str_selector_append);
  const EnvKey key_extend(str_extend);
  const EnvKey key_selector_extend(str_selector_extend);
  const EnvKey key_replace(str_replace);
  const EnvKey key_selector_replace(str_selector_replace);
  const EnvKey key_unify(str_unify);
  const EnvKey key_selector_unify(str_selector_unify);
  const EnvKey key_parse(str_parse);
  const EnvKey key_selector_parse(str_selector_parse);
  const EnvKey key_is_superselector(str_is_superselector);
  const EnvKey key_simple_selectors(str_simple_selectors);


	namespace Strings
	{

		const sass::string empty("");

    // For list functions
    const sass::string length("length");
    const sass::string nth("nth");
    const sass::string setNth("set-nth");
    const sass::string join("join");
    const sass::string append("append");
    const sass::string zip("zip");
    const sass::string listSeparator("list-separator");
    const sass::string isBracketed("is-bracketed");


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
    const sass::string calc("calc");
    const sass::string null("null");
    const sass::string boolean("bool");
    const sass::string error("error");
    const sass::string warning("warning");
    const sass::string string("string");
    const sass::string function("function");
    const sass::string calculation("calculation");
    const sass::string calcoperation("calcoperation");
    const sass::string mixin("mixin");
    const sass::string arglist("arglist");

    const sass::string url("url");
    const sass::string with("with");
    const sass::string list("list");
    const sass::string name("name");
		const sass::string media("media");
		const sass::string module("module");
    const sass::string supports("supports");
    const sass::string keyframes("keyframes");



    const sass::string useRule("@use");
		const sass::string forRule("@for");
		const sass::string warnRule("@warn");
		const sass::string errorRule("@error");
		const sass::string debugRule("@debug");
		const sass::string extendRule("@extend");
		const sass::string importRule("@import");
		const sass::string contentRule("@content");
    const sass::string forwardRule("@forward");

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

    // For list functions
    const EnvKey length(Strings::length);
    const EnvKey nth(Strings::nth);
    const EnvKey setNth(Strings::setNth);
    const EnvKey join(Strings::join);
    const EnvKey append(Strings::append);
    const EnvKey zip(Strings::zip);
    const EnvKey listSeparator(Strings::listSeparator);
    const EnvKey isBracketed(Strings::isBracketed);

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
