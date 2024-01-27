/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_STRINGS_HPP
#define SASS_STRINGS_HPP

// sass.hpp must go before all system headers
// to get the  __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// Include normalized keys
#include "environment_key.hpp"

#include "units.hpp"

namespace Sass {

  extern const sass::string str_empty;

  // For list functions
  extern const sass::string str_length;
  extern const sass::string str_nth;
  extern const sass::string str_set_nth;
  extern const sass::string str_join;
  extern const sass::string str_append;
  extern const sass::string str_zip;
  extern const sass::string str_slash;
  extern const sass::string str_list_separator;
  extern const sass::string str_is_bracketed;

  // Rounding strategies
  extern const sass::string str_up;
  extern const sass::string str_down;
  extern const sass::string str_nearest;
  extern const sass::string str_to_zero;

  // For map functions
  extern const sass::string str_set;
  extern const sass::string str_map_set;
  extern const sass::string str_get;
  extern const sass::string str_map_get;
  extern const sass::string str_merge;
  extern const sass::string str_map_merge;
  extern const sass::string str_remove;
  extern const sass::string str_map_remove;
  extern const sass::string str_keys;
  extern const sass::string str_map_keys;
  extern const sass::string str_base;
  extern const sass::string str_deg;
  extern const sass::string str_angle;
  extern const sass::string str_number;
  extern const sass::string str_value;
  extern const sass::string str_values;
  extern const sass::string str_map_values;
  extern const sass::string str_has_key;
  extern const sass::string str_map_has_key;
  extern const sass::string str_deep_merge;
  extern const sass::string str_deep_remove;

  // For text functions
  extern const sass::string str_unquote;
  extern const sass::string str_quote;
  extern const sass::string str_to_upper_case;
  extern const sass::string str_to_lower_case;
  // extern const sass::string str_length;
  extern const sass::string str_str_length;
  extern const sass::string str_insert;
  extern const sass::string str_str_insert;
  extern const sass::string str_index;
  extern const sass::string str_str_index;
  extern const sass::string str_slice;
  extern const sass::string str_split;
  extern const sass::string str_str_slice;
  extern const sass::string str_str_split;
  extern const sass::string str_unique_id;

  // For meta functions
  extern const sass::string str_load_css;
  extern const sass::string str_feature_exists;
  extern const sass::string str_type_of;
  extern const sass::string str_inspect;
  extern const sass::string str_keywords;
  extern const sass::string str_if;
  extern const sass::string str_apply;
  extern const sass::string str_calc_name;
  extern const sass::string str_calc_args;
  extern const sass::string str_get_mixin;
  extern const sass::string str_module_mixins;
  extern const sass::string str_accepts_content;
  extern const sass::string str_global_variable_exists;
  extern const sass::string str_variable_exists;
  extern const sass::string str_function_exists;
  extern const sass::string str_mixin_exists;
  extern const sass::string str_content_exists;
  extern const sass::string str_module_variables;
  extern const sass::string str_module_functions;
  extern const sass::string str_get_function;
  extern const sass::string str_call;

  // For color functions
  extern const sass::string str_rgb;
  extern const sass::string str_rgba;
  extern const sass::string str_hsl;
  extern const sass::string str_hsla;
  extern const sass::string str_hwb;
  extern const sass::string str_hwba;
  extern const sass::string str_red;
  extern const sass::string str_green;
  extern const sass::string str_blue;
  extern const sass::string str_hue;
  extern const sass::string str_from;
  extern const sass::string str_lightness;
  extern const sass::string str_saturation;
  extern const sass::string str_blackness;
  extern const sass::string str_whiteness;
  extern const sass::string str_invert;
  extern const sass::string str_grayscale;
  extern const sass::string str_complement;
  extern const sass::string str_desaturate;
  extern const sass::string str_saturate;
  extern const sass::string str_lighten;
  extern const sass::string str_darken;
  extern const sass::string str_adjust_hue;
  extern const sass::string str_adjust_color;
  extern const sass::string str_change_color;
  extern const sass::string str_scale_color;
  extern const sass::string str_adjust;
  extern const sass::string str_change;
  extern const sass::string str_scale;
  extern const sass::string str_mix;
  extern const sass::string str_opacify;
  extern const sass::string str_fade_in;
  extern const sass::string str_fade_out;
  extern const sass::string str_transparentize;
  extern const sass::string str_ie_hex_str;
  extern const sass::string str_alpha;
  extern const sass::string str_opacity;

  // For math functions
  extern const sass::string str_e;
  extern const sass::string str_pi;
  extern const sass::string str_tau;
  extern const sass::string str_epsilon;
  extern const sass::string str_min_safe_integer;
  extern const sass::string str_max_safe_integer;
  extern const sass::string str_min_number;
  extern const sass::string str_max_number;
  
  extern const sass::string str_infinity;
  extern const sass::string str_neg_infinity;
  extern const sass::string str_nan;

  extern const sass::string str_ceil;
  extern const sass::string str_calc;
  extern const sass::string str_clamp;
  extern const sass::string str_floor;
  extern const sass::string str_max;
  extern const sass::string str_min;
  extern const sass::string str_round;
  extern const sass::string str_abs;
  extern const sass::string str_exp;
  extern const sass::string str_sign;
  extern const sass::string str_hypot;
  extern const sass::string str_log;
  extern const sass::string str_div;
  extern const sass::string str_pow;
  extern const sass::string str_sqrt;
  extern const sass::string str_cos;
  extern const sass::string str_sin;
  extern const sass::string str_tan;
  extern const sass::string str_acos;
  extern const sass::string str_asin;
  extern const sass::string str_atan;
  extern const sass::string str_atan2;
  extern const sass::string str_mod;
  extern const sass::string str_rem;
  extern const sass::string str_random;
  extern const sass::string str_unit;
  extern const sass::string str_percentage;
  extern const sass::string str_unitless;
  extern const sass::string str_is_unitless;
  extern const sass::string str_compatible;
  extern const sass::string str_comparable;
  extern const sass::string str_rad;

  // For selector functions
  extern const sass::string str_nest;
  extern const sass::string str_selector_nest;
  // extern const sass::string str_append;
  extern const sass::string str_selector_append;
  extern const sass::string str_extend;
  extern const sass::string str_selector_extend;
  extern const sass::string str_replace;
  extern const sass::string str_selector_replace;
  extern const sass::string str_unify;
  extern const sass::string str_selector_unify;
  extern const sass::string str_parse;
  extern const sass::string str_selector_parse;
  extern const sass::string str_is_superselector;
  extern const sass::string str_simple_selectors;



















  // For list functions
  extern const EnvKey key_length;
  extern const EnvKey key_nth;
  extern const EnvKey key_set_nth;
  extern const EnvKey key_join;
  extern const EnvKey key_append;
  extern const EnvKey key_zip;
  extern const EnvKey key_slash;
  extern const EnvKey key_list_separator;
  extern const EnvKey key_is_bracketed;

  // For map functions
  extern const EnvKey key_set;
  extern const EnvKey key_map_set;
  extern const EnvKey key_get;
  extern const EnvKey key_map_get;
  extern const EnvKey key_merge;
  extern const EnvKey key_map_merge;
  extern const EnvKey key_remove;
  extern const EnvKey key_map_remove;
  extern const EnvKey key_keys;
  extern const EnvKey key_map_keys;
  extern const EnvKey key_values;
  extern const EnvKey key_map_values;
  extern const EnvKey key_has_key;
  extern const EnvKey key_map_has_key;
  extern const EnvKey key_deep_merge;
  extern const EnvKey key_deep_remove;

  // For text functions
  extern const EnvKey key_unquote;
  extern const EnvKey key_quote;
  extern const EnvKey key_to_upper_case;
  extern const EnvKey key_to_lower_case;
  // extern const EnvKey key_length;
  extern const EnvKey key_str_length;
  extern const EnvKey key_insert;
  extern const EnvKey key_str_insert;
  extern const EnvKey key_index;
  extern const EnvKey key_str_index;
  extern const EnvKey key_slice;
  extern const EnvKey key_split;
  extern const EnvKey key_str_slice;
  extern const EnvKey key_str_split;
  extern const EnvKey key_unique_id;

  // For meta functions
  extern const EnvKey key_load_css;
  extern const EnvKey key_feature_exists;
  extern const EnvKey key_type_of;
  extern const EnvKey key_inspect;
  extern const EnvKey key_keywords;
  extern const EnvKey key_if;
  extern const EnvKey key_apply;
  extern const EnvKey key_calc_name;
  extern const EnvKey key_calc_args;
  extern const EnvKey key_get_mixin;
  extern const EnvKey key_module_mixins;
  extern const EnvKey key_accepts_content;
  extern const EnvKey key_global_variable_exists;
  extern const EnvKey key_variable_exists;
  extern const EnvKey key_function_exists;
  extern const EnvKey key_mixin_exists;
  extern const EnvKey key_content_exists;
  extern const EnvKey key_module_variables;
  extern const EnvKey key_module_functions;
  extern const EnvKey key_get_function;
  extern const EnvKey key_call;

  // For color functions
  extern const EnvKey key_rgb;
  extern const EnvKey key_rgba;
  extern const EnvKey key_hsl;
  extern const EnvKey key_hsla;
  extern const EnvKey key_hwb;
  extern const EnvKey key_hwba;
  extern const EnvKey key_red;
  extern const EnvKey key_green;
  extern const EnvKey key_blue;
  extern const EnvKey key_hue;
  extern const EnvKey key_lightness;
  extern const EnvKey key_saturation;
  extern const EnvKey key_blackness;
  extern const EnvKey key_whiteness;
  extern const EnvKey key_invert;
  extern const EnvKey key_grayscale;
  extern const EnvKey key_complement;
  extern const EnvKey key_desaturate;
  extern const EnvKey key_saturate;
  extern const EnvKey key_lighten;
  extern const EnvKey key_darken;
  extern const EnvKey key_adjust_hue;
  extern const EnvKey key_adjust_color;
  extern const EnvKey key_change_color;
  extern const EnvKey key_scale_color;
  extern const EnvKey key_adjust;
  extern const EnvKey key_change;
  extern const EnvKey key_scale;
  extern const EnvKey key_mix;
  extern const EnvKey key_opacify;
  extern const EnvKey key_fade_in;
  extern const EnvKey key_fade_out;
  extern const EnvKey key_transparentize;
  extern const EnvKey key_ie_hex_str;
  extern const EnvKey key_alpha;
  extern const EnvKey key_opacity;

  // For math functions
  extern const EnvKey key_e;
  extern const EnvKey key_pi;
  extern const EnvKey key_tau;
  extern const EnvKey key_epsilon;
  extern const EnvKey key_min_safe_integer;
  extern const EnvKey key_max_safe_integer;
  extern const EnvKey key_min_number;
  extern const EnvKey key_max_number;

  extern const EnvKey key_ceil;
  extern const EnvKey key_clamp;
  extern const EnvKey key_floor;
  extern const EnvKey key_max;
  extern const EnvKey key_min;
  extern const EnvKey key_round;
  extern const EnvKey key_abs;
  extern const EnvKey key_hypot;
  extern const EnvKey key_log;
  extern const EnvKey key_div;
  extern const EnvKey key_pow;
  extern const EnvKey key_sqrt;
  extern const EnvKey key_cos;
  extern const EnvKey key_sin;
  extern const EnvKey key_tan;
  extern const EnvKey key_acos;
  extern const EnvKey key_asin;
  extern const EnvKey key_atan;
  extern const EnvKey key_atan2;
  extern const EnvKey key_random;
  extern const EnvKey key_unit;
  extern const EnvKey key_percentage;
  extern const EnvKey key_unitless;
  extern const EnvKey key_is_unitless;
  extern const EnvKey key_compatible;
  extern const EnvKey key_comparable;

  // For selector functions
  extern const EnvKey key_nest;
  extern const EnvKey key_selector_nest;
  // extern const EnvKey key_append;
  extern const EnvKey key_selector_append;
  extern const EnvKey key_extend;
  extern const EnvKey key_selector_extend;
  extern const EnvKey key_replace;
  extern const EnvKey key_selector_replace;
  extern const EnvKey key_unify;
  extern const EnvKey key_selector_unify;
  extern const EnvKey key_parse;
  extern const EnvKey key_selector_parse;
  extern const EnvKey key_is_superselector;
  extern const EnvKey key_simple_selectors;


  extern const Units unit_rad;
  extern const Units unit_deg;
  extern const Units unit_none;
  extern const Units unit_percent;





  namespace Strings {

    extern const sass::string empty;

    // For list functions
    extern const sass::string length;
    extern const sass::string nth;
    extern const sass::string setNth;
    extern const sass::string join;
    extern const sass::string append;
    extern const sass::string zip;
    extern const sass::string listSeparator;
    extern const sass::string isBracketed;

    extern const sass::string plus;
    extern const sass::string minus;
    extern const sass::string percent;

    extern const sass::string rgb;
    extern const sass::string hsl;
    extern const sass::string hwb;
    extern const sass::string rgba;
    extern const sass::string hsla;
    extern const sass::string hwba;

    extern const sass::string deg;
    extern const sass::string red;
    extern const sass::string hue;
    extern const sass::string blue;
    extern const sass::string green;
    extern const sass::string alpha;
    extern const sass::string color;
    extern const sass::string weight;
    extern const sass::string number;
    extern const sass::string amount;
    extern const sass::string invert;
    extern const sass::string degrees;
    extern const sass::string saturate;
    extern const sass::string grayscale;

    extern const sass::string whiteness;
    extern const sass::string blackness;
    extern const sass::string $whiteness;
    extern const sass::string $blackness;

    
    
    extern const sass::string lightness;
    extern const sass::string saturation;
    

    extern const sass::string key;
    extern const sass::string map;
    extern const sass::string map1;
    extern const sass::string map2;
    extern const sass::string args;
    extern const sass::string calc;
    extern const sass::string with;
    extern const sass::string url;
    extern const sass::string list;
    extern const sass::string name;
    extern const sass::string null;
    extern const sass::string media;
    extern const sass::string module;
    extern const sass::string supports;
    extern const sass::string keyframes;

    extern const sass::string boolean;
    extern const sass::string string;
    extern const sass::string arglist;
    extern const sass::string function;
    extern const sass::string calculation;
    extern const sass::string calcoperation;
    extern const sass::string mixin;
    extern const sass::string error;
    extern const sass::string warning;

    extern const sass::string useRule;
    extern const sass::string forRule;
    extern const sass::string warnRule;
    extern const sass::string errorRule;
    extern const sass::string debugRule;
    extern const sass::string extendRule;
    extern const sass::string importRule;
    extern const sass::string contentRule;
    extern const sass::string forwardRule;

    extern const sass::string scaleColor;
    extern const sass::string colorAdjust;
    extern const sass::string colorChange;

    extern const sass::string condition;
    extern const sass::string ifFalse;
    extern const sass::string ifTrue;

    extern const sass::string $red;
    extern const sass::string $green;
    extern const sass::string $blue;

    extern const sass::string $hue;
    extern const sass::string $saturation;
    extern const sass::string $lightness;

    extern const sass::string utf8bom;

    extern const sass::string argument;
    extern const sass::string _and_;
    extern const sass::string _or_;

  }

 
  namespace Keys {




    extern const EnvKey alpha;
    extern const EnvKey color;
    

    extern const EnvKey warnRule;
    extern const EnvKey errorRule;
    extern const EnvKey debugRule;
    extern const EnvKey contentRule;

    extern const EnvKey condition;
    extern const EnvKey ifFalse;
    extern const EnvKey ifTrue;

  }

}


#endif
