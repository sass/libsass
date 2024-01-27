/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_C_COMPILER_H
#define SASS_C_COMPILER_H

#include <sass/base.h>
#include <sass/function.h>

#ifdef __cplusplus
extern "C" {
#endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create a new LibSass compiler context
  ADDAPI struct SassCompiler* ADDCALL sass_make_compiler();

  // Release all memory allocated with the compiler
  ADDAPI void ADDCALL sass_delete_compiler(struct SassCompiler* compiler);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Parse the entry point and potentially all imports within.
  ADDAPI void ADDCALL sass_compiler_parse(struct SassCompiler* compiler);

  // Evaluate the parsed entry point and store resulting ast-tree.
  ADDAPI void ADDCALL sass_compiler_compile(struct SassCompiler* compiler);

  // Render the evaluated ast-tree to get the final output string.
  ADDAPI void ADDCALL sass_compiler_render(struct SassCompiler* compiler);

  // Write or print the output to the console or the configured output path
  ADDAPI void ADDCALL sass_compiler_write_output(struct SassCompiler* compiler);

  // Write source-map to configured path if options are set accordingly
  ADDAPI void ADDCALL sass_compiler_write_srcmap(struct SassCompiler* compiler);

  // Execute all compiler steps and write/print results
  ADDAPI int ADDCALL sass_compiler_execute(struct SassCompiler* compiler);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Add additional include paths where LibSass will look for includes.
  // Note: the passed in `paths` can be path separated (`;` on windows, `:` otherwise).
  ADDAPI void ADDCALL sass_compiler_add_include_paths(struct SassCompiler* compiler, const char* paths);

  // Load dynamic loadable plugins from `paths`. Plugins are only supported on certain OSs and
  // are still in experimental state. This will look for `*.dll`, `*.so` or `*.dynlib` files.
  // It then tries to load the found libraries and does a few checks to see if the library
  // is actually a LibSass plugin. We then call its init hook if the library is compatible.
  // Note: the passed in `paths` can be path separated (`;` on windows, `:` otherwise).
  ADDAPI void ADDCALL sass_compiler_load_plugins(struct SassCompiler* compiler, const char* paths);

  // Add a custom header importer that will always be executed before any other
  // compilations takes place. Useful to prepend a shared copyright header or to
  // provide global variables or functions. This feature is still in experimental state.
  // Note: With the adaption of Sass Modules this might be completely replaced in the future.
  ADDAPI void ADDCALL sass_compiler_add_custom_header(struct SassCompiler* compiler, struct SassImporter* header);

  // Add a custom importer that will be executed when a sass `@import` rule is found.
  // This is useful to e.g. rewrite import locations or to load content from remote.
  // For more please check https://github.com/sass/libsass/blob/master/docs/api-importer.md
  // Note: The importer will not be called for regular css `@import url()` rules.
  ADDAPI void ADDCALL sass_compiler_add_custom_importer(struct SassCompiler* compiler, struct SassImporter* importer);

  // Add a custom function that will be executed when the corresponding function call is
  // requested from any sass code. This is useful to provide custom functions in your code.
  // For more please check https://github.com/sass/libsass/blob/master/docs/api-function.md
  ADDAPI void ADDCALL sass_compiler_add_custom_function(struct SassCompiler* compiler, struct SassFunction* function);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Setter for output style (see `enum SassOutputStyle` for possible options).
  ADDAPI void ADDCALL sass_compiler_set_input_syntax(struct SassCompiler* compiler, enum SassImportSyntax syntax);

  // Setter for output style (see `enum SassOutputStyle` for possible options).
  ADDAPI void ADDCALL sass_compiler_set_output_style(struct SassCompiler* compiler, enum SassOutputStyle style);

  // Try to detect and set logger options for terminal colors, unicode and columns.
  ADDAPI void ADDCALL sass_compiler_autodetect_logger_capabilities(struct SassCompiler* compiler);

  // Setter for enabling/disabling logging with ANSI colors.
  ADDAPI void ADDCALL sass_compiler_set_logger_colors(struct SassCompiler* compiler, bool enable);

  // Setter for enabling/disabling logging with unicode text.
  ADDAPI void ADDCALL sass_compiler_set_logger_unicode(struct SassCompiler* compiler, bool enable);

  // Getter for number precision (how floating point numbers are truncated).
  ADDAPI int ADDCALL sass_compiler_get_precision(struct SassCompiler* compiler);

  // Setter for number precision (how floating point numbers are truncated).
  ADDAPI void ADDCALL sass_compiler_set_precision(struct SassCompiler* compiler, int precision);

  // Getter for compiler entry point (which file or data to parse first).
  ADDAPI struct SassImport* ADDCALL sass_compiler_get_entry_point(struct SassCompiler* compiler);

  // Setter for compiler entry point (which file or data to parse first).
  ADDAPI void ADDCALL sass_compiler_set_entry_point(struct SassCompiler* compiler, struct SassImport* import);

  // Getter for compiler output path (where to store the result)
  // Note: LibSass does not write the file, implementers should write to this path.
  ADDAPI const char* ADDCALL sass_compiler_get_output_path(struct SassCompiler* compiler);

  // Setter for compiler output path (where to store the result)
  // Note: LibSass does not write the file, implementers should write to this path.
  ADDAPI void ADDCALL sass_compiler_set_output_path(struct SassCompiler* compiler, const char* output_path);

  // Getter for option to suppress anything being printed on stderr (quiet mode)
  ADDAPI bool ADDCALL sass_compiler_get_suppress_stderr(struct SassCompiler* compiler);

  // Setter for option to suppress anything being printed on stderr (quiet mode)
  ADDAPI void ADDCALL sass_compiler_set_suppress_stderr(struct SassCompiler* compiler, bool suppress);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for warnings that occurred during any step.
  ADDAPI const char* ADDCALL sass_compiler_get_warn_string(struct SassCompiler* compiler);

  // Getter for output after parsing, compilation and rendering.
  ADDAPI const char* ADDCALL sass_compiler_get_output_string(struct SassCompiler* compiler);

  // Getter for footer string containing optional source-map (embedded or link).
  ADDAPI const char* ADDCALL sass_compiler_get_footer_string(struct SassCompiler* compiler);

  // Getter for string containing the optional source-mapping.
  ADDAPI const char* ADDCALL sass_compiler_get_srcmap_string(struct SassCompiler* compiler);

  // Check if implementor is expected to write a output file
  ADDAPI bool ADDCALL sass_compiler_has_output_file(struct SassCompiler* compiler);

  // Check if implementor is expected to write a source-map file
  ADDAPI bool ADDCALL sass_compiler_has_srcmap_file(struct SassCompiler* compiler);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Setter for source-map mode (how to embed or not embed the source-map).
  ADDAPI void ADDCALL sass_compiler_set_srcmap_mode(struct SassCompiler* compiler, enum SassSrcMapMode mode);

  // Setter for source-map path (where to store the source-mapping).
  // Note: if path is not explicitly given, we will deduct one from output path.
  // Note: LibSass does not write the file, implementers should write to this path.
  ADDAPI void ADDCALL sass_compiler_set_srcmap_path(struct SassCompiler* compiler, const char* path);

  // Getter for source-map path (where to store the source-mapping).
  // Note: if path is not explicitly given, we will deduct one from output path.
  // Note: the value will only be deducted after the main render phase is completed.
  // Note: LibSass does not write the file, implementers should write to this path.
  ADDAPI const char* ADDCALL sass_compiler_get_srcmap_path(struct SassCompiler* compiler);

  // Setter for source-map root (simply passed to the resulting srcmap info).
  // Note: if not given, no root attribute will be added to the srcmap info object.
  ADDAPI void ADDCALL sass_compiler_set_srcmap_root(struct SassCompiler* compiler, const char* root);

  // Setter for source-map file-url option (renders urls in srcmap as `file://` urls)
  ADDAPI void ADDCALL sass_compiler_set_srcmap_file_urls(struct SassCompiler* compiler, bool enable);

  // Setter for source-map embed-contents option (includes full sources in the srcmap info)
  ADDAPI void ADDCALL sass_compiler_set_srcmap_embed_contents(struct SassCompiler* compiler, bool enable);

  // Setter to enable more detailed source map (also meaning bigger payload).
  // Mostly useful if you want to post process the results again where the more detailed
  // source-maps might by used by downstream post-processor to point back to original files.
  ADDAPI void ADDCALL sass_compiler_set_srcmap_details(struct SassCompiler* compiler, bool openers, bool closers);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter to return the number of all included files.
  ADDAPI size_t ADDCALL sass_compiler_get_included_files_count(struct SassCompiler* compiler);

  // Getter to return path to the included file at position `n`.
  ADDAPI const char* ADDCALL sass_compiler_get_included_file_path(struct SassCompiler* compiler, size_t n);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for current import context. Use `SassImport` functions to query the state.
  ADDAPI const struct SassImport* ADDCALL sass_compiler_get_last_import(struct SassCompiler* compiler);

  // Returns pointer to error object associated with compiler.
  // Will be valid until the associated compiler is destroyed.
  ADDAPI const struct SassError* ADDCALL sass_compiler_get_error(struct SassCompiler* compiler);

  // Returns status code for compiler (0 meaning success, anything else is an error)
  ADDAPI int ADDCALL sass_compiler_get_status(struct SassCompiler* compiler);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Resolve a file relative to last import or include paths in the sass option struct.
  ADDAPI char* ADDCALL sass_compiler_find_file(const char* path, struct SassCompiler* compiler);

  // Resolve an include relative to last import or include paths in the sass option struct.
  // This will do a lookup as LibSass would do internally (partials, different extensions).
  // ToDo: Check if we should add `includeIndex` option to check for directory index files!?
  ADDAPI char* ADDCALL sass_compiler_find_include(const char* path, struct SassCompiler* compiler);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // EO extern "C".
#endif

#endif
