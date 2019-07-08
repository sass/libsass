#ifndef SASS_STRINGS_HPP
#define SASS_STRINGS_HPP

// sass.hpp must go before all system headers to get the
// __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

// Include normalized keys
#include "environment_key.hpp"

namespace Sass {

  namespace Strings {

    extern const sass::string empty;

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
    extern const sass::string error;
    extern const sass::string warning;

    extern const sass::string forRule;
    extern const sass::string warnRule;
    extern const sass::string errorRule;
    extern const sass::string debugRule;
    extern const sass::string extendRule;
    extern const sass::string importRule;
    extern const sass::string contentRule;

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

    extern const EnvKey red;
    extern const EnvKey hue;
    extern const EnvKey blue;
    extern const EnvKey green;
    extern const EnvKey alpha;
    extern const EnvKey color;
    extern const EnvKey lightness;
    extern const EnvKey saturation;
    extern const EnvKey whiteness;
    extern const EnvKey blackness;
    

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
