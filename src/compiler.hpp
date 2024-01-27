#ifndef SASS_COMPILER_H
#define SASS_COMPILER_H

// sass.hpp must go before all system headers
// to get the __EXTENSIONS__ fix on Solaris.
#include "capi_sass.hpp"

#include <map>
#include "logger.hpp"
#include "ast_css.hpp"
#include "stylesheet.hpp"
#include "capi_error.hpp"
#include "capi_import.hpp"
#include "capi_importer.hpp"
#include "capi_compiler.hpp"
#include "capi_traces.hpp"
#include "source_map.hpp"

namespace Sass {

  // Helper function to sort header and importer arrays by priorities
  inline bool cmpImporterPrio(struct SassImporter* i, struct SassImporter* j)
  {
    return sass_importer_get_priority(i) > sass_importer_get_priority(j);
  }

  // The main compiler context object holding config and results
  class Compiler final : public OutputOptions, public Logger {

  private:

    #ifdef DEBUG_MSVC_CRT_MEM
    _CrtMemState memState;
    #endif

  public:

    // Checking if a file exists can be quite extensive
    // Keep an internal map to avoid repeated system calls
    std::unordered_map<sass::string, bool> fileExistsCache;

    // Keep cache of resolved import filenames
    // Key is a pair of previous + import path
    std::unordered_map<ImportRequest, sass::vector<ResolvedImport>> resolveCache;

    // Include paths are local to context since we need to know
    // it for lookups during parsing. You may reset this for
    // another compilation when reusing the context.
    sass::vector<sass::string> includePaths;

  public:

    Root* modctx3 = nullptr;

    WithConfig* wconfig = nullptr;

    EnvKeyMap<BuiltInMod*> modules;

  protected:

    // Functions in order of appearance
    // Same order needed for function stack
    sass::vector<CallableObj> fnList;

  public:

    // sass::vector<WithConfigVar>* withConfig = nullptr;
    virtual ~Compiler();

    // EnvKeyMap<WithConfigVar> withConfig;

    // Sheets are filled after resources are parsed
    // This could be shared, should go to engine!?
    // ToDo: should be case insensitive on windows?
    std::map<const sass::string, RootObj> sheets;

    // Only used to cache `loadImport` calls
    std::map<const sass::string, ImportObj> sources;

    // Additional C-API stuff for interaction
    sass::vector<struct SassImporter*> cHeaders;
    sass::vector<struct SassImporter*> cImporters;
    sass::vector<struct SassFunction*> cFunctions;

  public:

    // The import stack during evaluation phase
    sass::vector<ImportObj> import_stack;

    // List of all sources that have been included
    sass::vector<SourceDataObj> included_sources;

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    // Helpers for `sass_prepare_context`
    /////////////////////////////////////////////////////////////////////////

    void addCustomHeader(struct SassImporter* header);
    void addCustomImporter(struct SassImporter* importer);
    void addCustomFunction(struct SassFunction* function);

    /////////////////////////////////////////////////////////////////////////
    // Some simple delegations to variable root for runtime queries
    /////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////

    // Load plugins from path, which can be path separated
    void loadPlugins(const sass::string& paths);

    // Add additional include paths, which can be path separated
    void addIncludePaths(const sass::string& paths);

    // Load import from the file-system and create source object
    // Results will be stored at `sources[source->getAbsPath()]`
    Import* loadImport(const ResolvedImport& import);

    // Implementation for `sass_compiler_find_file`
    // Looks for the file in regard to the current
    // import and then looks in all include folders.
    sass::string findFile(const sass::string& path);

    // Look for all possible filename variants (e.g. partials)
    // Returns all results (e.g. for ambiguous but valid imports)
    const sass::vector<ResolvedImport>& findIncludes(const ImportRequest& import, bool forImport);


  public:

    EnvRefs* getCurrentScope() const {
      if (varRoot.stack.empty()) return nullptr;
      return varRoot.stack.back();
    }

    EnvRefs* getCurrentModule() const {
      if (varRoot.stack.empty()) return nullptr;
      auto current = varRoot.stack.back();
      while (current->pscope) {
        if (current->module) break;
        current = current->pscope;
      }
      return current;
    }

    BuiltInMod& createModule(const sass::string& name) {
      auto it = modules.find(name);
      if (it != modules.end()) {
        return *it->second;
      }
      BuiltInMod* module = new BuiltInMod(varRoot);
      modules.insert({ name, module });
      return *module;
    }

    BuiltInMod* getModule(const sass::string& name) {
      auto it = modules.find(name);
      if (it == modules.end()) return nullptr;
      return it->second;
    }


    // Flag if we currently have a with-config
    bool hasWithConfig = false;

    // Stack of environment frames. New frames are appended
    // when parser encounters a new environment scoping.
    sass::vector<EnvRefs*> varStack3312;

    // The root environment where parsed root variables
    // and (custom) functions plus mixins are registered.
    EnvRoot varRoot; // Must be after varStack!

    // Functions only for evaluation phase (C-API functions and eval itself)
    // CallableObj* findFunction(const EnvKey& name) { return varRoot.findFunction(name); }

    // The current state the compiler is in.
    enum SassCompilerState state;

    // Where we want to store the output.
    // Source-map path is deducted from it.
    // Defaults to `stream://stdout`.
    sass::string output_path;

    // main entry point for compilation
    ImportObj entry_point;

    // Parsed ast-tree
    RootObj sheet;

    // Evaluated ast-tree
    CssRootObj compiled;

    // The rendered css content.
    sass::string content;

    // Rendered warnings and debugs. They can be emitted at any stage.
    // Therefore we make a copy into our string after each stage from
    // the actual logger instance (created by context). This is needed
    // in order to return a `c_str` on the API from the output-stream.
    sass::string warnings;

    // The rendered output footer. This includes the
    // rendered css comment footer for the source-map.
    // Append after output for the full output document.
    char* footer;

    // The rendered source map. This is what an implementor
    // would normally write out to the `output.css.map` file
    char* srcmap;

    // Runtime error
    SassError error;

    // Constructor
    Compiler();

    // Parse ast tree
    void parse();

    // Compile parsed tree
    void compile();

    // Render compiled tree
    OutputBuffer renderCss();

    // Get path of compilation entry point
    // Returns the resolved/absolute path
    sass::string getInputPath() const;

    // Get the output path for this compilation
    // Can be explicit or deducted from input path
    sass::string getOutputPath() const;

    // ToDO, maybe belongs to compiler?
    CssRootObj compileRoot(bool plainCss);

    char* renderSrcMapJson(const SourceMap& source_map);

    char* renderSrcMapLink(const SourceMap& source_map);

    char* renderEmbeddedSrcMap(const SourceMap& source_map);

    void reportSuppressedWarnings();

    // Check if we should write output file
    bool hasOutputFile() const;

    // Check if we should write srcmap file
    bool hasSrcMapFile() const;

    /////////////////////////////////////////////////////////////////////////
    // Register built-in function with only one parameter list.
    /////////////////////////////////////////////////////////////////////////
    uint32_t createBuiltInFunction(const EnvKey& name,
      const sass::string& signature, SassFnSig cb);
    uint32_t registerBuiltInFunction(const EnvKey& name,
      const sass::string& signature, SassFnSig cb);
    uint32_t createInternalFunction(const EnvKey& name,
      const sass::string& signature, SassFnSig cb);

    uint32_t registerInternalFunction(const EnvKey& name,
      const sass::string& signature, SassFnSig cb);

    void exposeFunction(const EnvKey& name, uint32_t idx);

    uint32_t createBuiltInVariable(const EnvKey& name, Value* value);

    // Only a few mixins will accept a content block
    uint32_t createBuiltInMixin(const EnvKey& name,
      const sass::string& signature, SassFnSig cb,
      bool acceptsContent = false);

    /////////////////////////////////////////////////////////////////////////
    // Register built-in functions that can take different
    // functional arguments (best suited will be chosen).
    /////////////////////////////////////////////////////////////////////////
    uint32_t createBuiltInOverloadFns(const EnvKey& name,
      const sass::vector<std::pair<const sass::string, SassFnSig>>& overloads);
    uint32_t registerBuiltInOverloadFns(const EnvKey& name,
      const sass::vector<std::pair<const sass::string, SassFnSig>>& overloads);

    /////////////////////////////////////////////////////////////////////////
    // @param imp_path The relative or custom path for be imported
    // @param pstate SourceSpan where import occurred (parent context)
    // @param rule The backing ImportRule that is added to the document
    /////////////////////////////////////////////////////////////////////////
    bool callCustomHeaders(const sass::string& imp_path, SourceSpan& pstate, ImportRule* rule)
    {
      return callCustomLoader(imp_path, pstate, rule, cHeaders, false);
    };

    /////////////////////////////////////////////////////////////////////////
    // @param imp_path The relative or custom path for be imported
    // @param pstate SourceSpan where import occurred (parent context)
    // @param rule The backing ImportRule that is added to the document
    /////////////////////////////////////////////////////////////////////////
    bool callCustomImporters(const sass::string& imp_path, SourceSpan& pstate, ImportRule* rule)
    {
      return callCustomLoader(imp_path, pstate, rule, cImporters, true);
    };

    // Parse the import (updates syntax flag if AUTO was set)
    // Results will be stored at `sheets[source->getAbsPath()]`
    Root* registerImport(ImportObj import);

    // Called by parserStylesheet on the very first parse call
    void applyCustomHeaders(StatementVector& root, SourceSpan pstate);

  protected:

    /////////////////////////////////////////////////////////////////////////
    // Called once to register all built-in functions.
    // This will invoke parsing for parameter lists.
    /////////////////////////////////////////////////////////////////////////
    void loadBuiltInFunctions();

    Root* parseRoot(ImportObj import);

  private:

    /////////////////////////////////////////////////////////////////////////
    // @param imp_path The relative or custom path for be imported
    // @param pstate SourceSpan where import occurred (parent context)
    // @param rule The backing ImportRule that is added to the document
    // @param importers Array of custom importers/headers to go through
    // @param singleton Whether to use all importers or only first successful
    /////////////////////////////////////////////////////////////////////////
    bool callCustomLoader(const sass::string& imp_path, SourceSpan& pstate, ImportRule* rule,
      const sass::vector<struct SassImporter*>& importers, bool singletone = true);

    public:
    /////////////////////////////////////////////////////////////////////////
    // Register an external custom sass function on the global scope.
    // Main entry point for custom functions passed through the C-API.
    // The function you pass in will be taken over and freed by us!
    /////////////////////////////////////////////////////////////////////////
      void registerCustomFunction(struct SassFunction* function);

    // Invoke parser according to import format
    RootObj parseSource(ImportObj source);

  public:

    void setPrecision(int precision);

    CAPI_WRAPPER(Compiler, SassCompiler);

  };

  class ImportStackFrame
  {
  private:

    Compiler& compiler;

  public:

    ImportStackFrame(
      Compiler& compiler,
      Import* import);

    ~ImportStackFrame();

  };


}


#endif
