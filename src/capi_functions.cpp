// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include "capi_functions.hpp"
#include "source.hpp"

using namespace Sass;

extern "C" {

  struct SassFunction* ADDCALL sass_make_function(const char* signature, SassFunctionLambda callback, void* cookie)
  {
    struct SassFunction* cb = new SassFunction{};
    if (cb == 0 || signature == 0) return 0;
    cb->signature = sass_copy_c_string(signature);
    cb->function = callback;
    cb->cookie = cookie;
    return cb;
  }

  void ADDCALL sass_delete_function(struct SassFunction* function)
  {
    sass_free_c_string(function->signature);
    delete function;
  }

  // Setters and getters for callbacks on function lists
  // struct SassFunction* ADDCALL sass_function_get_list_entry(struct SassFunctionList* list, uint32_t pos) { return list[pos]; }
  // void sass_function_set_list_entry(struct SassFunctionList* list, uint32_t pos, struct SassFunction* cb) { list[pos] = cb; }

  const char* ADDCALL sass_function_get_signature(struct SassFunction* function)
  {
    return function->signature;
  }

  SassFunctionLambda ADDCALL sass_function_get_function(struct SassFunction* function)
  {
    return function->function;
  }

  void* ADDCALL sass_function_get_cookie(struct SassFunction* function)
  {
    return function->cookie;
  }

  struct SassImporter* ADDCALL sass_make_importer(SassImporterLambda callback, double priority, void* cookie)
  {
    struct SassImporter* importer = new SassImporter{};
    if (importer == 0) return 0;
    importer->importer = callback;
    importer->priority = priority;
    importer->cookie = cookie;
    return importer;
  }

  SassImporterLambda ADDCALL sass_importer_get_callback(struct SassImporter* importer) { return importer->importer; }
  double ADDCALL sass_importer_get_priority (struct SassImporter* importer) { return importer->priority; }
  void* ADDCALL sass_importer_get_cookie(struct SassImporter* importer) { return importer->cookie; }

  // Just in case we have some stray import structs
  void ADDCALL sass_delete_importer (struct SassImporter* importer)
  {
    delete importer;
  }

  // Creator for a single import entry returned by the custom importer inside the list
  // We take ownership of the memory for source and srcmap (freed when context is destroyed)
  struct SassImport* ADDCALL sass_make_import(const char* imp_path, const char* abs_path, char* source, char* srcmap, enum SassImportFormat format)
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

  // struct SassImport* ADDCALL sass_make_import_error(const char* error)
  // {
  //   return LoadedImport::wrap(
  //     new LoadedImport(error)
  //   );
  // }


  // Upgrade a normal import entry to throw an error (original path can be re-used by error reporting)
  void ADDCALL sass_import_set_error_msg(struct SassImport* import, const char* error, uint32_t line, uint32_t col)
  {
    if (import == nullptr) return;
    // import->err_msg = error ? error : "";
  }

  // Setters and getters for entries on the import list
  // void ADDCALL sass_import_set_list_entry(struct SassImportList* list, uint32_t idx, struct SassImport* entry) { list[idx] = entry; }
  // struct SassImport* ADDCALL sass_import_get_list_entry(struct SassImportList* list, uint32_t idx) { return list[idx]; }

  // Just in case we have some stray import structs
  void ADDCALL sass_delete_import(struct SassImport* import)
  {
    // std::cerr << "Delete " << (void*)import << "\n";
    Import& object = Import::unwrap(import);
    if (object.refcount <= 1) {
      object.refcount = 0;
      delete& object;
    }
    else {
      object.refcount -= 1;
    }
  }

  // Getter for callee entry
  // const char* ADDCALL sass_callee_get_name(struct SassCallee* entry) { return entry->name; }
  // const char* ADDCALL sass_callee_get_path(struct SassCallee* entry) { return entry->path; }
  // uint32_t ADDCALL sass_callee_get_line(struct SassCallee* entry) { return entry->line; }
  // uint32_t ADDCALL sass_callee_get_column(struct SassCallee* entry) { return entry->column; }
  // enum Sass_Callee_Type ADDCALL sass_callee_get_type(struct SassCallee* entry) { return entry->type; }

  // Getters and Setters for environments (lexical, local and global)
  struct SassValue* ADDCALL sass_env_get_lexical (struct SassCompiler* compiler, const char* name) {
    return 0; // Value::wrap(reinterpret_cast<Sass::Context*>(compiler->context)->getVariableIdx(Sass::EnvKey(name)));
  }

  void ADDCALL sass_env_set_lexical (struct SassCompiler* compiler, const char* name, struct SassValue* val) {
    // Compiler::unwrap(compiler).setLexicalVariable(Sass::EnvKey(name), reinterpret_cast<Value*>(val));
  }

  struct SassValue* ADDCALL sass_env_get_local (struct SassCompiler* compiler, const char* name) {
    return 0; // return Value::wrap(reinterpret_cast<Sass::Context*>(compiler->context)->getLocalVariable(Sass::EnvKey(name)));
  }

  void ADDCALL sass_env_set_local (struct SassCompiler* compiler, const char* name, struct SassValue* val) {
    // reinterpret_cast<Sass::Context*>(compiler->context)->setLocalVariable(Sass::EnvKey(name), reinterpret_cast<Value*>(val));
  }

  struct SassValue* ADDCALL sass_env_get_global (struct SassCompiler* compiler, const char* name) {
    return 0; // Value::wrap(reinterpret_cast<Sass::Context*>(compiler->context)->getGlobalVariable(Sass::EnvKey(name)));
  }

  void ADDCALL sass_env_set_global (struct SassCompiler* compiler, const char* name, struct SassValue* val) {
    // Compiler::unwrap(compiler).setGlobalVariable(Sass::EnvKey(name), reinterpret_cast<Value*>(val));
  }

  // Getter for import entry
  const char* ADDCALL sass_import_get_imp_path(struct SassImport* entry) { return Import::unwrap(entry).getImpPath(); }
  const char* ADDCALL sass_import_get_abs_path(struct SassImport* entry) { return Import::unwrap(entry).getAbsPath(); }
  // const char* ADDCALL sass_import_get_source(struct SassImport* entry) { return Import::unwrap(entry).getContents(); }
  // const char* ADDCALL sass_import_get_srcmap(struct SassImport* entry) { return Import::unwrap(entry).source->srcmaps(); }
  enum SassImportFormat ADDCALL sass_import_get_type(struct SassImport* entry) { return Import::unwrap(entry).syntax;  }

  // Getter for import error entry
  // uint32_t ADDCALL sass_import_get_error_line(struct SassImport* entry) { return entry->line; }
  // uint32_t ADDCALL sass_import_get_error_column(struct SassImport* entry) { return entry->column; }
  const char* ADDCALL sass_import_get_error_message(struct SassImport* entry) {
    return 0;
    // return entry->err_msg.empty() ? 0 : entry->err_msg.c_str();
  }

  // Explicit functions to take ownership of the memory
  // Resets our own property since we do not know if it is still alive
  // char* ADDCALL sass_import_take_source(struct SassImport* entry) { char* ptr = entry->source; entry->source = 0; return ptr; }
  // char* ADDCALL sass_import_take_srcmap(struct SassImport* entry) { char* ptr = entry->srcmap; entry->srcmap = 0; return ptr; }

}
