/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "compiler.hpp"

#include "json.hpp"
#include "eval.hpp"
#include "output.hpp"
#include "stylesheet.hpp"
#include "capi_lists.hpp"
#include "parser_scss.hpp"
#include "parser_sass.hpp"
#include "parser_css.hpp"
#include "exceptions.hpp"
#include "remove_placeholders.hpp"

// Functions to register
#include "fn_maps.hpp"
#include "fn_meta.hpp"
#include "fn_lists.hpp"
#include "fn_texts.hpp"
#include "fn_colors.hpp"
#include "fn_numbers.hpp"
#include "fn_selectors.hpp"

#include <cstring>
#include <csignal>
#ifdef _MSC_VER
#include <excpt.h>
#endif

#include "debugger.hpp"

struct SassImport;

namespace Sass {



  struct SassValue* get_global(struct SassCompiler* comp, struct SassValue* name)
  {
    Value& value(Value::unwrap(name));
    Compiler& compiler(Compiler::unwrap(comp));
    ValueObj rv = compiler.getVariable(
      value.assertString(compiler, "name")->getText(), true);
    if (rv) rv->refcount += 1;
    return rv ? Value::wrap(rv) : sass_make_null();
  }

  struct SassValue* get_lexical(struct SassCompiler* comp, struct SassValue* name)
  {
    Value& value(Value::unwrap(name));
    Compiler& compiler(Compiler::unwrap(comp));
    ValueObj rv = compiler.getVariable(
      value.assertString(compiler, "name")->getText());
    if (rv) rv->refcount += 1;
    return rv ? Value::wrap(rv) : sass_make_null();
  }

  struct SassValue* get_local(struct SassCompiler* comp, struct SassValue* name)
  {
    Value& value(Value::unwrap(name));
    Compiler& compiler(Compiler::unwrap(comp));
    ValueObj rv = compiler.getVariable(
      value.assertString(compiler, "name")->getText());
    if (rv) rv->refcount += 1;
    return rv ? Value::wrap(rv) : sass_make_null();
  }

  struct SassValue* set_global(struct SassCompiler* comp, struct SassValue* name, struct SassValue* value)
  {
    Value& key(Value::unwrap(name));
    Value& val(Value::unwrap(value));
    Compiler& compiler(Compiler::unwrap(comp));
    compiler.setVariable(key.assertString(compiler, "name")->getText(), &val, true);
    return sass_make_null();
  }

  struct SassValue* set_lexical(struct SassCompiler* comp, struct SassValue* name, struct SassValue* value)
  {
    Value& key(Value::unwrap(name));
    Value& val(Value::unwrap(value));
    Compiler& compiler(Compiler::unwrap(comp));
    compiler.setVariable(key.assertString(compiler, "name")->getText(), &val);
    return sass_make_null();
  }

  struct SassValue* set_local(struct SassCompiler* comp, struct SassValue* name, struct SassValue* value)
  {
    Value& key(Value::unwrap(name));
    Value& val(Value::unwrap(value));
    Compiler& compiler(Compiler::unwrap(comp));
    compiler.setVariable(key.assertString(compiler, "name")->getText(), &val);
    return sass_make_null();
  }

  // so far only pow has two arguments
#define IMPLEMENT_2_ARG_FN(fn) \
struct SassValue* fn_##fn(struct SassValue* s_args, Sass_Function_Entry cb, struct SassCompiler* comp) \
{ \
  if (!sass_value_is_list(s_args)) { \
    return sass_make_error("Invalid arguments for " #fn); \
  } \
  if (sass_list_get_size(s_args) != 2) { \
    return sass_make_error("Exactly two arguments expected for " #fn); \
  } \
  struct SassValue* name = sass_list_get_value(s_args, 0); \
  struct SassValue* value = sass_list_get_value(s_args, 1); \
  return fn(comp, name, value); \
} \

  // so far only pow has two arguments
#define IMPLEMENT_1_ARG_FN(fn) \
struct SassValue* fn_##fn(struct SassValue* s_args, Sass_Function_Entry cb, struct SassCompiler* comp) \
{ \
  if (!sass_value_is_list(s_args)) { \
    return sass_make_error("Invalid arguments for " #fn); \
  } \
  if (sass_list_get_size(s_args) != 1) { \
    return sass_make_error("Exactly one arguments expected for " #fn); \
  } \
  struct SassValue* name = sass_list_get_value(s_args, 0); \
  return fn(comp, name); \
} \

// one argument functions
  // IMPLEMENT_2_ARG_FN(pow);

  IMPLEMENT_1_ARG_FN(get_global);
  IMPLEMENT_1_ARG_FN(get_lexical);
  IMPLEMENT_1_ARG_FN(get_local);
  IMPLEMENT_2_ARG_FN(set_global);
  IMPLEMENT_2_ARG_FN(set_lexical);
  IMPLEMENT_2_ARG_FN(set_local);

  // create a custom header to define to variables
  struct SassImportList* custom_header(const char* cur_path, struct SassImporter* cb, struct SassCompiler* comp)
  {
    // create a list to hold our import entries
    struct SassImportList* incs = sass_make_import_list();

    // sass_env_set_global(comp, "test", sass_make_number(42, ""));
    // sass_env_set_global(comp, "$test", sass_make_number(42, ""));
    // create our only import entry (must make copy)
    sass_import_list_push(incs, sass_make_import(
        "[math]", "[math]", sass_copy_c_string(
          "$E: 2.718281828459045235360287471352;\n"
          "$PI: 3.141592653589793238462643383275;\n"
          "$TAU: 6.283185307179586476925286766559;\n"
        ), 0, SASS_IMPORT_AUTO));
    // return imports
    return incs;
  }

  Compiler::Compiler() :
    Context(),
    state(SASS_COMPILER_CREATED),
    output_path("stream://stdout"),
    entry_point(),
    footer(nullptr),
    srcmap(nullptr),
    error()
  {
    // allocate a custom function caller
    // Sass_Importer_Entry c_header =
    //   sass_make_importer(custom_header, 5000, (void*)0);
    // addCustomHeader(c_header);
  }

  // Get path of compilation entry point
  // Returns the resolved/absolute path
  sass::string Compiler::getInputPath() const
  {
    // Simply return absolute path of entry point
    if (entry_point && entry_point->getAbsPath()) {
      return entry_point->getAbsPath();
    }
    // Fall back to stdin
    return "stream://stdin";
  }
  // EO getInputPath

  // Get the output path for this compilation
  // Can be explicit or deducted from input path
  sass::string Compiler::getOutputPath() const
  {
    // Specific output path was provided
    if (!output_path.empty()) {
      return output_path;
    }
    // Otherwise deduct it from input path
    sass::string path(getInputPath());
    // Check if input is coming from stdin
    if (path == "stream://stdin") {
      return "stream://stdout";
    }
    // Otherwise remove the file extension
    size_t dotpos = path.find_last_of('.');
    if (dotpos != sass::string::npos) {
      path.erase(dotpos);
    }
    // Use css extension
    path += ".css";
    return path;
  }
  // EO getOutputPath

  void Compiler::parse()
  {
    // Do initial loading
    entry_point->loadIfNeeded();
    // Now parse the entry point stylesheet
    sheet = parseRoot(entry_point);
    // Update the compiler state
    state = SASS_COMPILER_PARSED;
  }

  // parse root block from includes (Move to compiler)
  CssRootObj Compiler::compileRoot(bool plainCss)
  {
    RootObj root = sheet->root2;
    if (root == nullptr) return {};

    #ifdef DEBUG_SHARED_PTR
    // Enable reference tracking
    SharedObj::taint = true;
    #endif

    // abort if there is no data
    if (included_sources.size() == 0) return {};
    // abort on invalid root
    if (root.isNull()) return {};

    Eval eval(*this, *this, plainCss);
    EnvScope scoped(varRoot, varRoot.idxs);
    for (size_t i = 0; i < fnList.size(); i++) {
      varRoot.functions[i] = fnList[i];
    }

    // debug_ast(root);
    CssRootObj compiled = eval.acceptRoot(root); // 50%
    // debug_ast(compiled);

    Extension unsatisfied;
    // check that all extends were used
    if (eval.checkForUnsatisfiedExtends(unsatisfied)) {
      throw Exception::UnsatisfiedExtend(*this, unsatisfied);
    }

    // clean up by removing empty placeholders
    // ToDo: maybe we can do this somewhere else?
    Remove_Placeholders remove_placeholders;
    remove_placeholders.visitCssRoot(compiled); // 3%

    #ifdef DEBUG_SHARED_PTR
    // Enable reference tracking
    SharedObj::taint = false;
    #endif

    // return processed tree
    return compiled;
  }

  void Compiler::compile()
  {
    // Only act right after parsing
    if (state == SASS_COMPILER_PARSED) {
      // Compile the parsed ast-tree
      compiled = compileRoot(false);
      // Update the compiler state
      state = SASS_COMPILER_COMPILED;
    }
  }

  OutputBuffer Compiler::renderCss()
  {
    // Create the emitter object
    Output emitter(*this, srcmap_options.mode != SASS_SRCMAP_NONE);
    emitter.reserve(1024 * 1024); // 1MB
    emitter.in_declaration = false;
    // Start the render process
    if (compiled != nullptr) {
      emitter.visitCssRoot(compiled);
    }
    // Finish emitter stream
    emitter.finalize();
    // Update the compiler state
    state = SASS_COMPILER_RENDERED;
    // Move buffer from stream
    return emitter.getBuffer();
  }
  // EO renderCss

  // Case 1) output to stdout, source map must be fully inline
  // Case 2) output to path, source map output is deducted from it
  char* Compiler::renderSrcMapLink(struct SassSrcMapOptions options, const SourceMap& source_map)
  {
    // Source map json must already be there
    if (srcmap == nullptr) return nullptr;
    // Check if we output to stdout (any link would be useless)
    if (output_path.empty() || output_path == "stream://stdout") {
      // Instead always embed the source-map on stdout
      return renderEmbeddedSrcMap(options, source_map);
    }
    // Create resulting footer and return a copy
    return sass_copy_string("\n/*# sourceMappingURL=" +
      File::abs2rel(options.path, options.origin) + " */");

  }
  // EO renderSrcMapLink

  // Memory returned by this function must be freed by caller via `sass_free_c_string`
  char* Compiler::renderEmbeddedSrcMap(struct SassSrcMapOptions options, const SourceMap& source_map)
  {
    // Source map json must already be there
    if (srcmap == nullptr) return nullptr;
    // Encode json to base64
    sass::istream is(srcmap);
    sass::ostream buffer;
    buffer << "\n/*# sourceMappingURL=";
    buffer << "data:application/json;base64,";
    base64::encoder E;
    E.encode(is, buffer);
    buffer << " */";
    return sass_copy_string(buffer.str());
  }
  // EO renderEmbeddedSrcMap

  // Memory returned by this function must be freed by caller via `sass_free_c_string`
  char* Compiler::renderSrcMapJson(struct SassSrcMapOptions options, const SourceMap& source_map)
  {
    // Create the emitter object
    // Sass::OutputBuffer buffer;

    /**********************************************/
    // Create main object to render json
    /**********************************************/
    JsonNode* json_srcmap = json_mkobject();

    /**********************************************/
    // Create the source-map version information
    /**********************************************/
    json_append_member(json_srcmap, "version", json_mknumber(3));

    /**********************************************/
    // Create file reference to whom our mappings apply
    /**********************************************/
    sass::string origin(options.origin);
    origin = File::abs2rel(origin, CWD);
    JsonNode* json_file_name = json_mkstring(origin.c_str());
    json_append_member(json_srcmap, "file", json_file_name);

    /**********************************************/
    // pass-through source_map_root option
    /**********************************************/
    if (!options.root.empty()) {
      json_append_member(json_srcmap, "sourceRoot",
        json_mkstring(options.root.c_str()));
    }

    /**********************************************/
    // Create the included sources array
    /**********************************************/
    JsonNode* json_sources = json_mkarray();
    for (size_t i = 0; i < included_sources.size(); ++i) {
      const SourceData* source(included_sources[i]);
      sass::string path(source->getAbsPath());
      path = File::rel2abs(path, ".", CWD);
      // Optionally convert to file urls
      if (options.file_urls) {
        if (path[0] == '/') {
          // ends up with three slashes
          path = "file://" + path;
        }
        else {
          // needs an additional slash
          path = "file:///" + path;
        }
        // Append item to json array
        json_append_element(json_sources,
          json_mkstring(path.c_str()));
      }
      else {
        path = File::abs2rel(path, ".", CWD);
        // Append item to json array
        json_append_element(json_sources,
          json_mkstring(path.c_str()));
      }
    }
    json_append_member(json_srcmap, "sources", json_sources);

    // add a relative link to the source map output file
    // srcmap_links88.emplace_back(abs2rel(abs_path, file88, CWD));

    /**********************************************/
    // Check if we have any includes to render
    /**********************************************/
    if (options.embed_contents) {
      JsonNode* json_contents = json_mkarray();
      for (size_t i = 0; i < included_sources.size(); ++i) {
        const SourceData* source = included_sources[i];
        JsonNode* json_content = json_mkstring(source->content());
        json_append_element(json_contents, json_content);
      }
      json_append_member(json_srcmap, "sourcesContent", json_contents);
    }

    /**********************************************/
    // So far we have no implementation for names
    /**********************************************/
    json_append_member(json_srcmap, "names", json_mkarray());

    /**********************************************/
    // Create source remapping lookup table
    // For source-maps we need to output sources in
    // consecutive manner, but we might have used various
    // different stylesheets from the prolonged context
    /**********************************************/
    std::unordered_map<size_t, size_t> idxremap;
    for (auto& source : included_sources) {
      idxremap.insert(std::make_pair(
        source->getSrcIdx(),
        idxremap.size()));
    }

    /**********************************************/
    // Finally render the actual source mappings
    // Remap context srcidx to consecutive ordering
    /**********************************************/
    sass::string mappings(source_map.render(idxremap));
    JsonNode* json_mappings = json_mkstring(mappings.c_str());
    json_append_member(json_srcmap, "mappings", json_mappings);

    /**********************************************/
    // Render the json and return result
    // Memory must be freed by consumer!
    /**********************************************/
    char* data = json_stringify(json_srcmap, "\t");
    json_delete(json_srcmap);
    return data;

  }
  // EO renderSrcMapJson

  /////////////////////////////////////////////////////////////////////////
  // @param imp_path The relative or custom path for be imported
  // @param pstate SourceSpan where import occurred (parent context)
  // @param rule The backing ImportRule that is added to the document
  // @param importers Array of custom importers/headers to go through
  // @param singleton Whether to use all importers or only first successful
  /////////////////////////////////////////////////////////////////////////
  bool Compiler::callCustomLoader(const sass::string& imp_path, SourceSpan& pstate,
    ImportRule* rule, const sass::vector<struct SassImporter*>& importers, bool singleton)
  {
    // unique counter
    size_t count = 0;
    // need one correct import
    bool has_import = false;

    const char* ctx_path = pstate.getAbsPath();


    // Process custom importers and headers.
    // They must be presorted by priorities.
    for (struct SassImporter* importer : importers) {

      // Get the external importer function
      SassImporterLambda fn = sass_importer_get_callback(importer);

      // std::cerr << "Calling custom loader " << fn << "\n";

      // Call the external function, then check what it returned
      struct SassImportList* imports = fn(imp_path.c_str(), importer, this->wrap());
      // External provider want to handle this
      if (imports != nullptr) {

        // std::cerr << "HAS imports\n";

        // Get the list of possible imports
        // A list with zero items does nothing
        while (sass_import_list_size(imports) > 0) {
          // Increment counter
          ++count;
          // Consume the first item from the list
          struct SassImport* entry = sass_import_list_shift(imports);
          Import& import = Import::unwrap(entry);
          // Create a unique path to use as key
          sass::string uniq_path = imp_path;
          // Append counter to the path
          // Note: only for headers!
          if (!singleton && count) {
            sass::sstream path_strm;
            path_strm << uniq_path << ":" << count;
            uniq_path = path_strm.str();
          }
          // create the importer struct
          ImportRequest importer(uniq_path, ctx_path);
          // query data from the current include
          SourceDataObj source = import.source;
          // const char* content = sass_import_get_source(import);
          // const char* srcmap = sass_import_get_srcmap(import);
          // auto format = sass_import_get_type(import2);

          //if (sass_import_get_error_message(import)) {
          //  BackTraces& traces = *this;
          //  traces.push_back(BackTrace(pstate));
          //  Exception::CustomImportError err(traces, "custom error");
          //  sass_delete_import_list(imports);
          //  sass_delete_import(import2);
          //  throw err;
          //
          //  /*
          //
          //
          //
          //// Handle error message passed back from custom importer
          //// It may (or may not) override the line and column info
          //  size_t line = sass_import_get_error_line(import);
          //  size_t column = sass_import_get_error_column(import);
          //  // sass_delete_import(import); // will error afterwards
          //  if (line == sass::string::npos) error(err_message, pstate, *logger123);
          //  else if (column == sass::string::npos) error(err_message, pstate, *logger123);
          //  else error(err_message, { source, Offset::init(line, column) }, *logger123);
          //  */
          //}
          // Content for import was set.
          // No need to load it ourself.
          //else
            if (source) {

            const char* rel_path = import.getImpPath();
            const char* abs_path = import.getAbsPath();

            if (import.isLoaded()) {
              // Resolved abs_path should be set by custom importer
              // Use the created uniq_path as fall-back (enforce?)
              sass::string path_key(abs_path ? abs_path : uniq_path);
              // Import is ready to be served
              if (import.syntax == SASS_IMPORT_AUTO)
                import.syntax = SASS_IMPORT_SCSS;
              StyleSheet* sheet = registerImport(&import);
              // Add a dynamic import to the import rule
              rule->append(SASS_MEMORY_NEW(IncludeImport,
                pstate, sheet));
            }
            // Only a path was returned
            // Try to load it like normal
            else if (abs_path || rel_path) {
              // Create a copy for possible error reporting
              sass::string path(abs_path ? abs_path : rel_path);
              // Pass it on as if it was a regular import
              ImportRequest request(path, ctx_path);

              // Search for valid imports (e.g. partials) on the file-system
              // Returns multiple valid results for ambiguous import path
              const sass::vector<ResolvedImport> resolved(findIncludes(request));


              // Error if no file to import was found
              if (resolved.empty()) {
                BackTraces& traces = *this;
                traces.push_back(BackTrace(pstate));
                Exception::CustomImportNotFound err(traces, path);
                sass_delete_import_list(imports);
                sass_delete_import(entry);
                throw err;
              }
              // Error if multiple files to import were found
              else if (resolved.size() > 1) {
                BackTraces& traces = *this;
                traces.push_back(BackTrace(pstate));
                Exception::CustomImportAmbigous err(traces, path);
                sass_delete_import_list(imports);
                sass_delete_import(entry);
                throw err;
              }

              // We made sure exactly one entry was found, load its content
              if (ImportObj loaded = loadImport(resolved[0])) {
                StyleSheet* sheet = registerImport(loaded);
                rule->append(SASS_MEMORY_NEW(IncludeImport, pstate, sheet));
              }
              else {
                BackTraces& traces = *this;
                traces.push_back(BackTrace(pstate));
                Exception::CustomImportLoadError err(traces, path);
                sass_delete_import_list(imports);
                sass_delete_import(entry);
                throw err;
              }

            }
          }
          sass_delete_import(entry);
        }

        // Deallocate the returned memory
        sass_delete_import_list(imports);
        // Set success flag
        has_import = true;
        // Break out of loop
        if (singleton) break;
      }
    }
    // Return result
    return has_import;
  }
  // EO callCustomLoader




  void Compiler::applyCustomHeaders(StatementVector& statements, SourceSpan pstate)
  {
    // create a custom import to resolve headers
    ImportRuleObj rule = SASS_MEMORY_NEW(ImportRule, pstate);
    // dispatch headers which will add custom functions
    // custom headers are added to the import instance
    if (callCustomHeaders("sass://header", pstate, rule)) {
      statements.push_back(rule);
    }
  }


  /////////////////////////////////////////////////////////////////////////
  // Interface for built in functions
  /////////////////////////////////////////////////////////////////////////

  // Register built-in function with only one parameter list.
  void Compiler::registerBuiltInFunction(const sass::string& name,
    const sass::string& signature, SassFnSig cb)
  {
    EnvRoot root(varStack);
    SourceDataObj source = SASS_MEMORY_NEW(SourceString,
      "sass://signature", "(" + signature + ")");
    auto args = ArgumentDeclaration::parse(*this, source);
    auto callable = SASS_MEMORY_NEW(BuiltInCallable, name, args, cb);
    fnLookup.insert(std::make_pair(callable->envkey(), callable));
    varRoot.createFunction(callable->envkey());
    fnList.push_back(callable);
  }
  // EO registerBuiltInFunction

  // Register built-in functions that can take different
  // functional arguments (best suited will be chosen).
  void Compiler::registerBuiltInOverloadFns(const sass::string& name,
    const std::vector<std::pair<sass::string, SassFnSig>>& overloads)
  {
    SassFnPairs pairs;
    for (auto overload : overloads) {
      EnvRoot root(varStack);
      SourceDataObj source = SASS_MEMORY_NEW(SourceString,
        "sass://signature", "(" + overload.first + ")");
      auto args = ArgumentDeclaration::parse(*this, source);
      pairs.emplace_back(std::make_pair(args, overload.second));
    }
    auto callable = SASS_MEMORY_NEW(BuiltInCallables, name, pairs);
    fnLookup.insert(std::make_pair(name, callable));
    varRoot.createFunction(name);
    fnList.push_back(callable);
  }
  // EO registerBuiltInOverloadFns

  /////////////////////////////////////////////////////////////////////////
  // Interface for external custom functions
  /////////////////////////////////////////////////////////////////////////

  // Create a new external callable from the sass function. Parses
  // function signature into function name and argument declaration.
  ExternalCallable* Compiler::makeExternalCallable(struct SassFunction* function)
  {
    // Create temporary source object for signature
    SourceStringObj source = SASS_MEMORY_NEW(SourceString,
      "sass://signature", function->signature);
    // Create a new scss parser instance
    ScssParser parser(*this, source);
    ExternalCallable* callable =
      parser.parseExternalCallable();
    callable->function(function);
    return callable;
  }
  // EO makeExternalCallable

  // Register an external custom sass function on the global scope.
  // Main entry point for custom functions passed through the C-API.
  void Compiler::registerCustomFunction(struct SassFunction* function)
  {
    EnvRoot root(varStack);
    // Create a new external callable from the sass function
    ExternalCallable* callable = makeExternalCallable(function);
    // Currently external functions are treated globally
    if (fnLookup.count(callable->envkey()) == 0) {
      fnLookup.insert(std::make_pair(callable->envkey(), callable));
      varRoot.createFunction(callable->envkey());
      fnList.push_back(callable);
    }
  }
  // EO registerCustomFunction

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Invoke parser according to import format
  RootObj Compiler::parseSource(ImportObj import)
  {
    if (import->syntax == SASS_IMPORT_CSS)
    {
      CssParser parser(*this, import->source);
      return parser.parseRoot();
    }
    else if (import->syntax == SASS_IMPORT_SASS)
    {
      SassParser parser(*this, import->source);
      return parser.parseRoot();
    }
    else {
      ScssParser parser(*this, import->source);
      return parser.parseRoot();
    }
  }
  // EO parseSource

  // Parse the import (updates syntax flag if AUTO was set)
  // Results will be stored at `sheets[source->getAbsPath()]`
  StyleSheet* Compiler::registerImport(ImportObj import)
  {

    SassImportFormat& format(import->syntax);
    const SourceDataObj& source(import->source);
    const sass::string& abs_path(source->getAbsPath());

    // ToDo: We don't take format into account
    auto cached = sheets.find(abs_path);
    if (cached != sheets.end()) {
      return cached->second;
    }

    // Assign unique index to the source
    source->setSrcIdx(included_sources.size());
    // Add source to global include array
    included_sources.emplace_back(source);
    // Put import onto the stack array
    import_stack.emplace_back(import);

    // check existing import stack for possible recursion
    for (size_t i = 0; i < import_stack.size() - 2; ++i) {
      const SourceDataObj parent = import_stack[i]->source;
      if (std::strcmp(parent->getAbsPath(), source->getAbsPath()) == 0) {
        // make path relative to the current directory
        sass::string stack("An @import loop has been found:");
        for (size_t n = 1; n < i + 2; ++n) {
          stack += "\n    " + sass::string(File::abs2rel(import_stack[n]->source->getAbsPath(), CWD, CWD)) +
            " imports " + sass::string(File::abs2rel(import_stack[n + 1]->source->getAbsPath(), CWD, CWD));
        }
        // implement error throw directly until we
        // decided how to handle full stack traces
        throw Exception::RuntimeException(*this, stack);
      }
    }

    // Auto detect input file format
    if (format == SASS_IMPORT_AUTO) {
      using namespace StringUtils;
      if (endsWithIgnoreCase(abs_path, ".css", 4)) {
        format = SASS_IMPORT_CSS;
      }
      else if (endsWithIgnoreCase(abs_path, ".sass", 5)) {
        format = SASS_IMPORT_SASS;
      }
      else if (endsWithIgnoreCase(abs_path, ".scss", 5)) {
        format = SASS_IMPORT_SCSS;
      }
      else {
        throw Exception::RuntimeException(*this,
          "Can't find stylesheet to import.");
      }
    }

    // Invoke correct parser according to format
    StyleSheet* stylesheet = SASS_MEMORY_NEW(
      StyleSheet, import, parseSource(import));

    // Pop from import stack
    import_stack.pop_back();

    // Put the parsed stylesheet into the map
    sheets.insert({ abs_path, stylesheet });

    // Return pointer
    return stylesheet;

  }
  // EO registerImport

  // Called once to register all built-in functions.
  // This will invoke parsing for parameter lists.
  void Compiler::loadBuiltInFunctions()
  {
    Functions::Meta::registerFunctions(*this);
    Functions::Math::registerFunctions(*this);
    Functions::Maps::registerFunctions(*this);
    Functions::Lists::registerFunctions(*this);
    Functions::Colors::registerFunctions(*this);
    Functions::Texts::registerFunctions(*this);
    Functions::Selectors::registerFunctions(*this);
  }
  // EO loadBuiltInFunctions

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void sigHandler(int s)
  {
    std::cerr << "Abnormal program termination detected!\n";
    std::cerr << "  With Signal " << s << "\n";
    exit(EXIT_FAILURE);
  }

  StyleSheet* Compiler::parseRoot(ImportObj import)
  {

    // Attach signal handlers
    signal(SIGABRT, sigHandler);
    signal(SIGFPE, sigHandler);
    signal(SIGILL, sigHandler);
    signal(SIGINT, sigHandler);
    signal(SIGSEGV, sigHandler);
    signal(SIGTERM, sigHandler);

    // Insert ourself onto the sources cache
    sources.insert({ import->getAbsPath(), import });

    import_stack.emplace_back(import);

    loadBuiltInFunctions();


    // auto qwe = sass_make_function("pow($foo, $bar)", fn_pow, (void*)31);

    // registerCustomFunction(qwe);

    registerCustomFunction(sass_make_function("set-local($name, $value)", fn_set_local, (void*)31));
    registerCustomFunction(sass_make_function("set-global($name, $value)", fn_set_global, (void*)31));
    registerCustomFunction(sass_make_function("set-lexical($name, $value)", fn_set_lexical, (void*)31));

    registerCustomFunction(sass_make_function("get-local($name)", fn_get_local, (void*)31));
    registerCustomFunction(sass_make_function("get-global($name)", fn_get_global, (void*)31));
    registerCustomFunction(sass_make_function("get-lexical($name)", fn_get_lexical, (void*)31));

    // ScopedStack scoped(varStack, &varRoot);

    // register custom functions (defined via C-API)
    for (auto& function : cFunctions)
      registerCustomFunction(function);

    #ifdef DEBUG_SHARED_PTR
    // Enable reference tracking
    SharedObj::taint = true;
    #endif

    // load and register import
    StyleSheet* sheet = registerImport(import);

    #ifdef DEBUG_SHARED_PTR
    // Disable reference tracking
    SharedObj::taint = false;
    #endif

    // Return root node
    return sheet;

  }
  // EO parseRoot

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
