## Example main.c

```C
#include <sass/error.h>
#include <sass/values.h>
#include <sass/import.h>
#include <sass/compiler.h>

struct Cookie {
  double number;
};

// Note: some compilers allow to directly use `struct Cookie*` instead of `void*` for the cookie ptr.
struct SassValue* call_fn_foo(struct SassValue* s_args, struct SassCompiler* compiler, void* cookie)
{
  // Statically cast to type we passed the cookie
  struct Cookie* casted = (struct Cookie*)cookie;
  // Now we can access whatever we put into the cookie
  return sass_make_number(casted->number, "px");
}

int main(int argc, const char* argv[])
{

  // Get the input file from first argument or use default
  const char* input = argc > 1 ? argv[1] : "styles.scss";

  // Create the main compiler object instance
  struct SassCompiler* compiler = sass_make_compiler();

  // Cookie structure to attach to function
  // Allows you to pass anything you want
  struct Cookie cookie = { 42 };

  // This may fail and produce an error if
  // the passed signature cannot be parsed.
  sass_compiler_add_custom_function(compiler,
    sass_make_function("foo()", call_fn_foo, (void*)&cookie));

  // Create import, set as entry point and release our usage
  struct SassImport* import = sass_make_file_import(input);
  sass_compiler_set_entry_point(compiler, import);
  sass_delete_import(import);

  // Execute compiler and print/write results
  sass_compiler_execute(compiler, false);

  // Get result code after all compilation steps
  int status = sass_compiler_get_status(compiler);

  // Clean-up compiler, we're done
  sass_delete_compiler(compiler);

  // exit status
  return status;
}
```

### Compile main.c

```bash
gcc -c main.c -o main.o
g++ -o sample main.o -lsass
echo "foo { margin: foo(); }" > foo.scss
./sample foo.scss => "foo { margin: 42px }"
```

