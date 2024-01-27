/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
/* cencode.h - c source to a base64 encoding algorithm implementation        */
/* Part of the libb64 project, and has been placed in the public domain.     */
/* For details, see http://sourceforge.net/projects/libb64                   */
/*****************************************************************************/

#ifndef BASE64_CENCODE_HPP
#define BASE64_CENCODE_HPP

#ifdef _MSC_VER
#pragma warning(disable : 26812)
#endif

namespace base64 {

  typedef enum
  {
    step_A, step_B, step_C
  } base64_encodestep;

  typedef struct
  {
    base64_encodestep step;
    char result;
    int stepcount;
  } base64_encodestate;

  void base64_init_encodestate(base64_encodestate* state_in);

  char base64_encode_value(char value_in);

  int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in);

  int base64_encode_blockend(char* code_out, base64_encodestate* state_in);

}

#endif /* BASE64_CENCODE_H */

