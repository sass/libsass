/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_ENUMS_H
#define SASS_ENUMS_H

#ifdef __cplusplus
extern "C" {
#endif

  // Different render styles
  enum SassOutputStyle {
    SASS_STYLE_NESTED,
    SASS_STYLE_EXPANDED,
    SASS_STYLE_COMPACT,
    SASS_STYLE_COMPRESSED,
    // only used internally!
    SASS_STYLE_TO_CSS
  };

  // Type of parser to use
  enum SassImportSyntax {
    SASS_IMPORT_AUTO,
    SASS_IMPORT_SCSS,
    SASS_IMPORT_SASS,
    SASS_IMPORT_CSS,
  };

  // Config how to produce source-map
  enum SassSrcMapMode {
    // Don't render any source-mapping.
    SASS_SRCMAP_NONE,
    // Only render the `srcmap` string.
    // The `footer` will be `NULL`.
    SASS_SRCMAP_CREATE,
    // Write srcmap link into `footer`
    SASS_SRCMAP_EMBED_LINK,
    // Embed srcmap into `footer`
    SASS_SRCMAP_EMBED_JSON,
  };

  // State of the compiler object
  enum SassCompilerState {
    SASS_COMPILER_CREATED,
    SASS_COMPILER_PARSED,
    SASS_COMPILER_COMPILED,
    SASS_COMPILER_RENDERED,
    SASS_COMPILER_DESTROYED,
    SASS_COMPILER_FAILED,
  };

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
