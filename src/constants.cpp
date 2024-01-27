/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "constants.hpp"

#include "terminal.hpp"

namespace Sass {

  namespace Constants {

    // https://github.com/sass/libsass/issues/592
    // https://developer.mozilla.org/en-US/docs/Web/CSS/Specificity
    // https://github.com/sass/sass/issues/1495#issuecomment-61189114
    namespace Specificity
    {
      extern const unsigned long Star = 0;
      extern const unsigned long Universal = 0;
      extern const unsigned long Element = 1;
      extern const unsigned long Base = 1000;
      extern const unsigned long Class = 1000;
      extern const unsigned long Attr = 1000;
      extern const unsigned long Pseudo = 1000;
      extern const unsigned long ID = 1000000;
    }


    // http://en.wikipedia.org/wiki/Byte_order_mark
    namespace BOM {
      extern const unsigned char utf_8[] = { 0xEF, 0xBB, 0xBF };
      extern const unsigned char utf_16_be[] = { 0xFE, 0xFF };
      extern const unsigned char utf_16_le[] = { 0xFF, 0xFE };
      extern const unsigned char utf_32_be[] = { 0x00, 0x00, 0xFE, 0xFF };
      extern const unsigned char utf_32_le[] = { 0xFF, 0xFE, 0x00, 0x00 };
      extern const unsigned char utf_7_1[] = { 0x2B, 0x2F, 0x76, 0x38 };
      extern const unsigned char utf_7_2[] = { 0x2B, 0x2F, 0x76, 0x39 };
      extern const unsigned char utf_7_3[] = { 0x2B, 0x2F, 0x76, 0x2B };
      extern const unsigned char utf_7_4[] = { 0x2B, 0x2F, 0x76, 0x2F };
      extern const unsigned char utf_7_5[] = { 0x2B, 0x2F, 0x76, 0x38, 0x2D };
      extern const unsigned char utf_1[] = { 0xF7, 0x64, 0x4C };
      extern const unsigned char utf_ebcdic[] = { 0xDD, 0x73, 0x66, 0x73 };
      extern const unsigned char scsu[] = { 0x0E, 0xFE, 0xFF };
      extern const unsigned char bocu_1[] = { 0xFB, 0xEE, 0x28 };
      extern const unsigned char gb_18030[] = { 0x84, 0x31, 0x95, 0x33 };
    }


    namespace Terminal {
      const char reset[] = "\033[m";
      const char bold[] = "\033[1m";
      const char red[] = "\033[31m";
      const char green[] = "\033[32m";
      const char yellow[] = "\033[33m";
      const char blue[] = "\033[34m";
      const char magenta[] = "\033[35m";
      const char cyan[] = "\033[36m";
      const char white[] = "\033[37m";
      // Intensified on windows
      const char bold_red[] = "\033[1;31m";
      const char bold_green[] = "\033[1;32m";
      const char bold_yellow[] = "\033[1;33m";
      const char bold_blue[] = "\033[1;34m";
      const char bold_magenta[] = "\033[1;35m";
      const char bold_cyan[] = "\033[1;36m";
      const char bold_white[] = "\033[1;37m";
      const char bg_red[] = "\033[41m";
      const char bg_green[] = "\033[42m";
      const char bg_yellow[] = "\033[43m";
      const char bg_blue[] = "\033[44m";
      const char bg_magenta[] = "\033[45m";
      const char bg_cyan[] = "\033[46m";
      const char bg_white[] = "\033[47m";
      // Intensified on windows
      const char bg_bold_red[] = "\033[1;41m";
      const char bg_bold_green[] = "\033[1;42m";
      const char bg_bold_yellow[] = "\033[1;43m";
      const char bg_bold_blue[] = "\033[1;44m";
      const char bg_bold_magenta[] = "\033[1;45m";
      const char bg_bold_cyan[] = "\033[1;46m";
      const char bg_bold_white[] = "\033[1;47m";

    }

    namespace String {

      const char empty[] = "";

    }

    namespace Math {

      const double M_E = 2.71828182845904523536; // e
      const double M_LOG2E = 1.44269504088896340736; // log2(e)
      const double M_LOG10E = 0.434294481903251827651; // log10(e)
      const double M_LN2 = 0.693147180559945309417; // ln(2)
      const double M_LN10 = 2.30258509299404568402; // ln(10)
      const double M_PI = 3.14159265358979323846; // pi
      const double M_PI_2 = 1.57079632679489661923; // pi/2
      const double M_PI_4 = 0.785398163397448309616; // pi/4
      const double M_1_PI = 0.318309886183790671538; // 1/pi
      const double M_2_PI = 0.636619772367581343076; // 2/pi
      const double M_2_SQRTPI = 1.12837916709551257390; // 2/sqrt(pi)
      const double M_SQRT2 = 1.41421356237309504880; // sqrt(2)
      const double M_SQRT1_2 = 0.707106781186547524401; // 1/sqrt(2)
      const double RAD_TO_DEG = 57.295779513082320876798154814105;

    }

  }
}
