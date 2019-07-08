#ifndef SASS_C_IMPORT_H
#define SASS_C_IMPORT_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  // Create an import entry by reading from `stdin`
  ADDAPI struct SassImport* ADDCALL sass_make_stdin_import(const char* imp_path);

  // Crate import entry for input path (returns nullptr if not found or unreadable)
  // Will read the file without LibSass internal path resolving (relative to CWD)!
  // ADDAPI struct SassImport* ADDCALL sass_make_file_import(struct SassCompiler* compiler, const char* imp_path);

  // Crate an import entry for the passed data content and optional import path.
  // The import path is optional and defaults to `sass://data` if `null` is passed.
  // ADDAPI struct SassImport* ADDCALL sass_make_data_import(char* content, const char* imp_path);

  // Note: compiler ones do error reporting, since file loading my error ...
  // One solution would be to postpone the file loading into the parse stage

  // Crate an import entry for the passed input path
  ADDAPI struct SassImport* ADDCALL sass_make_file_import(const char* imp_path);

  // Crate an import entry for the passed input path
  ADDAPI struct SassImport* ADDCALL sass_make_content_import(char* content, const char* imp_path);

  // Set specific import format for the given import
  ADDAPI void ADDCALL sass_import_set_format(struct SassImport* import, enum SassImportFormat format);

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
