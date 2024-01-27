/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_function.hpp"

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Sass;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create custom function (with arbitrary data pointer called `cookie`)
  // The pointer is often used to store the callback into the actual binding.
  struct SassFunction* ADDCALL sass_make_function(const char* signature, SassFunctionLambda lambda, void* cookie)
  {
    if (lambda == nullptr) return nullptr;
    if (signature == nullptr) return nullptr;
    struct SassFunction* function = new SassFunction{};
    if (function == nullptr) return nullptr;
    function->lambda = lambda;
    function->signature = signature;
    function->cookie = cookie;
    return function;
  }

  // Deallocate custom function and release memory
  void ADDCALL sass_delete_function(struct SassFunction* function)
  {
    delete function;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for custom function lambda.
  SassFunctionLambda ADDCALL sass_function_get_lambda(struct SassFunction* function)
  {
    return function->lambda;
  }

  // Getter for custom function signature.
  const char* ADDCALL sass_function_get_signature(struct SassFunction* function)
  {
    return function->signature.c_str();
  }

  // Getter for custom function data cookie.
  void* ADDCALL sass_function_get_cookie(struct SassFunction* function)
  {
    return function->cookie;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
