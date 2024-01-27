/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_FUNCTION_H
#define SASS_FUNCTION_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Type definition for custom functions
  typedef struct SassValue* (*SassFunctionLambda)(
    struct SassValue*, struct SassCompiler* compiler, void* cookie);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create custom function (with arbitrary data pointer called `cookie`)
  // The pointer is often used to store the callback into the actual binding.
  ADDAPI struct SassFunction* ADDCALL sass_make_function (const char* signature, SassFunctionLambda lambda, void* cookie);

  // Deallocate custom function and release memory
  ADDAPI void ADDCALL sass_delete_function (struct SassFunction* entry);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for custom function signature.
  ADDAPI const char* ADDCALL sass_function_get_signature (struct SassFunction* function);

  // Getter for custom function lambda.
  ADDAPI SassFunctionLambda ADDCALL sass_function_get_lambda (struct SassFunction* function);

  // Getter for custom function data cookie.
  ADDAPI void* ADDCALL sass_function_get_cookie (struct SassFunction* function);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
