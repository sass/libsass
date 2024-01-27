/*****************************************************************************/
/* Part of LibSass, released under the MIT license (See LICENSE.txt).        */
/*****************************************************************************/
#include "capi_compiler.hpp"

#include <iostream>
#include <fstream>
#include "logger.hpp"
#include "compiler.hpp"
#include "exceptions.hpp"

namespace Sass {

  /////////////////////////////////////////////////////////////////////////
  // Some specific implementations that could also go into C++ part
  // ToDo: maybe we should guard all C-API functions this way!?
  /////////////////////////////////////////////////////////////////////////

  // Promote and format error onto compiler with given status, message and traces
  int handle_error(Compiler& compiler, int status, const char* what, StackTraces* traces)
  {

    sass::ostream error;
    bool has_final_lf = false;
    Logger& logger(compiler);
    error << "Error: ";
    // Add message and ensure it is
    // added with a final line-feed.
    const char* msg = what;
    if (msg != nullptr) {
      error << msg;
      while (*msg) {
        has_final_lf =
          *msg == '\r' ||
          *msg == '\n';
        ++msg;
      }
      if (!has_final_lf) {
        error << STRMLF;
      }
    }

    sass::ostream formatted;
    print_wrapped(error.str(), compiler.columns, formatted);

    // Clear the previous array
    compiler.error.traces.clear();
    // Some stuff is only logged if we have some traces
    // Otherwise we don't know where the error comes from
    if (traces && traces->size() > 0) {
      // Write traces to string with some indentation
      logger.writeStackTraces(formatted, *traces, "  ");
      // Copy items over to error object
      compiler.error.traces = *traces;
    }

    // Attach stuff to error object
    compiler.error.what.clear();
    compiler.error.status = status;
    if (what) compiler.error.what = what;
    compiler.error.formatted = formatted.str();
    compiler.state = SASS_COMPILER_FAILED;

    // Return status again
    return status;
  }
  // EO handle_error

  // Main entry point to catch errors
  static int handle_error(Compiler& compiler)
  {
    // Re-throw last error
    try { throw; }
    // Catch LibSass specific error cases
    catch (Exception::Base & e) { handle_error(compiler, 1, e.what(), &e.traces); }
    // Bad allocations can always happen, maybe we should exit in this case?
    catch (std::bad_alloc & e) { handle_error(compiler, 2, e.what()); }
    // Other errors should not really happen and indicate more severe issues!
    catch (std::exception & e) { handle_error(compiler, 3, e.what()); }
    catch (sass::string & e) { handle_error(compiler, 4, e.c_str()); }
    catch (const char* what) { handle_error(compiler, 4, what); }
    catch (...) { handle_error(compiler, 5, "unknown"); }
    // Return the error state
    return compiler.error.status;
  }
  // EO handle_error

  // allow one error handler to throw another error
  // this can happen with invalid utf8 and json lib
  // ToDo: this might be obsolete but doesn't hurt!?
  static int handle_errors(Compiler& compiler)
  {
    try { return handle_error(compiler); }
    catch (...) { return handle_error(compiler); }
  }
  // EO handle_errors

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Main implementation (caller is wrapped in try/catch)
  void _sass_compiler_parse(Compiler& compiler)
  {
    compiler.parse();
  }
  // EO _sass_compiler_parse

  // Main implementation (caller is wrapped in try/catch)
  void _sass_compiler_compile(Compiler& compiler)
  {
    compiler.compile();
  }
  // EO _sass_compiler_compile

  // Main implementation (caller is wrapped in try/catch)
  void _sass_compiler_render(Compiler& compiler)
  {

    // Bail out if we had any previous errors
    if (compiler.error.status != 0) return;
    // Make sure compile step was called before
    if (compiler.compiled == nullptr) return;

    // This will hopefully use move semantics
    OutputBuffer output(compiler.renderCss());
    compiler.content = std::move(output.buffer);

    // Create options to render source map and footer.
    SrcMapOptions& options(compiler.mapopt);
    // Deduct some options always from original values.
    // ToDo: is there really any need to customize this?
    if (options.origin.empty() || options.origin == "stream://stdout") {
      options.origin = compiler.getOutputPath();
    }
    if (options.path.empty() || options.path == "stream://stdout") {
      if (!options.origin.empty() && options.origin != "stream://stdout") {
        options.path = options.origin + ".map";
      }
    }

    switch (options.mode) {
    case SASS_SRCMAP_NONE:
      compiler.srcmap = nullptr;
      compiler.footer = nullptr;
      break;
    case SASS_SRCMAP_CREATE:
      compiler.srcmap = compiler.renderSrcMapJson(*output.srcmap);
      compiler.footer = nullptr; // Don't add link, just create map file
      break;
    case SASS_SRCMAP_EMBED_LINK:
      compiler.srcmap = compiler.renderSrcMapJson(*output.srcmap);
      compiler.footer = compiler.renderSrcMapLink(*output.srcmap);
      break;
    case SASS_SRCMAP_EMBED_JSON:
      compiler.srcmap = compiler.renderSrcMapJson(*output.srcmap);
      compiler.footer = compiler.renderEmbeddedSrcMap(*output.srcmap);
      break;
    }

  }
  // EO _sass_compiler_render

  // Main implementation (caller is wrapped in try/catch)
  void _sass_compiler_write_output(Compiler& compiler)
  {

    const char* path = compiler.output_path.c_str();

    // Write regular output if no error occurred
    if (compiler.error.status == 0) {

      // ToDo: can we use the same types?
      const char* footer = compiler.footer;
      const char* content = compiler.content.empty() ?
        nullptr : compiler.content.c_str();

      // Check if anything is to write
      if (content || footer) {
        // Check where to write it to
        if (path && compiler.hasOutputFile()) {
          std::ofstream fh(path, std::ios::out | std::ios::binary);
          if (!fh) throw Exception::IoError(compiler,
            "Error opening output file",
            File::abs2rel(path));
          // Write stuff to the output file
          if (content) { fh << content; }
          if (footer) { fh << footer; }
          // Close file-handle
          fh.close();
          // Check for error state after closing
          // This should report also write errors
          if (!fh) throw Exception::IoError(compiler,
            "Error writing output file",
            File::abs2rel(path));
        }
        else {
          // Simply print results to stdout
          if (content) std::cout << content;
          if (footer) std::cout << footer;
        }
      }

    }
    // Otherwise write special error css
    else if (path && compiler.hasOutputFile()) {
      std::ofstream fh(path, std::ios::out | std::ios::binary);
      // Skip writing if we already had an error
      if (compiler.error.status && !fh) return;
      if (!fh) throw Exception::IoError(compiler,
        "Error opening output file",
        File::abs2rel(path));
      // Write stuff to the output file
      compiler.error.writeCss(fh);
      // Close file-handle
      fh.close();
      // Check for error state after closing
      // This should report also write errors
      if (!fh) throw Exception::IoError(compiler,
        "Error writing output file",
        File::abs2rel(path));
    }

  }
  // EO _sass_compiler_write_output

  // Main implementation (caller is wrapped in try/catch)
  void _sass_compiler_write_srcmap(Compiler& compiler)
  {
    // Write to srcmap only if no errors occurred
    if (compiler.error.status != 0) return;

    const char* srcmap = compiler.srcmap;
    const char* path = compiler.mapopt.path.empty() ?
      nullptr : compiler.mapopt.path.c_str();

    // Write source-map if needed
    if (srcmap && path && compiler.hasSrcMapFile()) {
      std::ofstream fh(path, std::ios::out | std::ios::binary);
      if (!fh) throw std::runtime_error("Error opening srcmap file");
      // Write stuff to the srcmap file
      if (srcmap) { fh << srcmap; }
      // Close file-handle
      fh.close();
    }

  }

  void _sass_compiler_add_custom_function(Compiler& compiler, struct SassFunction* function)
  {
    compiler.registerCustomFunction(function);
  }
  // EO _sass_compiler_add_custom_function

  /////////////////////////////////////////////////////////////////////////
  // On windows we can improve the error handling by also catching
  // structured exceptions. In order for this to work we need a few
  // additional wrapper functions, which fortunately don't cost much.
  /////////////////////////////////////////////////////////////////////////

  #ifdef _MSC_VER
  // Helper function to filter how to handle exceptions
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

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Wrap Structured Exceptions for MSVC (void on no MSVC compilers)
  template <class T, typename ...ARGS> void sass_wrap_msvc_exception(
    T& compiler, void (*fn)(T& compiler, ARGS...), ARGS... args)
  {
    #ifdef _MSC_VER
    __try {
    #endif
      fn(compiler, args...);
    #ifdef _MSC_VER
    }
    __except (filter(GetExceptionCode(), GetExceptionInformation())) {
      throw std::runtime_error(seException(GetExceptionCode()));
    }
    #endif
  }
  // EO sass_wrap_msvc_exception

  // Wrap C++ exceptions and add to logger if any occur
  template <class T, typename ...ARGS> void sass_wrap_exception(
    T& compiler, void (*fn)(T& compiler, ARGS...), ARGS... args)
  {
    Logger& logger(compiler);
    try { sass_wrap_msvc_exception(compiler, fn, args...); }
    catch (...) { handle_errors(compiler); }
    compiler.warnings = logger.logstrm.str();
  }
  // EO sass_wrap_exception

}
// EO C++ helpers


/////////////////////////////////////////////////////////////////////////
// The actual C-API Implementations
/////////////////////////////////////////////////////////////////////////

extern "C" {

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  using namespace Sass;

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Create a new LibSass compiler context
  struct SassCompiler* ADDCALL sass_make_compiler()
  {
    return Compiler::wrap(new Compiler());
  }

  // Release all memory allocated with the compiler
  void ADDCALL sass_delete_compiler(struct SassCompiler* compiler)
  {
    delete& Compiler::unwrap(compiler);
    #ifdef DEBUG_SHARED_PTR
    RefCounted::dumpMemLeaks();
    #endif
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Parse the entry point and potentially all imports within.
  void ADDCALL sass_compiler_parse(struct SassCompiler* compiler)
  {
    sass_wrap_exception(Compiler::unwrap(compiler), _sass_compiler_parse);
  }

  // Evaluate the parsed entry point and store resulting ast-tree.
  void ADDCALL sass_compiler_compile(struct SassCompiler* compiler)
  {
    sass_wrap_exception(Compiler::unwrap(compiler), _sass_compiler_compile);
  }

  // Render the evaluated ast-tree to get the final output string.
  void ADDCALL sass_compiler_render(struct SassCompiler* compiler)
  {
    sass_wrap_exception(Compiler::unwrap(compiler), _sass_compiler_render);
  }

  // Write or print the output to the console or the configured output path
  void ADDCALL sass_compiler_write_output(struct SassCompiler* compiler)
  {
    sass_wrap_exception(Compiler::unwrap(compiler), _sass_compiler_write_output);
  }

  // Write source-map to configured path if options are set accordingly
  void ADDCALL sass_compiler_write_srcmap(struct SassCompiler* compiler)
  {
    sass_wrap_exception(Compiler::unwrap(compiler), _sass_compiler_write_srcmap);
  }

  // Execute all compiler steps and write/print results
  int ADDCALL sass_compiler_execute(struct SassCompiler* compiler)
  {

    // Execute all compiler phases
    // Will skip steps if one errors
    sass_compiler_parse(compiler);
    sass_compiler_compile(compiler);
    sass_compiler_render(compiler);

    // First print all warnings and deprecation messages
    if (!sass_compiler_get_suppress_stderr(compiler)) {
      if (sass_compiler_get_warn_string(compiler)) {
        sass_print_stderr(sass_compiler_get_warn_string(compiler));
      }
    }

    // Get original compiler exit status to return
    int result = sass_compiler_get_status(compiler);

    // Write/print the results
    sass_compiler_write_output(compiler);
    sass_compiler_write_srcmap(compiler);

    // Check for errors
    if (result != 0) {
      // Print error message if we have an error
      const struct SassError* error = sass_compiler_get_error(compiler);
      if (error) sass_print_stderr(sass_error_get_formatted(error));
    }

    // Return exit status
    return result;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Add additional include paths where LibSass will look for includes.
  // Note: the passed in `paths` can be path separated (`;` on windows, `:` otherwise).
  void ADDCALL sass_compiler_add_include_paths(struct SassCompiler* compiler, const char* paths)
  {
    Compiler::unwrap(compiler).addIncludePaths(paths);
  }

  // Load dynamic loadable plugins from `paths`. Plugins are only supported on certain OSs and
  // are still in experimental state. This will look for `*.dll`, `*.so` or `*.dynlib` files.
  // It then tries to load the found libraries and does a few checks to see if the library
  // is actually a LibSass plugin. We then call its init hook if the library is compatible.
  // Note: the passed in `paths` can be path separated (`;` on windows, `:` otherwise).
  void ADDCALL sass_compiler_load_plugins(struct SassCompiler* compiler, const char* paths)
  {
    Compiler::unwrap(compiler).loadPlugins(paths);
  }

  // Add a custom header importer that will always be executed before any other
  // compilations takes place. Useful to prepend a shared copyright header or to
  // provide global variables or functions. This feature is still in experimental state.
  // Note: With the adaption of Sass Modules this might be completely replaced in the future.
  void ADDCALL sass_compiler_add_custom_header(struct SassCompiler* compiler, struct SassImporter* header)
  {
    Compiler::unwrap(compiler).addCustomHeader(header);
  }

  // Add a custom importer that will be executed when a sass `@import` rule is found.
  // This is useful to e.g. rewrite import locations or to load content from remote.
  // For more please check https://github.com/sass/libsass/blob/master/docs/api-importer.md
  // Note: The importer will not be called for regular css `@import url()` rules.
  void ADDCALL sass_compiler_add_custom_importer(struct SassCompiler* compiler, struct SassImporter* importer)
  {
    Compiler::unwrap(compiler).addCustomImporter(importer);
  }

  // Add a custom function that will be executed when the corresponding function call is
  // requested from any sass code. This is useful to provide custom functions in your code.
  // For more please check https://github.com/sass/libsass/blob/master/docs/api-function.md
  // Note: since we need to parse the function signature this may throw an error!
  void ADDCALL sass_compiler_add_custom_function(struct SassCompiler* compiler, struct SassFunction* function)
  {
    // Wrap to correctly trap errors and to fill the error object if needed
    sass_wrap_exception(Compiler::unwrap(compiler), _sass_compiler_add_custom_function, function);
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Setter for output style (see `enum SassOutputStyle` for possible options).
  void ADDCALL sass_compiler_set_input_syntax(struct SassCompiler* compiler, enum SassImportSyntax syntax)
  {
    Compiler::unwrap(compiler).input_syntax = syntax;
  }

  // Setter for output style (see `enum SassOutputStyle` for possible options).
  void ADDCALL sass_compiler_set_output_style(struct SassCompiler* compiler, enum SassOutputStyle style)
  {
    Compiler::unwrap(compiler).output_style = style;
  }

  // Try to detect and set logger options for terminal colors, unicode and columns.
  void ADDCALL sass_compiler_autodetect_logger_capabilities(struct SassCompiler* compiler)
  {
    Compiler::unwrap(compiler).autodetectCapabalities();
  }

  // Setter for enabling/disabling logging with ANSI colors.
  void ADDCALL sass_compiler_set_logger_colors(struct SassCompiler* compiler, bool enable)
  {
   Compiler::unwrap(compiler).support_colors = enable;
  }

  // Setter for enabling/disabling logging with unicode text.
  void ADDCALL sass_compiler_set_logger_unicode(struct SassCompiler* compiler, bool enable)
  {
    Compiler::unwrap(compiler).support_unicode = enable;
  }

  // Getter for number precision (how floating point numbers are truncated).
  int ADDCALL sass_compiler_get_precision(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).precision;
  }

  // Setter for number precision (how floating point numbers are truncated).
  void ADDCALL sass_compiler_set_precision(struct SassCompiler* compiler, int precision)
  {
    Compiler::unwrap(compiler).setPrecision(precision);
  }

  // Getter for compiler entry point (which file or data to parse first).
  struct SassImport* ADDCALL sass_compiler_get_entry_point(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).entry_point->wrap();
  }

  // Setter for compiler entry point (which file or data to parse first).
  void ADDCALL sass_compiler_set_entry_point(struct SassCompiler* compiler, struct SassImport* import)
  {
    Compiler::unwrap(compiler).entry_point = &Import::unwrap(import);
  }

  // Getter for compiler output path (where to store the result)
  // Note: LibSass does not write the file, implementers should write to this path.
  const char* ADDCALL sass_compiler_get_output_path(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).output_path.c_str();
  }

  // Setter for compiler output path (where to store the result)
  // Note: LibSass does not write the file, implementers should write to this path.
  void ADDCALL sass_compiler_set_output_path(struct SassCompiler* compiler, const char* output_path)
  {
    Compiler::unwrap(compiler).output_path = output_path ? output_path : "stream://stdout";
  }

  // Getter for option to suppress anything being printed on stderr (quiet mode)
  bool ADDCALL sass_compiler_get_suppress_stderr(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).suppress_stderr;
  }

  // Setter for option to suppress anything being printed on stderr (quiet mode)
  void ADDCALL sass_compiler_set_suppress_stderr(struct SassCompiler* compiler, bool suppress)
  {
    Compiler::unwrap(compiler).suppress_stderr = suppress;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for warnings that occurred during any step.
  const char* ADDCALL sass_compiler_get_warn_string(struct SassCompiler* compiler)
  {
    if (Compiler::unwrap(compiler).warnings.empty()) return nullptr;
    Compiler::unwrap(compiler).reportSuppressedWarnings();
    return Compiler::unwrap(compiler).warnings.c_str();
  }

  // Getter for output after parsing, compilation and rendering.
  const char* ADDCALL sass_compiler_get_output_string(struct SassCompiler* compiler)
  {
    if (Compiler::unwrap(compiler).content.empty()) return nullptr;
    return Compiler::unwrap(compiler).content.c_str();
  }

  // Getter for footer string containing optional source-map (embedded or link).
  const char* ADDCALL sass_compiler_get_footer_string(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).footer;
  }

  // Getter for string containing the optional source-mapping.
  const char* ADDCALL sass_compiler_get_srcmap_string(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).srcmap;
  }

  // Check if implementor is expected to write a output file
  bool ADDCALL sass_compiler_has_output_file(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).hasOutputFile();
  }

  // Check if implementor is expected to write a source-map file
  bool ADDCALL sass_compiler_has_srcmap_file(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).hasSrcMapFile();
  }


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Setter for source-map mode (how to embed or not embed the source-map).
  void ADDCALL sass_compiler_set_srcmap_mode(struct SassCompiler* compiler, enum SassSrcMapMode mode)
  {
    Compiler::unwrap(compiler).mapopt.mode = mode;
  }

  // Setter for source-map path (where to store the source-mapping).
  // Note: if path is not explicitly given, we will deduct one from output path.
  // Note: LibSass does not write the file, implementers should write to this path.
  void ADDCALL sass_compiler_set_srcmap_path(struct SassCompiler* compiler, const char* path)
  {
    Compiler::unwrap(compiler).mapopt.path = path;
  }

  // Getter for source-map path (where to store the source-mapping).
  // Note: if path is not explicitly given, we will deduct one from output path.
  // Note: the value will only be deducted after the main render phase is completed.
  // Note: LibSass does not write the file, implementers should write to this path.
  const char* ADDCALL sass_compiler_get_srcmap_path(struct SassCompiler* compiler)
  {
    const sass::string& path(Compiler::unwrap(compiler).mapopt.path);
    return path.empty() ? nullptr : path.c_str();
  }

  // Setter for source-map root (simply passed to the resulting srcmap info).
  // Note: if not given, no root attribute will be added to the srcmap info object.
  void ADDCALL sass_compiler_set_srcmap_root(struct SassCompiler* compiler, const char* root)
  {
    Compiler::unwrap(compiler).mapopt.root = root;
  }

  // Setter for source-map file-url option (renders urls in srcmap as `file://` urls)
  void ADDCALL sass_compiler_set_srcmap_file_urls(struct SassCompiler* compiler, bool enable)
  {
    Compiler::unwrap(compiler).mapopt.file_urls = enable;
  }

  // Setter for source-map embed-contents option (includes full sources in the srcmap info)
  void ADDCALL sass_compiler_set_srcmap_embed_contents(struct SassCompiler* compiler, bool enable)
  {
    Compiler::unwrap(compiler).mapopt.embed_contents = enable;
  }

  // Setter to enable more detailed source map (also meaning bigger payload).
  // Mostly useful if you want to post process the results again where the more detailed
  // source-maps might by used by downstream post-processor to point back to original files.
  void ADDCALL sass_compiler_set_srcmap_details(struct SassCompiler* compiler, bool openers, bool closers)
  {
    Compiler::unwrap(compiler).mapopt.enable_closers = closers;
    Compiler::unwrap(compiler).mapopt.enable_openers = openers;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter to return the number of all included files.
  size_t ADDCALL sass_compiler_get_included_files_count(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).included_sources.size();
  }

  // Getter to return path to the included file at position `n`.
  const char* ADDCALL sass_compiler_get_included_file_path(struct SassCompiler* compiler, size_t n)
  {
    return Compiler::unwrap(compiler).included_sources.at(n)->getAbsPath();
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Getter for current import context. Use `SassImport` functions to query the state.
  const struct SassImport* ADDCALL sass_compiler_get_last_import(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).import_stack.back()->wrap();
  }

  // Returns status code for compiler (0 meaning success, anything else is an error)
  int ADDCALL sass_compiler_get_status(struct SassCompiler* compiler)
  {
    return Compiler::unwrap(compiler).error.status;
  }

  // Returns pointer to error object associated with compiler.
  // Will be valid until the associated compiler is destroyed.
  const struct SassError* ADDCALL sass_compiler_get_error(struct SassCompiler* compiler)
  {
    return &Compiler::unwrap(compiler).error;
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  // Resolve a file relative to last import or include paths in the sass option struct.
  char* ADDCALL sass_compiler_find_file(const char* file, struct SassCompiler* compiler)
  {
    sass::string path(Compiler::unwrap(compiler).findFile(file));
    return path.empty() ? nullptr : sass_copy_c_string(path.c_str());
  }

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////



  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  //size_t ADDCALL sass_compiler_count_traces(struct SassCompiler* compiler)
  //{
  //  Logger& logger(Compiler::unwrap(compiler));
  //  return logger.callStack.size();
  //}
  //
  //const struct SassTrace* ADDCALL sass_compiler_get_trace(struct SassCompiler* compiler, size_t i)
  //{
  //  Logger& logger(Compiler::unwrap(compiler));
  //  return logger.callStack.at(i).wrap();
  //}
  //
  //const struct SassTrace* ADDCALL sass_compiler_last_trace(struct SassCompiler* compiler)
  //{
  //  Logger& logger(Compiler::unwrap(compiler));
  //  return logger.callStack.back().wrap();
  //}

  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////

  //   bool ADDCALL sass_compiler_get_source_comments(struct SassCompiler* compiler)
  //   {
  //     return Compiler::unwrap(compiler).source_comments;
  //   }
  //
  //   void ADDCALL sass_compiler_set_source_comments(struct SassCompiler* compiler, bool source_comments)
  //   {
  //     Compiler::unwrap(compiler).source_comments = source_comments;
  //   }

}
