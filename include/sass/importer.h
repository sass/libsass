/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_C_IMPORTER_H
#define SASS_C_IMPORTER_H

#include <sass/base.h>
#include <sass/import.h>

#ifdef __cplusplus
extern "C" {
#endif

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Type definitions for importer functions
  typedef struct SassImportList* (*SassImporterLambda)(
    const char* url, struct SassImporter* cb, struct SassCompiler* compiler);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create custom importer (with arbitrary data pointer called `cookie`)
  // The pointer is often used to store the callback into the actual binding.
  ADDAPI struct SassImporter* ADDCALL sass_make_importer(
    SassImporterLambda lambda, double priority, void* cookie);

  // Deallocate the importer and release memory
  ADDAPI void ADDCALL sass_delete_importer(struct SassImporter* cb);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for importer lambda function (the one being actually invoked)
  ADDAPI SassImporterLambda ADDCALL sass_importer_get_lambda(struct SassImporter* cb);

  // Getter for importer priority (lowest priority is invoked first)
  ADDAPI double ADDCALL sass_importer_get_priority(struct SassImporter* cb);

  // Getter for arbitrary cookie (used by implementers to store stuff)
  ADDAPI void* ADDCALL sass_importer_get_cookie(struct SassImporter* cb);

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
