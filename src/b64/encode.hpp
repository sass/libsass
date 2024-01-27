/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* encode.hpp - c source to a base64 encoding algorithm implementation       */
/* Part of the libb64 project, and has been placed in the public domain.     */
/* For details, see http://sourceforge.net/projects/libb64                   */
/*****************************************************************************/

#ifndef BASE64_ENCODE_HPP
#define BASE64_ENCODE_HPP

#include <iostream>
#include "cencode.hpp"

#ifndef BASE64_BUFFERSIZE
#define BASE64_BUFFERSIZE 255
#endif

namespace base64
{
	struct encoder
	{
		base64_encodestate _state;
		int _buffersize;

		encoder(int buffersize_in = BASE64_BUFFERSIZE)
		: _buffersize(buffersize_in)
		{
			base64_init_encodestate(&_state);
		}

		int encode(char value_in)
		{
			return base64_encode_value(value_in);
		}

		int encode(const char* code_in, const int length_in, char* plaintext_out)
		{
			return base64_encode_block(code_in, length_in, plaintext_out, &_state);
		}

		int encode_end(char* plaintext_out)
		{
			return base64_encode_blockend(plaintext_out, &_state);
		}

		void encode(std::istream& istream_in, std::ostream& ostream_in)
		{
			base64_init_encodestate(&_state);
			//
      size_t N = _buffersize;
			char* plaintext = new char[N];
			char* code = new char[N*2];
			int plainlength;
			int codelength;

			do
			{
				istream_in.read(plaintext, N);
				plainlength = static_cast<int>(istream_in.gcount());
				//
				codelength = encode(plaintext, plainlength, code);
				ostream_in.write(code, codelength);
			}
			while (istream_in.good() && plainlength > 0);

			codelength = encode_end(code);
			ostream_in.write(code, codelength);
			//
			base64_init_encodestate(&_state);

			delete [] code;
			delete [] plaintext;
		}
	};

} // namespace base64

#endif // BASE64_ENCODE_H

