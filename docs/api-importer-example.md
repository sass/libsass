## Example importer.c

```C
#include <stdio.h>
#include <string.h>
#include "sass/compiler.h"
#include "sass/importer.h"

struct SassImportList* sass_importer(SassImporterLambda lambda, double priority, void* cookie)
{
  // get the cookie from importer descriptor
  struct SassImportList* list = sass_make_import_list();
  char* local = sass_copy_c_string("local { color: green; }");
  char* remote = sass_copy_c_string("remote { color: red; }");
  sass_import_list_emplace(list, sass_make_content_import(local, "/tmp/styles.scss"));
  sass_import_list_emplace(list, sass_make_content_import(remote, "http://www.example.com"));
  return list;
}

int main(int argc, const char* argv[])
{
  // create compiler object holding config and states
  struct SassCompiler* compiler = sass_make_compiler();
  // create the file context and get all related structs
  struct SassImport* import = sass_make_content_import("@import 'foobar';", 0);
  // each compiler must have exactly one entry point
  sass_compiler_set_entry_point(compiler, import);
  // entry point now passed to compiler, so its reference count was increased
  // in order to not leak memory we must release our own usage (usage after is UB)
  sass_delete_import(import); // decrease ref-count

  // allocate custom importer
  struct SassImporter* importer =
    sass_make_importer(sass_importer, 0, 0);
  // create list for all custom importers
  sass_compiler_add_custom_importer(compiler, importer);
  // context is set up, call the compile step now
  int status = sass_compiler_execute(compiler, false);
  // release allocated memory
  sass_delete_compiler(compiler);
  // exit status
  return status;

}
```

Compile importer.c

```bash
gcc -c importer.c -o importer.o -Iinclude
g++ -o importer importer.o -lsass -Llib
echo "@import 'foobar';" > importer.scss
./importer importer.scss
```

## Importer Behavior Examples

```C
struct SassImportList* importer(SassImporterLambda lambda, double priority, void* cookie) {
  // Skip this importer and try next in queue
  return NULL;
}

struct SassImportList* importer(SassImporterLambda lambda, double priority, void* cookie) {
  // Let LibSass load the file identified by the importer
  // No further importers are consulted
  struct SassImportList* list = sass_make_import_list();
  sass_import_list_emplace(list, sass_make_file_import(path));
  return list;
}

struct SassImportList* importer(SassImporterLambda lambda, double priority, void* cookie) {
  // Return an error to halt execution
  struct SassImportList* list = sass_make_import_list();
  const char* message = "some error message";
  struct SassImport* import = sass_make_file_import(path);
  sass_import_set_error_message(import, sass_copy_c_string(message));
  sass_import_list_emplace(list, import);
  return list;
}

struct SassImportList* importer(SassImporterLambda lambda, double priority, void* cookie) {
  // Let LibSass load the file identified by the importer
  struct SassImportList* list = sass_make_import_list();
  sass_import_list_emplace(list, sass_make_file_import(path));
  return list;
}

struct SassImportList* importer(SassImporterLambda lambda, double priority, void* cookie) {
  // Completely hide the import
  // No further importers are consulted
  struct SassImportList* list = sass_make_import_list();
  return list;
}

```
