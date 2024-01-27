/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_C_IMPORT_H
#define SASS_C_IMPORT_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create import entry by reading from `stdin`.
  ADDAPI struct SassImport* ADDCALL sass_make_stdin_import(const char* imp_path);

  // Create import entry to load the passed input path.
  ADDAPI struct SassImport* ADDCALL sass_make_file_import(const char* imp_path);

  // Create import entry for the passed data with optional path.
  // Note: we take ownership of the passed `content` memory.
  ADDAPI struct SassImport* ADDCALL sass_make_content_import(char* content, const char* imp_path);

  // Create single import entry returned by the custom importer inside the list.
  // Note: source/srcmap can be empty to let LibSass do the file resolving.
  // Note: we take ownership of the passed `source` and `srcmap` memory.
  ADDAPI struct SassImport* ADDCALL sass_make_import(const char* imp_path, const char* abs_base,
    char* source, char* srcmap, enum SassImportSyntax format);

  // Just in case we have some stray import structs
  ADDAPI void ADDCALL sass_delete_import(struct SassImport* import);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for specific import format for the given import (force css/sass/scss or set to auto)
  ADDAPI enum SassImportSyntax ADDCALL sass_import_get_type(const struct SassImport* import);

  // Setter for specific import format for the given import (force css/sass/scss or set to auto)
  ADDAPI void ADDCALL sass_import_set_syntax(struct SassImport* import, enum SassImportSyntax syntax);

  // Getter for original import path (as seen when parsed)
  ADDAPI const char* ADDCALL sass_import_get_imp_path(const struct SassImport* import);

  // Getter for resolve absolute path (after being resolved)
  ADDAPI const char* ADDCALL sass_import_get_abs_path(const struct SassImport* import);

  // Getter for import error message (used by custom importers).
  // If error is not `nullptr`, the import must be considered as failed.
  ADDAPI const char* ADDCALL sass_import_get_error_message(struct SassImport* import);

  // Setter for import error message (used by custom importers).
  // If error is not `nullptr`, the import must be considered as failed.
  ADDAPI void ADDCALL sass_import_set_error_message(struct SassImport* import, const char* msg);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create new list container for imports.
  ADDAPI struct SassImportList* ADDCALL sass_make_import_list();

  // Release memory of list container and all children.
  ADDAPI void ADDCALL sass_delete_import_list(struct SassImportList* list);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return number of items currently in the list.
  ADDAPI size_t ADDCALL sass_import_list_size(struct SassImportList* list);

  // Remove and return first item in the list (as in a fifo queue).
  ADDAPI struct SassImport* ADDCALL sass_import_list_shift(struct SassImportList* list);

  // Append additional import to the list container.
  ADDAPI void ADDCALL sass_import_list_push(struct SassImportList* list, struct SassImport* import);

  // Append additional import to the list container and takes ownership of the import.
  ADDAPI void ADDCALL sass_import_list_emplace(struct SassImportList* list, struct SassImport* import);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
