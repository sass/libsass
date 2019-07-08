#ifndef SASS_C_LISTS_H
#define SASS_C_LISTS_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct SassImportList;
  struct SassImporterList;

  // Creator for sass custom importer return argument list
  ADDAPI struct SassImportList* ADDCALL sass_make_import_list();
  ADDAPI void ADDCALL sass_delete_import_list(struct SassImportList* list);
  ADDAPI size_t ADDCALL sass_import_list_size(struct SassImportList* list);

  ADDAPI struct SassImport* ADDCALL sass_import_list_shift(struct SassImportList* list);
  ADDAPI void ADDCALL sass_import_list_push(struct SassImportList* list, struct SassImport*);

  // Creator for sass plugin loading return arguments
  ADDAPI struct SassImporterList* ADDCALL sass_make_importer_list();
  ADDAPI void ADDCALL sass_delete_importer_list(struct SassImporterList* list);
  ADDAPI size_t ADDCALL sass_importer_list_size(struct SassImporterList* list);
  ADDAPI struct SassImporter* ADDCALL sass_importer_list_shift(struct SassImporterList* list);
  ADDAPI void ADDCALL sass_importer_list_push(struct SassImporterList* list, struct SassImporter*);

  // Creators for sass function list and function descriptors
  ADDAPI struct SassFunctionList* ADDCALL sass_make_function_list();
  ADDAPI void ADDCALL sass_delete_function_list(struct SassFunctionList* list);
  ADDAPI size_t ADDCALL sass_function_list_size(struct SassFunctionList* list);
  ADDAPI struct SassFunction* ADDCALL sass_function_list_shift(struct SassFunctionList* list);
  ADDAPI void ADDCALL sass_function_list_push(struct SassFunctionList* list, struct SassFunction*);

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
