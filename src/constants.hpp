/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_CONSTANTS_HPP
#define SASS_CONSTANTS_HPP

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

namespace Sass
{

	namespace Constants
	{

		// https://github.com/sass/libsass/issues/592
		// https://developer.mozilla.org/en-US/docs/Web/CSS/Specificity
		// https://github.com/sass/sass/issues/1495#issuecomment-61189114
		namespace Specificity
		{

			extern const unsigned long Star;
			extern const unsigned long Universal;
			extern const unsigned long Element;
			extern const unsigned long Base;
			extern const unsigned long Class;
			extern const unsigned long Attr;
			extern const unsigned long Pseudo;
			extern const unsigned long ID;

    } // namespace Specificity

		// http://en.wikipedia.org/wiki/Byte_order_mark
		namespace BOM
		{

      extern const unsigned char utf_8[];
			extern const unsigned char utf_16_be[];
			extern const unsigned char utf_16_le[];
			extern const unsigned char utf_32_be[];
			extern const unsigned char utf_32_le[];
			extern const unsigned char utf_7_1[];
			extern const unsigned char utf_7_2[];
			extern const unsigned char utf_7_3[];
			extern const unsigned char utf_7_4[];
			extern const unsigned char utf_7_5[];
			extern const unsigned char utf_1[];
			extern const unsigned char utf_ebcdic[];
			extern const unsigned char scsu[];
			extern const unsigned char bocu_1[];
			extern const unsigned char gb_18030[];

    } // namespace BOM

		namespace Terminal
		{

			extern const char reset[];
			extern const char bold[];
			extern const char red[];
			extern const char green[];
			extern const char yellow[];
			extern const char blue[];
			extern const char magenta[];
			extern const char cyan[];
			extern const char bold_red[];
			extern const char bold_green[];
			extern const char bold_yellow[];
			extern const char bold_blue[];
			extern const char bold_magenta[];
			extern const char bold_cyan[];
			extern const char bg_red[];
			extern const char bg_green[];
			extern const char bg_yellow[];
			extern const char bg_blue[];
			extern const char bg_magenta[];
			extern const char bg_cyan[];

		} // namespace Terminal

		namespace String
		{

			extern const char empty[];

		} // namespace String

	} // namespace Constants

} // namespace Sass

#endif
