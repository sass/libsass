/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_import.hpp"

#include "import.hpp"
#include "sources.hpp"

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Sass;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create import entry by reading from `stdin`.
  struct SassImport* ADDCALL sass_make_stdin_import(const char* path)
  {
    std::istreambuf_iterator<char> begin(std::cin), end;
    sass::string text(begin, end); // consume everything
    Import* import = SASS_MEMORY_NEW(Import,
      SASS_MEMORY_NEW(SourceString,
        path ? path : "stream://stdin",
        std::move(text)),
      SASS_IMPORT_AUTO);
    import->refcount += 1;
    return import->wrap();
  }
  // EO sass_make_stdin_import

  // Create import entry to load the passed input path.
  struct SassImport* ADDCALL sass_make_file_import(const char* imp_path)
  {
    sass::string abs_path(File::rel2abs(imp_path, CWD()));
    Import* loaded = SASS_MEMORY_NEW(Import,
      SASS_MEMORY_NEW(SourceFile,
        imp_path, abs_path.c_str(),
        nullptr, nullptr),
      SASS_IMPORT_AUTO);
    loaded->refcount += 1;
    return loaded->wrap();
  }
  // EO sass_make_file_import

  // Create import entry for the passed data with optional path.
  // Note: we take ownership of the passed `content` memory.
  struct SassImport* ADDCALL sass_make_content_import(char* content, const char* path)
  {
    Import* loaded = SASS_MEMORY_NEW(Import,
      SASS_MEMORY_NEW(SourceString,
        path ? path : "stream://stdin",
        path ? path : "stream://stdin",
        content ? content : "",
        ""),
      SASS_IMPORT_AUTO);
    loaded->refcount += 1;
    return loaded->wrap();
  }
  // EO sass_make_content_import

  // Create single import entry returned by the custom importer inside the list.
  // Note: source/srcmap can be empty to let LibSass do the file resolving.
  // Note: we take ownership of the passed `source` and `srcmap` memory.
  struct SassImport* ADDCALL sass_make_import(const char* imp_path, const char* abs_path,
    char* source, char* srcmap, enum SassImportSyntax format)
  {
    Import* import =
      SASS_MEMORY_NEW(Import,
        SASS_MEMORY_NEW(SourceFile,
          imp_path, abs_path,
          source ? source : 0,
          srcmap ? srcmap : 0),
        format);
    // Use reference counter
    import->refcount += 1;
    // Create the shared source object
    return Import::wrap(import);
  }
  // EO sass_make_import

  // Just in case we have some stray import structs
  void ADDCALL sass_delete_import(struct SassImport* import)
  {
    Import& object = Import::unwrap(import);
    if (object.refcount <= 1) {
      object.refcount = 0;
      delete &object;
    }
    else {
      object.refcount -= 1;
    }
  }
  // EO sass_delete_import

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for specific import format for the given import (force css/sass/scss or set to auto)
  enum SassImportSyntax ADDCALL sass_import_get_type(const struct SassImport* entry)
  {
    return Import::unwrap(entry).syntax;
  }

  // Setter for specific import format for the given import (force css/sass/scss or set to auto)
  void ADDCALL sass_import_set_syntax(struct SassImport* import, enum SassImportSyntax syntax)
  {
    Import::unwrap(import).syntax = syntax;
  }

  // Getter for original import path (as seen when parsed)
  const char* ADDCALL sass_import_get_imp_path(const struct SassImport* entry)
  {
    return Import::unwrap(entry).getImpPath();
  }

  // Getter for resolve absolute path (after being resolved)
  const char* ADDCALL sass_import_get_abs_path(const struct SassImport* entry)
  {
    return Import::unwrap(entry).getAbsPath();
  }

  // Getter for import error message (used by custom importers).
  // If error is not `nullptr`, the import must be considered as failed.
  const char* ADDCALL sass_import_get_error_message(struct SassImport* entry)
  {
    return Import::unwrap(entry).getErrorMsg();
  }

  // Setter for import error message (used by custom importers).
  // If error is not `nullptr`, the import must be considered as failed.
  void ADDCALL sass_import_set_error_message(struct SassImport* entry, const char* msg)
  {
    return Import::unwrap(entry).setErrorMsg(msg);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create new list container for imports.
  struct SassImportList* ADDCALL sass_make_import_list()
  {
    return new SassImportList;
  }

  // Release memory of list container and all children.
  void ADDCALL sass_delete_import_list(struct SassImportList* list)
  {
    if (list == nullptr) return;
    for (auto import : *list) {
      sass_delete_import(import);
    }
    delete list;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Return number of items currently in the list.
  size_t ADDCALL sass_import_list_size(struct SassImportList* list)
  {
    if (list == nullptr) return 0;
    return list->size();
  }

  // Remove and return first item in the list (as in a fifo queue).
  struct SassImport* ADDCALL sass_import_list_shift(struct SassImportList* list)
  {
    if (list == nullptr) return nullptr;
    if (list->empty()) return nullptr;
    auto ptr = list->front();
    list->pop_front();
    return ptr;
  }

  // Append additional import to the list container.
  void ADDCALL sass_import_list_push(struct SassImportList* list, struct SassImport* import)
  {
    if (list == nullptr) return;
    if (import) list->push_back(import);
  }

  // Append additional import to the list container and takes ownership of the import.
  void ADDCALL sass_import_list_emplace(struct SassImportList* list, struct SassImport* import)
  {
    sass_import_list_push(list, import);
    // Reduce refcount by one (we took ownership)
    if (import) Import::unwrap(import).refcount--;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
