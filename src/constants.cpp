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
			extern const unsigned char utf_8[] = {0xEF, 0xBB, 0xBF};
			extern const unsigned char utf_16_be[] = {0xFE, 0xFF};
			extern const unsigned char utf_16_le[] = {0xFF, 0xFE};
			extern const unsigned char utf_32_be[] = {0x00, 0x00, 0xFE, 0xFF};
			extern const unsigned char utf_32_le[] = {0xFF, 0xFE, 0x00, 0x00};
			extern const unsigned char utf_7_1[] = {0x2B, 0x2F, 0x76, 0x38};
			extern const unsigned char utf_7_2[] = {0x2B, 0x2F, 0x76, 0x39};
			extern const unsigned char utf_7_3[] = {0x2B, 0x2F, 0x76, 0x2B};
			extern const unsigned char utf_7_4[] = {0x2B, 0x2F, 0x76, 0x2F};
			extern const unsigned char utf_7_5[] = {0x2B, 0x2F, 0x76, 0x38, 0x2D};
			extern const unsigned char utf_1[] = {0xF7, 0x64, 0x4C};
			extern const unsigned char utf_ebcdic[] = {0xDD, 0x73, 0x66, 0x73};
			extern const unsigned char scsu[] = {0x0E, 0xFE, 0xFF};
			extern const unsigned char bocu_1[] = {0xFB, 0xEE, 0x28};
			extern const unsigned char gb_18030[] = {0x84, 0x31, 0x95, 0x33};
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
			const char bold_red[] = "\033[1;31m";
			const char bold_green[] = "\033[1;32m";
			const char bold_yellow[] = "\033[1;33m";
			const char bold_blue[] = "\033[1;34m";
			const char bold_magenta[] = "\033[1;35m";
			const char bold_cyan[] = "\033[1;36m";
			const char bg_red[] = "\033[41m";
			const char bg_green[] = "\033[42m";
			const char bg_yellow[] = "\033[43m";
			const char bg_blue[] = "\033[44m";
			const char bg_magenta[] = "\033[45m";
			const char bg_cyan[] = "\033[46m";
		}

    namespace String {

      const char empty[] = "";

    }


  }
}
