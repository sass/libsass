#ifndef SASS_C_FUNCTIONS_H
#define SASS_C_FUNCTIONS_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

// Creator for sass custom importer return argument list
// ADDAPI struct SassImportList* ADDCALL sass_make_import_list();
// ADDAPI void ADDCALL sass_delete_import_list(struct SassImportList* list);
// ADDAPI size_t ADDCALL sass_import_list_size(struct SassImportList* list);
// ADDAPI struct SassImport* ADDCALL sass_import_list_shift(struct SassImportList* list);
// ADDAPI void ADDCALL sass_import_list_push(struct SassImportList* list, struct SassImport*);

// Creator for sass custom importer return argument list
// ADDAPI struct SassImporterList* ADDCALL sass_make_importer_list ();
// ADDAPI void ADDCALL sass_delete_importer_list(struct SassImporterList* list);
// ADDAPI size_t ADDCALL sass_importer_list_size(struct SassImporterList* list);
// ADDAPI struct SassImporter* ADDCALL sass_importer_list_shift(struct SassImporterList* list);
// ADDAPI void ADDCALL sass_importer_list_push(struct SassImporterList* list, struct SassImporter*);

// Creators for custom importer callback (with some additional pointer)
// The pointer is mostly used to store the callback into the actual binding
  ADDAPI struct SassImporter* ADDCALL sass_make_importer(SassImporterLambda importer, double priority, void* cookie);

  // Getters for import function descriptors
  ADDAPI SassImporterLambda ADDCALL sass_importer_get_callback(struct SassImporter* cb);
  ADDAPI double ADDCALL sass_importer_get_priority(struct SassImporter* cb);
  ADDAPI void* ADDCALL sass_importer_get_cookie(struct SassImporter* cb);

  // Deallocator for associated memory
  ADDAPI void ADDCALL sass_delete_importer(struct SassImporter* cb);

  // Creator for a single import entry returned by the custom importer inside the list
  ADDAPI struct SassImport* ADDCALL sass_make_import(const char* imp_path, const char* abs_base, char* source, char* srcmap, enum SassImportFormat format);
  // ADDAPI struct SassImport* ADDCALL sass_make_import_error(const char* error);

  // set error message to abort import and to print out a message (path from existing object is used in output)
  ADDAPI void ADDCALL sass_import_set_error_msg(struct SassImport* import, const char* message, uint32_t line, uint32_t col);

  // Getters for callee entry
// ADDAPI const char* ADDCALL sass_callee_get_name (struct SassCallee*);
// ADDAPI const char* ADDCALL sass_callee_get_path (struct SassCallee*);
// ADDAPI uint32_t ADDCALL sass_callee_get_line (struct SassCallee*);
// ADDAPI uint32_t ADDCALL sass_callee_get_column (struct SassCallee*);
// ADDAPI enum Sass_Callee_Type ADDCALL sass_callee_get_type (struct SassCallee*);

// Getters and Setters for environments (lexical, local and global)
// ADDAPI struct SassValue* ADDCALL sass_env_get_lexical (struct SassCompiler*, const char*);
ADDAPI void ADDCALL sass_env_set_lexical (struct SassCompiler*, const char*, struct SassValue*);
// ADDAPI struct SassValue* ADDCALL sass_env_get_local (struct SassCompiler*, const char*);
// ADDAPI void ADDCALL sass_env_set_local (struct SassCompiler*, const char*, struct SassValue*);
// ADDAPI struct SassValue* ADDCALL sass_env_get_global (struct SassCompiler*, const char*);
ADDAPI void ADDCALL sass_env_set_global (struct SassCompiler*, const char*, struct SassValue*);

// Getters for import entry
ADDAPI const char* ADDCALL sass_import_get_imp_path (struct SassImport*);
ADDAPI const char* ADDCALL sass_import_get_abs_path (struct SassImport*);
// ADDAPI const char* ADDCALL sass_import_get_source (struct SassImport*);
// ADDAPI const char* ADDCALL sass_import_get_srcmap (struct SassImport*);
ADDAPI enum SassImportFormat ADDCALL sass_import_get_type(struct SassImport*);

// Getters from import error entry
// ADDAPI uint32_t ADDCALL sass_import_get_error_line (struct SassImport*);
// ADDAPI uint32_t ADDCALL sass_import_get_error_column (struct SassImport*);
ADDAPI const char* ADDCALL sass_import_get_error_message (struct SassImport*);

// Just in case we have some stray import structs
ADDAPI void ADDCALL sass_delete_import(struct SassImport*);


ADDAPI struct SassFunction* ADDCALL sass_make_function (const char* signature, SassFunctionLambda cb, void* cookie);
ADDAPI void ADDCALL sass_delete_function (struct SassFunction* entry);

// Getters for custom function descriptors
ADDAPI const char* ADDCALL sass_function_get_signature (struct SassFunction* cb);
ADDAPI SassFunctionLambda ADDCALL sass_function_get_function (struct SassFunction* cb);
ADDAPI void* ADDCALL sass_function_get_cookie (struct SassFunction* cb);


#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
