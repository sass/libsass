#include "capi_compiler.hpp"

#include "source.hpp"
#include "exceptions.hpp"

#ifdef _MSC_VER
// #include <windows.h>
#endif

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Some specific implementations that could also go into C++ part
  /////////////////////////////////////////////////////////////////////////

  // Format the error and fill compiler error states
  static int handle_error(Compiler& compiler, StackTraces* traces, const char* what, int status)
  {

    sass::ostream formatted;
    bool has_final_lf = false;
    Logger& logger(compiler);
    formatted << "Error: ";
    // Add message and ensure it is
    // added with a final line-feed.
    if (what != nullptr) {
      formatted << what;
      while (*what) {
        has_final_lf =
          *what == '\r' ||
          *what == '\n';
        ++what;
      }
      if (!has_final_lf) {
        formatted << STRMLF;
      }
    }

    // Clear the previous array
    compiler.error.traces.clear();
    // Some stuff is only logged if we have some traces
    // Otherwise we don't know where the error comes from
    if (traces && traces->size() > 0) {
      logger.writeStackTraces(formatted, *traces, "  ");
      // Copy items over to error object
      compiler.error.traces = *traces;
    }

    // Attach stuff to error object
    compiler.error.what.clear();
    compiler.error.status = status;
    if (what) compiler.error.what = what;
    compiler.error.formatted = formatted.str();

    return status;

  }
  // EO handle_error

  // Main entry point to catch errors
  static int handle_error(Compiler& compiler)
  {
    // Re-throw last error
    try { throw; }
    // Catch LibSass specific error cases
    catch (Exception::Base & e) { handle_error(compiler, &e.traces, e.what(), 1); }
    // Bad allocations can always happen, maybe we should exit in this case?
    catch (std::bad_alloc & e) { handle_error(compiler, nullptr, e.what(), 2); }
    // Other errors should not really happen and indicate more severe issues!
    catch (std::exception & e) { handle_error(compiler, nullptr, e.what(), 3); }
    catch (sass::string & e) { handle_error(compiler, nullptr, e.c_str(), 4); }
    catch (const char* e) { handle_error(compiler, nullptr, e, 4); }
    catch (...) { handle_error(compiler, nullptr, "unknown", 5); }
    // Return the error state
    return compiler.error.status;
  }
  // EO handle_error

  // allow one error handler to throw another error
  // this can happen with invalid utf8 and json lib
  // Note: this might be obsolete but doesn't hurt!?
  static int handle_errors(Compiler& compiler)
  {
    try { return handle_error(compiler); }
    catch (...) { return handle_error(compiler); }
  }
  // EO handle_errors

  // Main implementation (caller is wrapped in try/catch)
  void __sass_compiler_parse(Compiler& compiler)
  {
    compiler.parse();
  }
  // EO __sass_compiler_parse

  // Main implementation (caller is wrapped in try/catch)
  void __sass_compiler_compile(Compiler& compiler)
  {
    compiler.compile();
  }
  // EO __sass_compiler_compile

  // Main implementation (caller is wrapped in try/catch)
  void __sass_compiler_render(Compiler& compiler)
  {

    // Bail out if we had any previous errors
    if (compiler.error.status != 0) return;
    // Make sure compile step was called before
    if (compiler.compiled == nullptr) return;

    // This will hopefully use move semantics
    OutputBuffer output(compiler.renderCss());
    compiler.content = std::move(output.buffer);

    // Create options to render source map and footer.
    struct SassSrcMapOptions options(compiler.srcmap_options);
    // Deduct some options always from original values.
    // ToDo: is there really any need to customize this?
    if (options.origin.empty() || options.origin == "stream://stdout") {
      options.origin = compiler.getOutputPath();
    }
    if (options.path.empty() || options.path == "stream://stdout") {
      options.path = options.origin + ".map";
    }

    switch (options.mode) {
    case SASS_SRCMAP_NONE:
      compiler.srcmap = 0;
      compiler.footer = 0;
      break;
    case SASS_SRCMAP_CREATE:
      compiler.srcmap = compiler.renderSrcMapJson(options, *output.smap);
      compiler.footer = nullptr; // Don't add link, just create map file
      break;
    case SASS_SRCMAP_EMBED_LINK:
      compiler.srcmap = compiler.renderSrcMapJson(options, *output.smap);
      compiler.footer = compiler.renderSrcMapLink(options, *output.smap);
      break;
    case SASS_SRCMAP_EMBED_JSON:
      compiler.srcmap = compiler.renderSrcMapJson(options, *output.smap);
      compiler.footer = compiler.renderEmbeddedSrcMap(options, *output.smap);
      break;
    }

  }
  // EO __sass_compiler_render

  /////////////////////////////////////////////////////////////////////////
  // On windows we can improve the error handling by also catching
  // structured exceptions. In order for this to work we need a few
  // additional wrapper functions, which fortunately don't cost much.
  /////////////////////////////////////////////////////////////////////////

  #ifdef _MSC_VER
  int filter(unsigned int code, struct _EXCEPTION_POINTERS* ep)
  {
    // Handle exceptions we can't handle otherwise
    // Because these are not regular C++ exceptions
    if (code == EXCEPTION_ACCESS_VIOLATION)
    {
      return EXCEPTION_EXECUTE_HANDLER;
    }
    else if (code == EXCEPTION_STACK_OVERFLOW)
    {
      return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
      // Do not handle any other exceptions here
      // Should be handled by regular try/catch
      return EXCEPTION_CONTINUE_SEARCH;
    }
  }

  // Convert exception codes to strings
  const char* seException(unsigned int code)
  {
    switch (code) {
    case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
    case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
    case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
    case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
    case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
    case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
    default: return "UNKNOWN EXCEPTION";
    }
  }
  #endif

  // Wrap Structured Exceptions for MSVC
  void _sass_compiler_parse(Compiler& compiler)
  {
    #ifdef _MSC_VER
    __try {
    #endif
      __sass_compiler_parse(compiler);
    #ifdef _MSC_VER
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation())) {
      throw std::runtime_error(seException(GetExceptionCode()));
    }
    #endif
  }

  // Wrap Structured Exceptions for MSVC
  void _sass_compiler_compile(Compiler& compiler)
  {
    #ifdef _MSC_VER
    __try {
    #endif
      __sass_compiler_compile(compiler);
    #ifdef _MSC_VER
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation())) {
      throw std::runtime_error(seException(GetExceptionCode()));
    }
    #endif
  }

  // Wrap Structured Exceptions for MSVC
  void _sass_compiler_render(Compiler& compiler)
  {
    #ifdef _MSC_VER
    __try {
    #endif
      __sass_compiler_render(compiler);
    #ifdef _MSC_VER
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation())) {
      throw std::runtime_error(seException(GetExceptionCode()));
    }
    #endif
  }

  // Main entry point (wrap try/catch then MSVC)
  void sass_compiler_parse(Compiler& compiler)
  {
    Logger& logger(compiler);
    try { _sass_compiler_parse(compiler); }
    catch (...) { handle_errors(compiler); }
    compiler.warnings = logger.logstrm.str();
  }

  // Main entry point (wrap try/catch then MSVC)
  void sass_compiler_compile(Compiler& compiler)
  {
    Logger& logger(compiler);
    try { _sass_compiler_compile(compiler); }
    catch (...) { handle_errors(compiler); }
    compiler.warnings = logger.logstrm.str();
  }

  // Main entry point (wrap try/catch then MSVC)
  void sass_compiler_render(Compiler& compiler)
  {
    Logger& logger(compiler);
    try { _sass_compiler_render(compiler); }
    catch (...) { handle_errors(compiler); }
    compiler.warnings = logger.logstrm.str();
  }

  extern "C" {

    bool ADDCALL sass_compiler_get_source_comments(struct SassCompiler* compiler)
    {
      return Compiler::unwrap(compiler).source_comments;
    }

    // void ADDCALL sass_compiler_set_source_comments(struct SassCompiler* compiler, bool source_comments)
    // {
    //   Compiler::unwrap(compiler).source_comments = source_comments;
    // }

    // Returns pointer to error object associated with compiler.
    // Will be valid until the associated compiler is destroyed.
    const struct SassError* ADDCALL sass_compiler_get_error(struct SassCompiler* compiler)
    {
      return &Compiler::unwrap(compiler).error;
    }

    struct SassImport* ADDCALL sass_compiler_get_last_import(struct SassCompiler* compiler)
    {
      return Compiler::unwrap(compiler).import_stack.back()->wrap();
    }

    size_t ADDCALL sass_compiler_count_traces(struct SassCompiler* compiler)
    {
      Logger& logger(Compiler::unwrap(compiler));
      return logger.callStack.size();
    }

    const struct SassTrace* ADDCALL sass_compiler_last_trace(struct SassCompiler* compiler)
    {
      Logger& logger(Compiler::unwrap(compiler));
      return logger.callStack.back().wrap();
    }

    const struct SassTrace* ADDCALL sass_compiler_get_trace(struct SassCompiler* compiler, size_t i)
    {
      Logger& logger(Compiler::unwrap(compiler));
      return logger.callStack.at(i).wrap();
    }

  }

}


/////////////////////////////////////////////////////////////////////////
// The actual C-API Implementations
/////////////////////////////////////////////////////////////////////////

using namespace Sass;

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  struct SassCompiler* ADDCALL sass_make_compiler()
  {
    return Compiler::wrap(new Compiler());
  }

  void ADDCALL sass_delete_compiler(struct SassCompiler* compiler)
  {
    delete& Compiler::unwrap(compiler);
    #ifdef DEBUG_SHARED_PTR
    SharedObj::dumpMemLeaks();
    #endif

  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Push function for include paths (no manipulation support for now)
  void ADDCALL sass_compiler_add_include_paths(struct SassCompiler* compiler, const char* paths)
  {
    Compiler::unwrap(compiler).addIncludePaths(paths);
  }

  // Push function for plugin paths (no manipulation support for now)
  void ADDCALL sass_compiler_load_plugins(struct SassCompiler* compiler, const char* paths)
  {
    Compiler::unwrap(compiler).loadPlugins(paths);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_compiler_add_custom_header(struct SassCompiler* compiler, struct SassImporter* header)
  {
    Compiler::unwrap(compiler).addCustomHeader(header);
  }

  void ADDCALL sass_compiler_add_custom_importer(struct SassCompiler* compiler, struct SassImporter* importer)
  {
    Compiler::unwrap(compiler).addCustomImporter(importer);
  }

  void ADDCALL sass_compiler_add_custom_function(struct SassCompiler* compiler, struct SassFunction* function)
  {
    Compiler::unwrap(compiler).addCustomFunction(function);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_compiler_set_precision(struct SassCompiler* compiler, int precision)
  {
    Compiler::unwrap(compiler).precision = precision;
    Compiler::unwrap(compiler).setLogPrecision(precision);
  }

  int ADDCALL sass_compiler_get_precision(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).precision;
  }

  const char* ADDCALL sass_compiler_get_output_path(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).output_path.c_str();
  }

  struct SassImport* ADDCALL sass_compiler_get_entry_point(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).entry_point->wrap();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_compiler_set_srcmap_mode(struct SassCompiler* compiler, enum SassSrcMapMode mode)
  {
    Compiler::unwrap(compiler).srcmap_options.mode = mode;
  }

  void ADDCALL sass_compiler_set_srcmap_root(struct SassCompiler* compiler, const char* root)
  {
    Compiler::unwrap(compiler).srcmap_options.root = root;
  }

  void ADDCALL sass_compiler_set_srcmap_path(struct SassCompiler* compiler, const char* path)
  {
    Compiler::unwrap(compiler).srcmap_options.path = path;
  }

  void ADDCALL sass_compiler_set_srcmap_file_urls(struct SassCompiler* compiler, bool enable)
  {
    Compiler::unwrap(compiler).srcmap_options.file_urls = enable;
  }

  void ADDCALL sass_compiler_set_srcmap_embed_contents(struct SassCompiler* compiler, bool enable)
  {
    Compiler::unwrap(compiler).srcmap_options.embed_contents = enable;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  size_t ADDCALL sass_compiler_get_included_files_count(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).included_sources.size();
  }

  const char* ADDCALL sass_compiler_get_included_file_path(struct SassCompiler* compiler, size_t n)
  {
    return Compiler::unwrap(compiler).included_sources.at(n)->getAbsPath();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Implement file-loading from given data
  struct SassImport* ADDCALL sass_make_stdin_import(const char* path)
  {
    std::istreambuf_iterator<char> begin(std::cin), end;
    sass::string text(begin, end); // consume everything
    Import* import = SASS_MEMORY_NEW(Import,
      SASS_MEMORY_NEW(SourceString,
        path ? path : "stream://stdin",
        std::move(text)),
      SASS_IMPORT_AUTO);
    // Use reference counter
    import->refcount += 1;
    // Create the shared source object
    return Import::wrap(import);
  }
  // EO sass_make_stdin_import

  // Implement initial data-loading without import path resolving.
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

  // Implement initial file-loading without import path resolving.
  struct SassImport* ADDCALL sass_make_file_import(const char* imp_path)
  {
    sass::string abs_path(File::rel2abs(imp_path, CWD));
    Import* loaded = SASS_MEMORY_NEW(Import,
      SASS_MEMORY_NEW(SourceFile,
        imp_path, abs_path.c_str(),
        nullptr, nullptr),
      SASS_IMPORT_AUTO);
    loaded->refcount += 1;
    return loaded->wrap();
  }
  // EO sass_make_file_import
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_compiler_set_entry_point(struct SassCompiler* compiler, struct SassImport* import)
  {
    auto& qwe = Import::unwrap(import);
    // std::cerr << "1) " << qwe.refcount << "\n";
    Compiler::unwrap(compiler).entry_point = &qwe;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_compiler_set_output_path(struct SassCompiler* compiler, const char* output_path)
  {
    Compiler::unwrap(compiler).output_path = output_path ? output_path : "stream://stdout";
  }

  void ADDCALL sass_compiler_set_output_style(struct SassCompiler* compiler, enum SassOutputStyle output_style)
  {
    Compiler::unwrap(compiler).output_style = output_style;
  }

  void ADDCALL sass_compiler_set_logger_style(struct SassCompiler* compiler, enum SassLoggerStyle log_style)
  {
    Compiler::unwrap(compiler).setLogStyle(log_style);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  void ADDCALL sass_compiler_parse(struct SassCompiler* sass_compiler)
  {
    sass_compiler_parse(Compiler::unwrap(sass_compiler));
  }

  void ADDCALL sass_compiler_compile(struct SassCompiler* sass_compiler)
  {
    sass_compiler_compile(Compiler::unwrap(sass_compiler));
  }

  void ADDCALL sass_compiler_render(struct SassCompiler* sass_compiler)
  {
    sass_compiler_render(Compiler::unwrap(sass_compiler));
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  const char* ADDCALL sass_compiler_get_output_string(struct SassCompiler* compiler)
  {
    if (Compiler::unwrap(compiler).content.empty()) return nullptr;
    return Compiler::unwrap(compiler).content.c_str();
  }
  
  const char* ADDCALL sass_compiler_get_warn_string(struct SassCompiler* compiler)
  {
    if (Compiler::unwrap(compiler).warnings.empty()) return nullptr;
    return Compiler::unwrap(compiler).warnings.c_str();
  }

  const char* ADDCALL sass_compiler_get_footer_string(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).footer;
  }

  const char* ADDCALL sass_compiler_get_srcmap_string(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).srcmap;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

}
