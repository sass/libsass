
## LibSass import C-API

Imports on the C-API side can either be used as compilation entry points or
imports returned from custom importers/headers. They represent a loadable
resource with text to be parsed. These object are reference-counted and
must always be freed by the allocator. To improve usage with `SassImportList`
there is a convenience method `sass_import_list_emplace` that will transfer
the ownership of the import to the list object.

### Imports and source-maps

Source-maps embedded in imports are not automatically parsed out. Ideally we
would want to automatically load associated source-maps for inputs. Currently
LibSass does not make use of any upstream source-maps. The API has been designed
with that case in mind, but implementation is not done yet. Everything still
works, but additional source-maps may end up as superfluous comments.

### Basic Usage

```C
#include <sass/import.h>
```

### Sass Import API

```C
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Create import entry by reading from `stdin`.
struct SassImport* sass_make_stdin_import(const char* imp_path);

// Create import entry to load the passed input path.
struct SassImport* sass_make_file_import(const char* imp_path);

// Create import entry for the passed data with optional path.
// Note: we take ownership of the passed `content` memory.
struct SassImport* sass_make_content_import(char* content, const char* imp_path);

// Create single import entry returned by the custom importer inside the list.
// Note: source/srcmap can be empty to let LibSass do the file resolving.
// Note: we take ownership of the passed `source` and `srcmap` memory.
struct SassImport* sass_make_import(const char* imp_path, const char* abs_base,
  char* source, char* srcmap, enum SassImportSyntax format);

// Just in case we have some stray import structs
void sass_delete_import(struct SassImport* import);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Getter for specific import format for the given import (force css/sass/scss or set to auto)
enum SassImportSyntax sass_import_get_type(const struct SassImport* import);

// Setter for specific import format for the given import (force css/sass/scss or set to auto)
void sass_import_set_syntax(struct SassImport* import, enum SassImportSyntax syntax);

// Getter for original import path (as seen when parsed)
const char* sass_import_get_imp_path(const struct SassImport* import);

// Getter for resolve absolute path (after being resolved)
const char* sass_import_get_abs_path(const struct SassImport* import);

// Getter for import error message (used by custom importers).
// If error is not `nullptr`, the import must be considered as failed.
const char* sass_import_get_error_message(struct SassImport* import);

// Setter for import error message (used by custom importers).
// If error is not `nullptr`, the import must be considered as failed.
void sass_import_set_error_message(struct SassImport* import, const char* msg);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Create new list container for imports.
struct SassImportList* sass_make_import_list();

// Release memory of list container and all children.
void sass_delete_import_list(struct SassImportList* list);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

// Return number of items currently in the list.
size_t sass_import_list_size(struct SassImportList* list);

// Remove and return first item in the list (as in a fifo queue).
struct SassImport* sass_import_list_shift(struct SassImportList* list);

// Append an additional import to the list container.
void sass_import_list_push(struct SassImportList* list, struct SassImport* import);

// Append an additional import to the list container and takes ownership of the import.
void sass_import_list_emplace(struct SassImportList* list, struct SassImport* import);

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
```
