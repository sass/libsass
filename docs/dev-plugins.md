# LibSass plugins

Plugins are shared object files (.so on *nix and .dll on win) that can be
loaded by LibSass on runtime. Each plugin must have a main entry point
`libsass_init_plugin(struct SassCompiler* compiler)`. There you can add
new variables, custom functions or do anything you want with the compiler.

## plugin.cpp

```C++
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <sass.h>

struct SassValue* ADDCALL call_fn_foo(struct SassValue* s_args, struct SassCompiler* comp, void* cookie)
{
  // we actually abuse the void* to store an "int"
  return sass_make_number((intptr_t)cookie, "px");
}

extern "C" const char* ADDCALL libsass_get_version() {
  return libsass_version();
}

// entry point for libsass to request custom functions from plugin
extern "C" void ADDCALL libsass_init_plugin(struct SassCompiler* compiler)
{

  // Add constants via custom headers
  sass_compiler_add_custom_function(compiler,
    sass_make_function("foo()", call_fn_foo, (void*)42));
}
```

To compile the plugin you need to have LibSass already built as a shared library
(to link against it). The commands below expect the shared library in the `lib`
sub-directory (`-Llib`). The plugin and the main LibSass process should "consume"
the same shared LibSass library on runtime. It will probably also work if they
use different LibSass versions. In this case we check if the major versions are
compatible (i.e. 3.1.3 and 3.1.1 would be considered compatible).

## Compile with gcc on linux

```bash
g++ -O2 -shared plugin.cpp -o plugin.so -fPIC -Llib -lsass
```

## Compile with mingw on windows

```bash
g++ -O2 -shared plugin.cpp -o plugin.dll -Llib -lsass
```
