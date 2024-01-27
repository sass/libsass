## Example with data import

```C:data.c
#include <stdio.h>
#include <sass.h>

int main(int argc, const char* argv[])
{

  // LibSass will take control of data you pass in
  // Therefore we need to make a copy of static data
  char* text = sass_copy_c_string("a{b:c;}");
  // Normally you'll load data into a buffer from i.e. the disk.
  // Use `sass_alloc_memory` to get a buffer to pass to LibSass
  // then fill it with data you load from disk or somewhere else.

  // create compiler object holding config and states
  struct SassCompiler* compiler = sass_make_compiler();
  // we've set the input name to "styles" here, which means ...
  struct SassImport* data = sass_make_content_import(text, "styles");
  // ... we can't deduct the type automatically from the extension.
  sass_import_set_syntax(data, SASS_IMPORT_SCSS);
  // each compiler must have exactly one entry point
  sass_compiler_set_entry_point(compiler, data);
  // entry point now passed to compiler, so its reference count was increased
  // in order to not leak memory we must release our own usage (usage after is UB)
  sass_delete_import(data); // decrease ref-count

  // Execute all three phases
  sass_compiler_parse(compiler);
  sass_compiler_compile(compiler);
  sass_compiler_render(compiler);

  // Print any warning to console
  if (sass_compiler_get_warn_string(compiler)) {
    sass_print_stderr(sass_compiler_get_warn_string(compiler));
  }

  // Print error message if we have an error
  if (sass_compiler_get_status(compiler) != 0) {
    const struct SassError* error = sass_compiler_get_error(compiler);
    sass_print_stderr(sass_error_get_string(error));
  }

  // Get result code after all compilation steps
  int status = sass_compiler_get_status(compiler);

  // Write to output if no errors occurred
  if (status == 0) {

    // Check if config tells us to write some files
    bool writeOutput = sass_compiler_has_output_file(compiler);
    bool writeSrcMap = sass_compiler_has_srcmap_file(compiler);
    // Paths where to write stuff to (might be `stream://stdout`)
    const char* outfile = sass_compiler_get_output_path(compiler);
    const char* mapfile = sass_compiler_get_srcmap_path(compiler);
    // Get the parts to be added to the output file (or stdout)
    const char* content = sass_compiler_get_output_string(compiler);
    const char* footer = sass_compiler_get_footer_string(compiler);
    const char* srcmap = sass_compiler_get_srcmap_string(compiler);

    // Output all results
    if (content) puts(content);
    if (footer) puts(footer);

  }

  // exit status
  return status;

}
```

### Compile data.c

```bash
gcc -c data.c -o data.o
gcc -o sample data.o -lsass
echo "foo { margin: 21px * 2; }" > foo.scss
./sample foo.scss => "foo { margin: 42px }"
```

## Example for file import

```C:file.c
#include <stdio.h>
#include <sass.h>

int main(int argc, const char* argv[])
{

  // create compiler object holding config and states
  struct SassCompiler* compiler = sass_make_compiler();
  // set input name with extension so we can deduct type from it
  struct SassImport* import = sass_make_file_import("foo.scss");
  // each compiler must have exactly one entry point
  sass_compiler_set_entry_point(compiler, import);
  // entry point now passed to compiler, so its reference count was increased
  // in order to not leak memory we must release our own usage (usage after is UB)
  sass_delete_import(import); // decrease ref-count

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

### Compile file.c

```bash
gcc -c file.c -o file.o
gcc -o sample file.o -lsass
echo "foo { margin: 21px * 2; }" > foo.scss
./sample foo.scss => "foo { margin: 42px }"
```

