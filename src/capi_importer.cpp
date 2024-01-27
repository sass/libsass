/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_importer.hpp"

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Sass;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create custom importer (with arbitrary pointer called `cookie`)
  // The pointer is often used to store the callback into the actual binding.
  struct SassImporter* ADDCALL sass_make_importer(SassImporterLambda lambda, double priority, void* cookie)
  {
    if (lambda == nullptr) return nullptr;
    struct SassImporter* importer = new SassImporter{};
    if (importer == nullptr) return nullptr;
    importer->lambda = lambda;
    importer->priority = priority;
    importer->cookie = cookie;
    return importer;
  }

  // Deallocate the importer and release memory
  void ADDCALL sass_delete_importer(struct SassImporter* importer)
  {
    delete importer;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for importer lambda function (the one being actually invoked)
  SassImporterLambda ADDCALL sass_importer_get_lambda(struct SassImporter* importer)
  {
    return importer->lambda;
  }

  // Getter for importer priority (lowest priority is invoked first)
  double ADDCALL sass_importer_get_priority(struct SassImporter* importer)
  {
    return importer->priority;
  }

  // Getter for arbitrary cookie (used by implementers to store stuff)
  void* ADDCALL sass_importer_get_cookie(struct SassImporter* importer)
  {
    return importer->cookie;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
