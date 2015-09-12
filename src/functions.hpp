#ifndef SASS_FUNCTIONS_H
#define SASS_FUNCTIONS_H

#include "listize.hpp"
#include "position.hpp"
#include "environment.hpp"
#include "ast_fwd_decl.hpp"
#include "sass/functions.h"

#define BUILT_IN(name) Expression*\
name(Env& env, Env& d_env, Context& ctx, Signature sig, Parameters* params, ParserState pstate, Backtrace* backtrace)

namespace Sass {
  class Context;
  struct Backtrace;
  class AST_Node;
  class Expression;
  class Definition;
  typedef Environment<AST_Node*> Env;
  typedef const char* Signature;
  typedef Expression* (*Native_Function)(Env&, Env&, Context&, Signature, Parameters*, ParserState, Backtrace*);

  Definition* make_native_function(std::string name, Signature, Parameters*, Native_Function, Context& ctx);
  Definition* make_c_function(Sass_Function_Entry c_func, Context& ctx);

  std::string function_name(Signature);

  namespace Functions {

    extern Signature rgb_sig;
    extern Parameter rgb_red;
    extern Parameter rgb_green;
    extern Parameters rgb_params;

    extern Signature rgba_4_sig;
    extern Parameter rgba_4_red;
    extern Parameter rgba_4_green;
    extern Parameter rgba_4_blue;
    extern Parameter rgba_4_alpha;
    extern Parameters rgba_4_params;

    extern Signature rgba_2_sig;
    extern Parameter rgba_2_alpha;
    extern Parameter rgba_2_color;
    extern Parameters rgba_2_params;

    extern Signature red_sig;
    extern Parameter red_color;
    extern Parameters red_params;

    extern Signature green_sig;
    extern Parameter green_color;
    extern Parameters green_params;

    extern Signature blue_sig;
    extern Parameter blue_color;
    extern Parameters blue_params;

    extern Signature mix_sig;
    extern Parameter mix_color_1;
    extern Parameter mix_color_2;
    extern Parameter mix_weight;
    extern Number mix_weight_default;
    extern Parameters mix_params;

    extern Signature hsl_sig;
    extern Parameter hsl_hue;
    extern Parameter hsl_saturation;
    extern Parameter hsl_lightness;
    extern Parameters hsl_params;

    extern Signature hsla_sig;
    extern Parameter hsla_hue;
    extern Parameter hsla_saturation;
    extern Parameter hsla_lightness;
    extern Parameter hsla_alpha;
    extern Parameters hsla_params;

    extern Signature hue_sig;
    extern Parameter hue_color;
    extern Parameters hue_params;

    extern Signature saturation_sig;
    extern Parameter saturation_color;
    extern Parameters saturation_params;

    extern Signature lightness_sig;
    extern Parameter lightness_color;
    extern Parameters lightness_params;

    extern Signature adjust_hue_sig;
    extern Parameter adjust_hue_color;
    extern Parameter adjust_hue_degrees;
    extern Parameters adjust_hue_params;

    extern Signature lighten_sig;
    extern Parameter lighten_color;
    extern Parameter lighten_amount;
    extern Parameters lighten_params;

    extern Signature darken_sig;
    extern Parameter darken_color;
    extern Parameter darken_amount;
    extern Parameters darken_params;

    extern Signature saturate_sig;
    extern Parameter saturate_color;
    extern Parameter saturate_amount;
    extern Parameters saturate_params;
    extern Boolean saturate_amount_default;

    extern Signature desaturate_sig;
    extern Parameter desaturate_color;
    extern Parameter desaturate_amount;
    extern Parameters desaturate_params;

    extern Signature grayscale_sig;
    extern Parameter grayscale_color;
    extern Parameters grayscale_params;

    extern Signature complement_sig;
    extern Parameter compliment_color;
    extern Parameters complement_params;

    extern Signature invert_sig;
    extern Parameter invert_color;
    extern Parameters invert_params;


    extern Signature alpha_sig;
    extern Parameter alpha_color;
    extern Parameters alpha_params;

    extern Signature opacity_sig;
    extern Parameter opacity_color;
    extern Parameters opacity_params;

    extern Signature opacify_sig;
    extern Parameter opacify_color;
    extern Parameter opacify_amount;
    extern Parameters opacify_params;

    extern Signature fade_in_sig;
    extern Parameter fade_in_color;
    extern Parameter fade_in_amount;
    extern Parameters fade_in_params;

    extern Signature transparentize_sig;
    extern Parameter transparentize_color;
    extern Parameter transparentize_amount;
    extern Parameters transparentize_params;

    extern Signature fade_out_sig;
    extern Parameter fade_out_color;
    extern Parameter fade_out_amount;
    extern Parameters fade_out_params;


    extern Signature adjust_color_sig;
    extern Parameter adjust_color_color;
    extern Parameter adjust_color_red;
    extern Parameter adjust_color_green;
    extern Parameter adjust_color_blue;
    extern Parameter adjust_color_hue;
    extern Parameter adjust_color_saturation;
    extern Parameter adjust_color_lightness;
    extern Parameter adjust_color_alpha;
    extern Parameters adjust_color_params;
    extern Boolean adjust_color_red_default;
    extern Boolean adjust_color_green_default;
    extern Boolean adjust_color_blue_default;
    extern Boolean adjust_color_hue_default;
    extern Boolean adjust_color_saturation_default;
    extern Boolean adjust_color_lightness_default;
    extern Boolean adjust_color_alpha_default;

    extern Signature scale_color_sig;
    extern Parameter scale_color_color;
    extern Parameter scale_color_red;
    extern Parameter scale_color_green;
    extern Parameter scale_color_blue;
    extern Parameter scale_color_hue;
    extern Parameter scale_color_saturation;
    extern Parameter scale_color_lightness;
    extern Parameter scale_color_alpha;
    extern Parameters scale_color_params;
    extern Boolean scale_color_red_default;
    extern Boolean scale_color_green_default;
    extern Boolean scale_color_blue_default;
    extern Boolean scale_color_hue_default;
    extern Boolean scale_color_saturation_default;
    extern Boolean scale_color_lightness_default;
    extern Boolean scale_color_alpha_default;

    extern Signature change_color_sig;
    extern Parameter change_color_color;
    extern Parameter change_color_red;
    extern Parameter change_color_green;
    extern Parameter change_color_blue;
    extern Parameter change_color_hue;
    extern Parameter change_color_saturation;
    extern Parameter change_color_lightness;
    extern Parameter change_color_alpha;
    extern Parameters change_color_params;
    extern Boolean change_color_red_default;
    extern Boolean change_color_green_default;
    extern Boolean change_color_blue_default;
    extern Boolean change_color_hue_default;
    extern Boolean change_color_saturation_default;
    extern Boolean change_color_lightness_default;
    extern Boolean change_color_alpha_default;

    extern Signature ie_hex_str_sig;
    extern Parameter ie_hex_str_color;
    extern Parameters ie_hex_str_params;

    extern Signature unquote_sig;
    extern Parameter unquote_string;
    extern Parameters unquote_params;

    extern Signature quote_sig;
    extern Parameter quote_string;
    extern Parameters quote_params;

    extern Signature str_length_sig;
    extern Parameter str_length_string;
    extern Parameters str_length_params;

    extern Signature str_insert_sig;
    extern Parameter str_insert_string;
    extern Parameter str_insert_insert;
    extern Parameter str_insert_index;
    extern Parameters str_insert_params;

    extern Signature str_index_sig;
    extern Parameter str_index_string;
    extern Parameter str_index_substring;
    extern Parameters str_index_params;

    extern Signature str_slice_sig;
    extern Parameter str_slice_string;
    extern Parameter str_slice_start_at;
    extern Parameter str_slice_end_at;
    extern Parameters str_slice_params;
    extern Number str_slice_end_at_default;

    extern Signature to_upper_case_sig;
    extern Parameter to_upper_case_string;
    extern Parameters to_upper_case_params;

    extern Signature to_lower_case_sig;
    extern Parameter to_lower_case_string;
    extern Parameters to_lower_case_params;

    extern Signature percentage_sig;
    extern Parameter percentage_number;
    extern Parameters percentage_params;

    extern Signature round_sig;
    extern Parameter round_number;
    extern Parameters round_params;

    extern Signature ceil_sig;
    extern Parameter ceil_number;
    extern Parameters ceil_params;

    extern Signature floor_sig;
    extern Parameter floor_number;
    extern Parameters floor_params;

    extern Signature abs_sig;
    extern Parameter abs_number;
    extern Parameters abs_params;

    extern Signature min_sig;
    extern Parameter min_numbers;
    extern Parameters min_params;

    extern Signature max_sig;
    extern Parameter max_numbers;
    extern Parameters max_params;

    extern Signature random_sig;
    extern Parameter random_limit;
    extern Parameters random_params;
    extern Boolean random_limit_default;

    extern Signature length_sig;
    extern Parameter length_list;
    extern Parameters length_params;

    extern Signature nth_sig;
    extern Parameter nth_list;
    extern Parameter nth_n;
    extern Parameters nth_params;

    extern Signature set_nth_sig;
    extern Parameter set_nth_list;
    extern Parameter set_nth_n;
    extern Parameter set_nth_value;
    extern Parameters set_nth_params;

    extern Signature index_sig;
    extern Parameter index_list;
    extern Parameter index_value;
    extern Parameters index_params;

    extern Signature join_sig;
    extern Parameter join_list_1;
    extern Parameter join_list_2;
    extern Parameter join_separator;
    extern Parameters join_params;
    extern String_Constant join_separator_default;

    extern Signature append_sig;
    extern Parameter append_list;
    extern Parameter append_val;
    extern Parameter append_separator;
    extern Parameters append_params;
    extern String_Constant append_separator_default;

    extern Signature zip_sig;
    extern Parameter zip_lists;
    extern Parameters zip_params;

    extern Signature list_separator_sig;
    extern Parameter list_separator_lists;
    extern Parameters list_separator_params;

    extern Signature map_get_sig;
    extern Parameter map_get_map;
    extern Parameter map_get_key;
    extern Parameters map_get_params;

    extern Signature map_has_key_sig;
    extern Parameter map_has_key_map;
    extern Parameter map_has_key_key;
    extern Parameters map_has_key_params;

    extern Signature map_keys_sig;
    extern Parameter map_keys_map;
    extern Parameters map_keys_params;

    extern Signature map_values_sig;
    extern Parameter map_values_map;
    extern Parameters map_values_params;

    extern Signature map_merge_sig;
    extern Parameter map_merge_map_1;
    extern Parameter map_merge_map_2;
    extern Parameters map_merge_params;

    extern Signature map_remove_sig;
    extern Parameter map_remove_map;
    extern Parameter map_remove_keys;
    extern Parameters map_remove_params;

    extern Signature keywords_sig;
    extern Parameter keywords_args;
    extern Parameters keywords_params;

    extern Signature type_of_sig;
    extern Parameter type_of_value;
    extern Parameters type_of_params;

    extern Signature unit_sig;
    extern Parameter unit_number;
    extern Parameters unit_params;

    extern Signature unitless_sig;
    extern Parameter unitless_number;
    extern Parameters unitless_params;

    extern Signature comparable_sig;
    extern Parameter comparable_number_1;
    extern Parameter comparable_number_2;
    extern Parameters comparable_params;

    extern Signature variable_exists_sig;
    extern Parameter variable_exists_name;
    extern Parameters variable_exists_params;

    extern Signature global_variable_exists_sig;
    extern Parameter global_variable_exists_name;
    extern Parameters global_variable_exists_params;

    extern Signature function_exists_sig;
    extern Parameter function_exists_name;
    extern Parameters function_exists_params;

    extern Signature mixin_exists_sig;
    extern Parameter mixin_exists_name;
    extern Parameters mixin_exists_params;

    extern Signature feature_exists_sig;
    extern Parameter feature_exists_name;
    extern Parameters feature_exists_params;

    extern Signature call_sig;
    extern Parameter call_name;
    extern Parameter call_args;
    extern Parameters call_params;

    extern Signature not_sig;
    extern Parameters not_params;
    extern Signature if_sig;
    extern Parameters if_params;

    extern Signature inspect_sig;
    extern Parameters inspect_params;

    extern Signature unique_id_sig;
    extern Parameters unique_id_params;

    extern Signature selector_nest_sig;
    extern Parameter selector_nest_selectors;
    extern Parameters selector_nest_params;

    extern Signature selector_append_sig;
    extern Parameter selector_append_selectors;
    extern Parameters selector_append_params;

    extern Signature selector_unify_sig;
    extern Parameter selector_unify_selector_1;
    extern Parameter selector_unify_selector_2;
    extern Parameters selector_unify_params;

    extern Signature simple_selectors_sig;
    extern Parameter simple_selectors_selector;
    extern Parameters simple_selectors_params;

    extern Signature selector_extend_sig;
    extern Parameter selector_extend_selector;
    extern Parameter selector_extend_extendee;
    extern Parameter selector_extend_extender;
    extern Parameters selector_extend_params;

    extern Signature selector_replace_sig;
    extern Parameter selector_replace_selector;
    extern Parameter selector_replace_original;
    extern Parameter selector_replace_replacement;
    extern Parameters selector_replace_params;

    extern Signature selector_parse_sig;
    extern Parameter selector_parse_selector;
    extern Parameters selector_parse_params;

    extern Signature is_superselector_sig;
    extern Parameter is_superselector_super;
    extern Parameter is_superselector_sub;
    extern Parameters is_superselector_params;

    BUILT_IN(rgb);
    BUILT_IN(rgba_4);
    BUILT_IN(rgba_2);
    BUILT_IN(red);
    BUILT_IN(green);
    BUILT_IN(blue);
    BUILT_IN(mix);
    BUILT_IN(hsl);
    BUILT_IN(hsla);
    BUILT_IN(hue);
    BUILT_IN(saturation);
    BUILT_IN(lightness);
    BUILT_IN(adjust_hue);
    BUILT_IN(lighten);
    BUILT_IN(darken);
    BUILT_IN(saturate);
    BUILT_IN(desaturate);
    BUILT_IN(grayscale);
    BUILT_IN(complement);
    BUILT_IN(invert);
    BUILT_IN(alpha);
    BUILT_IN(opacify);
    BUILT_IN(transparentize);
    BUILT_IN(adjust_color);
    BUILT_IN(scale_color);
    BUILT_IN(change_color);
    BUILT_IN(ie_hex_str);
    BUILT_IN(sass_unquote);
    BUILT_IN(sass_quote);
    BUILT_IN(str_length);
    BUILT_IN(str_insert);
    BUILT_IN(str_index);
    BUILT_IN(str_slice);
    BUILT_IN(to_upper_case);
    BUILT_IN(to_lower_case);
    BUILT_IN(percentage);
    BUILT_IN(round);
    BUILT_IN(ceil);
    BUILT_IN(floor);
    BUILT_IN(abs);
    BUILT_IN(min);
    BUILT_IN(max);
    BUILT_IN(inspect);
    BUILT_IN(random);
    BUILT_IN(length);
    BUILT_IN(nth);
    BUILT_IN(index);
    BUILT_IN(join);
    BUILT_IN(append);
    BUILT_IN(zip);
    BUILT_IN(list_separator);
    BUILT_IN(type_of);
    BUILT_IN(unit);
    BUILT_IN(unitless);
    BUILT_IN(comparable);
    BUILT_IN(variable_exists);
    BUILT_IN(global_variable_exists);
    BUILT_IN(function_exists);
    BUILT_IN(mixin_exists);
    BUILT_IN(feature_exists);
    BUILT_IN(call);
    BUILT_IN(sass_not);
    BUILT_IN(sass_if);
    BUILT_IN(image_url);
    BUILT_IN(map_get);
    BUILT_IN(map_merge);
    BUILT_IN(map_remove);
    BUILT_IN(map_keys);
    BUILT_IN(map_values);
    BUILT_IN(map_has_key);
    BUILT_IN(keywords);
    BUILT_IN(set_nth);
    BUILT_IN(unique_id);
    BUILT_IN(selector_nest);
    BUILT_IN(selector_append);
    BUILT_IN(selector_extend);
    BUILT_IN(selector_replace);
    BUILT_IN(selector_unify);
    BUILT_IN(is_superselector);
    BUILT_IN(simple_selectors);
    BUILT_IN(selector_parse);
  }
}

#endif
