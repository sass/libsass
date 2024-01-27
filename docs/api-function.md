## LibSass custom functions C-API

Sass functions are used to define custom functions callable by Sass code.
They are also used to overload `@debug`, `@warn` or `@error` rules. Additionally you
can define a fallback function, which is called for every unknown function found in
your Sass code. Functions get passed zero or more `SassValues` (a `SassList` value)
and they must also return a `SassValue`. Any custom function may also return
a `SassError` value to signal an error during execution.

## Special signatures

- `*` - Fallback implementation
- `@warn` - Overload warn statements
- `@error` - Overload error statements
- `@debug` - Overload debug statements

Note: The fallback implementation will be given the name of the called function
as the first argument, before all the original function arguments. These features
are pretty new and should be considered experimental.

### Example code

See [sass function code example](api-function-example.md).

### Basic Usage

```C
#include <sass/function.h>
```

## Sass Function API

```C
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Type definition for custom functions
typedef struct SassValue* (*SassFunctionLambda)(
  struct SassValue*, struct SassCompiler* compiler, void* cookie);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Create custom function (with arbitrary data pointer called `cookie`)
// The pointer is often used to store the callback into the actual binding.
struct SassFunction* sass_make_function (const char* signature, SassFunctionLambda lambda, void* cookie);

// Deallocate custom function and release memory
void sass_delete_function (struct SassFunction* entry);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Getter for custom function signature.
const char* sass_function_get_signature (struct SassFunction* function);

// Getter for custom function lambda.
SassFunctionLambda sass_function_get_lambda (struct SassFunction* function);

// Getter for custom function data cookie.
void* sass_function_get_cookie (struct SassFunction* function);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
```
