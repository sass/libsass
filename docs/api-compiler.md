## Sass Compiler API

Sass Compiler is the main object to facilitate the Sass compilation process.
It holds the configuration, the compilation state and the results. It's the
main object you will interact on the C-API side.

The regular life-cycle of a compiler will mostly look like this:
- Create new compiler object
- Set various configuration options
- Set one compilation entry point
- Call the parse function
- Call the compile function
- Call the render function
- Process the results

The split between parse, compile and render is done because there
are cases where we might want to omit certain phases. One example
would be to only execute the init phase for a watcher to get all
includes first, before triggering the first compilation.

### Compiler entry point

Every compiler must have one entry point. This can either be a file or
a data entry point. See [import api][api-import.md] for further details.

### Writing output files

Implementors may write output files on their own or you can tell
LibSass to write them to the configured locations. The functions
`sass_compiler_write_output` and `sass_compiler_write_srcmap` should
act according to the configuration and the produced results.

### Logger configuration

LibSass supports error reporting with ANSI colors on windows and unix. It can
also make use of Unicode compatible terminals. There is some basic auto-detection
you can trigger by calling `sass_compiler_autodetect_logger_capabilities`. You
can still overload the detected settings afterwards. Additionally LibSass will
try to break running-text into the available columns and shorten stack-traces.

### Example code

See [sass compiler code example](api-compiler-example.md).

### Basic Usage

```C
#include <sass/compiler.h>
```

### Sass Compiler API

```C
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Create a new LibSass compiler context
struct SassCompiler* sass_make_compiler();

// Release all memory allocated with the compiler
void sass_delete_compiler(struct SassCompiler* compiler);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Parse the entry point and potentially all imports within.
void sass_compiler_parse(struct SassCompiler* compiler);

// Evaluate the parsed entry point and store resulting ast-tree.
void sass_compiler_compile(struct SassCompiler* compiler);

// Render the evaluated ast-tree to get the final output string.
void sass_compiler_render(struct SassCompiler* compiler);

// Write or print the output to the console or the configured output path
void sass_compiler_write_output(struct SassCompiler* compiler);

// Write source-map to configured path if options are set accordingly
void sass_compiler_write_srcmap(struct SassCompiler* compiler);

// Execute all compiler steps and write/print results
int sass_compiler_execute(struct SassCompiler* compiler, bool quiet);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Add additional include paths where LibSass will look for includes.
// Note: the passed in `paths` can be path separated (`;` on windows, `:` otherwise).
void sass_compiler_add_include_paths(struct SassCompiler* compiler, const char* paths);

// Load dynamic loadable plugins from `paths`. Plugins are only supported on certain OSs and
// are still in experimental state. This will look for `*.dll`, `*.so` or `*.dynlib` files.
// It then tries to load the found libraries and does a few checks to see if the library
// is actually a LibSass plugin. We then call its init hook if the library is compatible.
// Note: the passed in `paths` can be path separated (`;` on windows, `:` otherwise).
void sass_compiler_load_plugins(struct SassCompiler* compiler, const char* paths);

// Add a custom header importer that will always be executed before any other
// compilations takes place. Useful to prepend a shared copyright header or to
// provide global variables or functions. This feature is still in experimental state.
// Note: With the adaption of Sass Modules this might be completely replaced in the future.
void sass_compiler_add_custom_header(struct SassCompiler* compiler, struct SassImporter* header);

// Add a custom importer that will be executed when a sass `@import` rule is found.
// This is useful to e.g. rewrite import locations or to load content from remote.
// For more please check https://github.com/sass/libsass/blob/master/docs/api-importer.md
// Note: The importer will not be called for regular css `@import url()` rules.
void sass_compiler_add_custom_importer(struct SassCompiler* compiler, struct SassImporter* importer);

// Add a custom function that will be executed when the corresponding function call is
// requested from any sass code. This is useful to provide custom functions in your code.
// For more please check https://github.com/sass/libsass/blob/master/docs/api-function.md
void sass_compiler_add_custom_function(struct SassCompiler* compiler, struct SassFunction* function);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Setter for output style (see `enum SassOutputStyle` for possible options).
void sass_compiler_set_output_style(struct SassCompiler* compiler, enum SassOutputStyle output_style);

// Try to detect and set logger options for terminal colors, unicode and columns.
void sass_compiler_autodetect_logger_capabilities(struct SassCompiler* compiler);

// Setter for enabling/disabling logging with ANSI colors.
void sass_compiler_set_logger_colors(struct SassCompiler* compiler, bool enable);

// Setter for enabling/disabling logging with unicode text.
void sass_compiler_set_logger_unicode(struct SassCompiler* compiler, bool enable);

// Getter for number precision (how floating point numbers are truncated).
int sass_compiler_get_precision(struct SassCompiler* compiler);

// Setter for number precision (how floating point numbers are truncated).
void sass_compiler_set_precision(struct SassCompiler* compiler, int precision);

// Getter for compiler entry point (which file or data to parse first).
struct SassImport* sass_compiler_get_entry_point(struct SassCompiler* compiler);

// Setter for compiler entry point (which file or data to parse first).
void sass_compiler_set_entry_point(struct SassCompiler* compiler, struct SassImport* import);

// Getter for compiler output path (where to store the result)
// Note: LibSass does not write the file, implementers should write to this path.
const char* sass_compiler_get_output_path(struct SassCompiler* compiler);

// Setter for compiler output path (where to store the result)
// Note: LibSass does not write the file, implementers should write to this path.
void sass_compiler_set_output_path(struct SassCompiler* compiler, const char* output_path);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Getter for warnings that occurred during any step.
const char* sass_compiler_get_warn_string(struct SassCompiler* compiler);

// Getter for output after parsing, compilation and rendering.
const char* sass_compiler_get_output_string(struct SassCompiler* compiler);

// Getter for footer string containing optional source-map (embedded or link).
const char* sass_compiler_get_footer_string(struct SassCompiler* compiler);

// Getter for string containing the optional source-mapping.
const char* sass_compiler_get_srcmap_string(struct SassCompiler* compiler);

// Check if implementor is expected to write a output file
bool sass_compiler_has_output_file(struct SassCompiler* compiler);

// Check if implementor is expected to write a source-map file
bool sass_compiler_has_srcmap_file(struct SassCompiler* compiler);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Setter for source-map mode (how to embed or not embed the source-map).
void sass_compiler_set_srcmap_mode(struct SassCompiler* compiler, enum SassSrcMapMode mode);

// Setter for source-map path (where to store the source-mapping).
// Note: if path is not explicitly given, we will deduct one from output path.
// Note: LibSass does not write the file, implementers should write to this path.
void sass_compiler_set_srcmap_path(struct SassCompiler* compiler, const char* path);

// Getter for source-map path (where to store the source-mapping).
// Note: if path is not explicitly given, we will deduct one from output path.
// Note: the value will only be deducted after the main render phase is completed.
// Note: LibSass does not write the file, implementers should write to this path.
const char* sass_compiler_get_srcmap_path(struct SassCompiler* compiler);

// Setter for source-map root (simply passed to the resulting srcmap info).
// Note: if not given, no root attribute will be added to the srcmap info object.
void sass_compiler_set_srcmap_root(struct SassCompiler* compiler, const char* root);

// Setter for source-map file-url option (renders urls in srcmap as `file://` urls)
void sass_compiler_set_srcmap_file_urls(struct SassCompiler* compiler, bool enable);

// Setter for source-map embed-contents option (includes full sources in the srcmap info)
void sass_compiler_set_srcmap_embed_contents(struct SassCompiler* compiler, bool enable);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Getter to return the number of all included files.
size_t sass_compiler_get_included_files_count(struct SassCompiler* compiler);

// Getter to return path to the included file at position `n`.
const char* sass_compiler_get_included_file_path(struct SassCompiler* compiler, size_t n);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Getter for current import context. Use `SassImport` functions to query the state.
const struct SassImport* sass_compiler_get_last_import(struct SassCompiler* compiler);

// Returns pointer to error object associated with compiler.
// Will be valid until the associated compiler is destroyed.
const struct SassError* sass_compiler_get_error(struct SassCompiler* compiler);

// Returns status code for compiler (0 meaning success, anything else is an error)
int sass_compiler_get_status(struct SassCompiler* compiler);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Resolve a file relative to last import or include paths in the sass option struct.
char* sass_compiler_find_file(const char* path, struct SassCompiler* compiler);

// Resolve an include relative to last import or include paths in the sass option struct.
// This will do a lookup as LibSass would do internally (partials, different extensions).
// ToDo: Check if we should add `includeIndex` option to check for directory index files!?
char* sass_compiler_find_include(const char* path, struct SassCompiler* compiler);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
```
