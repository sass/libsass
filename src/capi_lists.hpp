#ifndef SASS_SASS_LISTS_H
#define SASS_SASS_LISTS_H

#include <sass.h>
#include <sass/base.h>
#include <sass/fwdecl.h>

#include <deque>

// Some structures are simple c++ vectors.
// There might be a more efficient way to achieve this?
// Although compiler optimization should see this case easily!
struct SassImportList : std::deque<struct SassImport*> {};
struct SassImporterList : std::deque<struct SassImporter*> {};
struct SassFunctionList : std::deque<struct SassFunction*> {};

#endif
