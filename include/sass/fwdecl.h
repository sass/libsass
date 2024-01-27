/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#ifndef SASS_FWDECL_H
#define SASS_FWDECL_H

#include <sass/base.h>

#ifdef __cplusplus
extern "C" {
#endif

  // Forward declare anonymous structs
  // C never sees any implementation
  struct SassValue; // ref-counted
  struct SassTrace; // C++ wrapper
  struct SassError; // CAPI-struct
  struct SassCompiler; // C++ wrapper
  struct SassFunction; // CAPI-struct
  struct SassSource; // C++ wrapper
  struct SassSrcSpan; // C++ wrapper
  struct SassImport; // ref-counted
  struct SassImporter; // CAPI-struct
  struct SassImportList; // std::deque
  struct SassMapIterator; // CAPI-struct
  struct SassGetOpt; // Helper

#ifdef __cplusplus
} // __cplusplus defined.
#endif

#endif
