#ifndef SASS_H
#define SASS_H

// #define DEBUG 1

// Note: we can't forward declare with inheritance
// https://stackoverflow.com/a/10145303/1550314

// include API headers
#include <sass/base.h>
#include <sass/fwdecl.h>
#include <sass/version.h>
#include <sass/lists.h>
#include <sass/values.h>
#include <sass/error.h>
#include <sass/import.h>
#include <sass/traces.h>
#include <sass/functions.h>
#include <sass/context.h>
#include <sass/compiler.h>

typedef struct SassImport* Sass_Import_Entry;
typedef struct SassImporter* Sass_Importer_Entry;
typedef struct SassFunction* Sass_Function_Entry;

typedef struct SassImportList* Sass_Import_List;
typedef struct SassImporterList* Sass_Importer_List;
typedef struct SassFunctionList* Sass_Function_List;

#endif
